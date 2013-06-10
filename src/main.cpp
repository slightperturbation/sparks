#include "SoftTestDeclarations.hpp"

#include "Display.hpp"
#include "Renderable.hpp"
#include "Projection.hpp"
#include "Viewport.hpp"
#include "TextureManager.hpp"
#include "RenderTarget.hpp"
#include "FileAssetFinder.hpp"

#include "Spark.hpp"
#include "PointSparkRenderable.hpp"

#include "LSpark.hpp"
#include "TexturedSparkRenderable.hpp"

#include "SlicedVolume.hpp"
#include "RayCastVolume.hpp"
#include "Fluid.hpp"

#include "Scene.hpp"
#include "ArcBall.hpp"

#include "GuiEventPublisher.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <IL/il.h>

#include <memory>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdlib.h>

using namespace std;
using namespace Eigen;
using namespace spark;

// Define the global Logger
cpplog::BaseLogger* g_log = NULL;
boost::mutex g_logGuard;
cpplog::BaseLogger* g_baseLogger = NULL;

// Need globals to allow GLFW callbacks access.
static spark::GuiEventPublisherPtr g_guiEventPublisher;
static spark::ArcBallPtr g_arcBall;

class InteractionVars
{
public:
    InteractionVars()
    : fps( 0 ),
      isSavingFrames( false )
    {
        lightPos[0] = 0.0f;
        lightPos[1] = 0.5f;
        lightPos[2] = 0.5f;
        lightColor[0] = 0.7f;
        lightColor[0] = lightColor[0] = 0.3f;
    }
    float lightPos[3];
    float lightColor[3];
    
    float fps;
    bool isSavingFrames;
};

void GLFWCALL resizeWindowCallback( int width, int height )
{
    if( g_guiEventPublisher )
    {
        g_guiEventPublisher->resizeViewport( 0, 0, width, height );
    }
}

void GLFWCALL mousePosCallback( int x, int y )
{
    if( !g_arcBall ) return;
    g_arcBall->onMouseMove( x, y );
}

void GLFWCALL mouseButtonCallback( int button, int action )
{
    if( !g_arcBall ) return;
    int x, y;
    glfwGetMousePos( &x, &y );
    if( button == GLFW_MOUSE_BUTTON_LEFT )
    {
        g_arcBall->onMouseRotationButton( x, y, GLFW_PRESS == action );
    }
    if( button == GLFW_MOUSE_BUTTON_RIGHT )
    {
        g_arcBall->onMouseDollyButton( x, y, GLFW_PRESS == action );
    }
    if( button == GLFW_MOUSE_BUTTON_MIDDLE )
    {
        g_arcBall->onMouseTrackingButton( x, y, GLFW_PRESS == action );
    }
}

void setGLFWCallbacks( void )
{
    // Set GLFW event callbacks
    // - Redirect window size changes to the callback function WindowSizeCB
    glfwSetWindowSizeCallback( resizeWindowCallback );
    glfwSetMousePosCallback( mousePosCallback );
    glfwSetMouseButtonCallback( mouseButtonCallback );
#ifdef __APPLE__
    glGetError(); // eat spurious GLFW-caused OpenGL errors on OSX/iOS
#endif
}

/// Example of creating a full-screen quad for HUD-style overlay
MeshPtr createOverlayQuad( TextureManagerPtr tm,
                           ShaderManagerPtr sm,
                           const TextureName& textureName,
                           const RenderPassName& renderPassName,
                           const ShaderName& shaderName )
{
    using namespace Eigen;
    MeshPtr overlay( new Mesh() );
    overlay->name( "Overlay_Quad" );
    overlay->addQuad( Vector3f(0,0,0), Vector2f(0,1), // Lower Left
                      Vector3f(1,0,0), Vector2f(1,1), // Lower Right
                      Vector3f(0,1,0), Vector2f(0,0), // Upper Left
                      Vector3f(1,1,0), Vector2f(1,0), // Upper Right
                      Vector3f(0,0,-1) ); // Normal points according to right-hand system
    overlay->setRequireExplicitMaterial( true );
    ShaderInstancePtr colorShader = sm->createShaderInstance( shaderName );
    MaterialPtr colorMaterial( new Material( tm, colorShader ) );
    colorMaterial->addTexture( "s_color", textureName );
    overlay->setMaterialForPassName( renderPassName, colorMaterial );
    return overlay;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Replace with loading from file (or running a script, python? lua?)
void loadTextures( TextureManagerPtr textureManager )
{
    textureManager->loadCheckerTexture( "checker" );
    textureManager->loadTestTexture( "test" );
    textureManager->loadTextureFromImageFile( "esuColor", "ESU_alpha_square.png" );
    textureManager->loadTextureFromImageFile( "cat", "sample.png" );
    textureManager->loadTextureFromImageFile( "skinColor", "skin_tile.jpg" );
    textureManager->loadTextureFromImageFile( "sparkColor", "sparkCircularGradient.png" );
}

void loadShaders( ShaderManagerPtr shaderManager )
{
    shaderManager->loadShaderFromFiles( "densityShader",
                                        "base3DVertexShader.glsl",
                                        "density3DFragmentShader.glsl" );

    shaderManager->loadShaderFromFiles( "rayCastVolumeShader",
                                        "rayCastVertexShader.glsl",
                                        "rayCastFragmentShader.glsl" );

    shaderManager->loadShaderFromFiles( "colorShader",
                                        "baseVertexShader.glsl",
                                        "colorFragmentShader.glsl" );
    
    shaderManager->loadShaderFromFiles( "constantColorShader",
                                        "baseVertexShader.glsl",
                                        "constantColorFragmentShader.glsl" );

    shaderManager->loadShaderFromFiles( "phongShader",
                                        "baseVertexShader.glsl",
                                        "phongFragmentShader.glsl" );
    
    shaderManager->loadShaderFromFiles( "texturedOverlayShader",
                                        "baseVertexShader.glsl",
                                        "texturedOverlayFragmentShader.glsl" );

    shaderManager->loadShaderFromFiles( "texturedSparkShader", 
                                        "baseVertexShader.glsl",
                                        "texturedSparkFragmentShader.glsl" );
}

MaterialPtr loadPhongMaterial( TextureManagerPtr textureManager,
                               ShaderManagerPtr shaderManager )
{
    ShaderInstancePtr phongShader = shaderManager
        ->createShaderInstance( "phongShader" );
    MaterialPtr phongMaterial( new Material( textureManager, phongShader ) );
    // Bind texture to the color sampler (s_color is used in shader)
    phongMaterial->addTexture( "s_color", "checker" );
    //Todo these uniforms should be set by illumination in scene?
    phongMaterial->setShaderUniform( "u_light.position_camera",
                                    glm::vec4(5.0f,10.0f,0.0f,1.0f) );
    phongMaterial->setShaderUniform( "u_light.diffuse",
                                    glm::vec4(0.8f,0.8f,0.8f,1.0f) );
    phongMaterial->setShaderUniform( "u_ambientLight",
                                    glm::vec4(0.3f,0.1f,0.1f,1.0f) );
    // Phong/Blinn material properties
    phongMaterial->setShaderUniform( "u_ka",
                                    glm::vec4(1.0f,1.0f,1.0f,1.0f) );
    phongMaterial->setShaderUniform( "u_kd",
                                    glm::vec4(1.0f,1.0f,1.0f,1.0f) );
    phongMaterial->setShaderUniform( "u_ks",
                                    glm::vec4(1.0f,1.0f,1.0f,1.0f) );
    phongMaterial->setShaderUniform( "u_ns", 100.0f );
    return phongMaterial;
}
////////////////////////////////////////////////////////////////////////////////

void loadScene( ScenePtr scene,
                FileAssetFinderPtr finder,
                TextureManagerPtr textureManager,
                ShaderManagerPtr shaderManager )
{
    MaterialPtr phongMaterial = loadPhongMaterial( textureManager,
                                                   shaderManager );
    if( false )
    {
        std::vector< MeshPtr > meshes;
        createMeshesFromFile( "stomach.obj", finder, meshes );
        for( auto meshItr = meshes.begin();
             meshItr != meshes.end(); ++meshItr )
        {
            MeshPtr mesh = *meshItr;
            mesh->setMaterialForPassName( g_opaqueRenderPassName,
                                          phongMaterial );
            scene->add( mesh );
        }
    }
    if( false )
    {
        MeshPtr box( new Mesh() );
        box->unitCube();
        box->name("TestBox");
        box->setMaterialForPassName( g_opaqueRenderPassName, phongMaterial );
        glm::mat4 xform( 1.0f );
        xform = glm::translate( xform, glm::vec3( -2.0f, 0.0f, -2.0f ) );
        xform = glm::scale( xform, glm::vec3(5.0f, 0.5f, 5.0f) );
        xform = glm::rotate( xform, 90.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) ) ;
        box->setTransform( xform );
        scene->add( box );
    }
}

/// Online simulation and display of fluid
int runSimulation(int argc, char** argv)
{
    OpenGLWindow window( "Spark" );
    using namespace std;
    InteractionVars vars;
    
    // Setup Gui Callbacks
    g_guiEventPublisher = GuiEventPublisherPtr( new GuiEventPublisher );
    setGLFWCallbacks();
        
    // Tell managers where to find file resources
    FileAssetFinderPtr finder( new FileAssetFinder );
    finder->addRecursiveSearchPath( DATA_PATH );
    ShaderManagerPtr shaderManager( new ShaderManager );
    shaderManager->setAssetFinder( finder );
    TextureManagerPtr textureManager( new TextureManager );
    textureManager->setAssetFinder( finder );
    
    int width = 0; int height = 0; glfwGetWindowSize( &width, &height );

    g_arcBall = ArcBallPtr( new spark::ArcBall );
    g_arcBall->resizeViewport( 0, 0, width, height );
    g_guiEventPublisher->subscribe( g_arcBall );

    ScenePtr scene( new Scene );
    loadTextures( textureManager );
    loadShaders( shaderManager );
    loadScene( scene, finder, textureManager, shaderManager );
    
    OrthogonalProjectionPtr overlayPerspective( new OrthogonalProjection );
    PerspectiveProjectionPtr cameraPerspective( new PerspectiveProjection );
    cameraPerspective->cameraPos( 1.0f, 4.0f, -5.0f );
    cameraPerspective->cameraTarget( 0.5f, 0.5f, 0.0f );

    //////////////////////////////////////////////////////////////////////////
    // Define render passes
    //////////////////////////////////////////////////////////////////////////
    // Pass to render directly to framebuffer
    FrameBufferRenderTargetPtr frameBufferTarget(
        new FrameBufferRenderTarget( width, height ) );
    frameBufferTarget->initialize( textureManager );
    frameBufferTarget->setClearColor( glm::vec4( 0,1,0,0 ) );
    g_guiEventPublisher->subscribe( frameBufferTarget );

    // Render a full-screen quad to the framebuffer -- does the final drawing
    // "MainRenderTargetTexture" texture holds the previously rendered scene
    {
        RenderPassPtr msaaRenderPass( new RenderPass( "MSAAFinalRenderPass" ) );
        msaaRenderPass->initialize( frameBufferTarget, 
                                    overlayPerspective,
                                    0.0f );
        //msaaRenderPass->disableBlending();  //useInterpolatedBlending();
        msaaRenderPass->useAdditiveBlending();
        msaaRenderPass->setDepthTest(false);
        scene->add( msaaRenderPass );
        // Create full-screen quad to render MainRenderTargetTexture
        // with the MSAAFinalRenderPass
        MeshPtr msaaOverlay = createOverlayQuad( textureManager,
                                                 shaderManager,
                                                 "MainRenderTargetTexture",
                                                 "MSAAFinalRenderPass",
                                                 "texturedOverlayShader" );
        scene->add( msaaOverlay );
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // Setup Standard render passes
    //////////////////////////////////////////////////////////////////////////
    // mainRenderTarget is the target for final composition.
    {
        const int multiSampleAntiAliasingFactor = 1;
        ScaledTextureRenderTargetPtr mainRenderTarget(
            new ScaledTextureRenderTarget( "MainRenderTargetTexture",
                                           width, height,
                                           multiSampleAntiAliasingFactor,
                                           multiSampleAntiAliasingFactor ) );
        mainRenderTarget->initialize( textureManager );
        mainRenderTarget->setClearColor( glm::vec4( 0.5,0.2,0.2,0 ) );
        // g_opaqueRenderPassName and g_transparencyRenderPassName
        if(true)
        {
            RenderPassPtr opaqueRenderPass(
                new RenderPass( g_opaqueRenderPassName ) );
            opaqueRenderPass->initialize( mainRenderTarget,
                                          cameraPerspective, 1.0f );
            opaqueRenderPass->setDepthWrite( true );
            opaqueRenderPass->setDepthTest( true );
            opaqueRenderPass->disableBlending();
            //opaqueRenderPass->nearToFarRenderOrder();
            scene->add( opaqueRenderPass );
        }
        // render transparent objects after opaque
        if( true )
        {
            RenderPassPtr transparencyRenderPass(
                new RenderPass( g_transparencyRenderPassName ) );
            transparencyRenderPass->initialize( mainRenderTarget,
                                                cameraPerspective, 0.9f );
            transparencyRenderPass->setDepthWrite( false );
            transparencyRenderPass->setDepthTest( true );
            transparencyRenderPass->useAdditiveBlending();
            //transparencyRenderPass->farToNearRenderOrder();
            scene->add( transparencyRenderPass );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    // Setup Glow effect
    // Make it easier to render to an off-screen texture that gets composited
    // to main render pass?
    if( false )
    {
        const RenderPassName glowPassName = "GlowRenderPass";
        const TextureName glowTargetTextureName = "GlowRenderPassTexture";
        {
            float downSampleFactor = 0.25f;
            // Render target to glows to.
            // Down-sample to get a cheap blur effect
            ScaledTextureRenderTargetPtr glowRenderTargetTexture(
                new ScaledTextureRenderTarget( glowTargetTextureName,
                                               width, height,
                                               downSampleFactor,
                                               downSampleFactor ) );
            glowRenderTargetTexture->setClearColor( glm::vec4( 0.2,0.2,0.5,1 ) );
            glowRenderTargetTexture->initialize( textureManager );
            // Listen to viewport resize events to resize texture
            g_guiEventPublisher->subscribe( glowRenderTargetTexture );

            // Render the world to the depth buffer, using a trivial default material
            if( false )
            {
                RenderPassPtr preGlowRenderPass( new RenderPass( "PreGlowPassName" ) );
                MaterialPtr blankMaterial(
                    new Material( textureManager,
                    ShaderInstancePtr( new ShaderInstance(
                        "constantColorShader", shaderManager ) ) ) );
                preGlowRenderPass->disableBlending();
                preGlowRenderPass->setColorWrite( false );
                preGlowRenderPass->setDepthWrite( true );
                preGlowRenderPass->useDefaultMaterial( blankMaterial );
                preGlowRenderPass->initialize( glowRenderTargetTexture,
                                               cameraPerspective,
                                               1.2f );
                scene->add( preGlowRenderPass );
            }
            
            // Render pass to render the glowing stuff to the target texture
            if( true )
            {
                RenderPassPtr glowRenderPass( new RenderPass( glowPassName ) );
                glowRenderPass->initialize( glowRenderTargetTexture,
                                            cameraPerspective,
                                            1.1f );
                glowRenderPass->setDepthWrite( false );
                glowRenderPass->useAdditiveBlending();
                scene->add( glowRenderPass );
            }
            // Create full-screen quad for overlay
//            MeshPtr glowOverlay = createOverlayQuad( textureManager,
//                                                    shaderManager,
//                                                    glowTargetTextureName,
//                                                    g_effectOverlayRenderPassName,
//                                                    "texturedOverlayShader" );
//            glowOverlay->setRequireExplicitMaterial(true);
//            scene->add( glowOverlay );
        }
    }



    /// Sparks (glowing)
    // spark -> [camera, texture]
    // overlay -> [ortho, framebuffer]
    
    
//    // To add glow to sparks, we render the sparks to a texture target
//    TextureRenderTargetPtr sparkRenderTarget( new TextureRenderTarget(
//        "SparkRenderTargetTexture", width, height ) );
//    sparkRenderTarget->initialize( textureManager );
//    // And then render this texture target as an overlay quad in the transparency pass
//    // using a glow/blur fragment shader
//    {
//        RenderPassName overlayCompositeRenderPassName = "OverlayCompositeRenderPass";
//        RenderPassPtr overlayCompositeRenderPass( new RenderPass( overlayCompositeRenderPassName ) );
//        overlayCompositeRenderPass->initialize( frameBufferTarget, //mainRenderTarget,
//                                                overlayPerspective,
//                                                0.01f );
//        MeshPtr sparkOverlay = createOverlayQuad( textureManager,
//                                                  shaderManager,
//                                                  "SparkRenderTargetTexture",
//                                                  overlayCompositeRenderPassName,
//                                                  "texturedOverlayShader" );
//        scene->add( sparkOverlay );
//    }
//    // Add a render-pass to render the spark to the render-texture
//    RenderPassPtr sparkRenderPass( new RenderPass( "SparkRenderPass" ) );
//    overlayRenderPass->initialize( sparkRenderTarget, cameraPerspective, 0.1f );
//    // The spark

    // Build material for spark

    if( true )
    {
        MaterialPtr sparkColorMaterial;
        bool usePhong = false;
        if( usePhong )
        {
            sparkColorMaterial = loadPhongMaterial( textureManager,
                                                   shaderManager );
        }
        else
        {
            sparkColorMaterial = MaterialPtr( new Material( textureManager,
                                                           ShaderInstancePtr( new ShaderInstance( "texturedSparkShader",
                                                                                                 shaderManager ) ) )
                                             );
        }
        sparkColorMaterial->setShaderUniform( "u_color", glm::vec4(1,1,1,1) );
        sparkColorMaterial->addTexture( "s_color", "sparkColor" );
        
        LSparkPtr theSpark( new LSpark );
        theSpark->create( Vector3f(4.9f,0,0), Vector3f(-4.9f,0,0), 1.0f, 5 );
        
        TexturedSparkRenderablePtr sparkRenderable( 
            new TexturedSparkRenderable( theSpark ) );
        sparkRenderable->setRequireExplicitMaterial( true );
        
//        sparkRenderable->setMaterialForPassName( "GlowRenderPass",
//                                                 sparkColorMaterial );
        sparkRenderable->setMaterialForPassName( g_transparencyRenderPassName,
                                                 sparkColorMaterial );
        scene->add( sparkRenderable );
    }
    if( true )
    {
        MaterialPtr phongMaterial = loadPhongMaterial( textureManager,
                                                       shaderManager );
        MaterialPtr colorMaterial = MaterialPtr( new Material( textureManager,
            ShaderInstancePtr( new ShaderInstance( "constantColorShader",
                                                   shaderManager ) ) )
                                         );
        colorMaterial->setShaderUniform( "u_color", glm::vec4(1,1,0.3,1) );
        phongMaterial->addTexture( "s_color", "skinColor" );
        MeshPtr s( new Mesh() );
        s->unitCube();
        s->name("SkinSurface");
        //s->setMaterialForPassName( g_opaqueRenderPassName, colorMaterial );
        s->setMaterialForPassName( g_opaqueRenderPassName, phongMaterial );
        glm::mat4 xform( 1.0f );
        xform = glm::translate( xform, glm::vec3( -2.0f, 0.0f, -2.0f ) );
        xform = glm::scale( xform, glm::vec3(5.0f, 0.5f, 5.0f) );
        xform = glm::rotate( xform, 90.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) ) ;
        s->setTransform( xform );
        scene->add( s );
    }

    
    
    // spark renders to sparkRenderTexture
    // overlay renders to g_transparentRenderPass using a glow shader
    
    FluidPtr fluidData( new Fluid(16) );
    //fluidData->loadFromFile( "test.fluid" );
        std::shared_ptr< spark::SlicedVolume > slices( new
            SlicedVolume( textureManager,
            shaderManager, 128, fluidData ) );

//    RayCastVolumePtr rayCastFluid( new RayCastVolume( "fluid_raycastvolume",
//                                               textureManager,
//                                               shaderManager,
//                                               fluidData ) );
    
    glm::mat4 xform = glm::translate( glm::mat4(1.0f), glm::vec3( -2.5f, -2.5f, 0.0f ) );
    xform = glm::scale( xform, glm::vec3(5.0f, 5.0f, 5.0f) );
    //rayCastFluid->setTransform( xform );
    //r    slices->setTransform( xform );

    /////////////////
    //scene->add( rayCastFluid );
    scene->add( slices );
    
    // sim->add( slices OR fluidData );
    
    const double startTime = glfwGetTime();
    double currTime = startTime;
    double lastTime = startTime;
    while( window.isRunning() )
    {
        LOG_TRACE(g_log) << ".................................................";

        glm::mat4 viewTransform;

        lastTime = currTime;
        currTime = glfwGetTime();
        vars.fps = 1.0f/(currTime - lastTime);

        // UPDATE
        const float dt = 1.0f/60.0f;
        //testSpark->update( dt );
        //fluidData->update( dt );
        //        slices->update( dt );
        //rayCastFluid->update( dt );
        
        if( g_arcBall )
        {
            g_arcBall->updatePerspective( cameraPerspective );
        }
        // Point slices to camera
        //slices->setCameraDirection(  cameraPerspective->cameraTarget()
        //                           - cameraPerspective->cameraPos() );

        scene->prepareRenderCommands();

        scene->render();

        glfwSwapBuffers();
            
        LOG_TRACE(g_log) << "Scene end - glfwSwapBuffers()";

        if( vars.isSavingFrames ) writeFrameBufferToFile( "sparks_" );

        //////////////////////////////////////////////////////////////////////////
        // Process Inputs
        if( glfwGetKey( GLFW_KEY_ENTER ) == GLFW_PRESS )
        {
            LOG_INFO(g_log) << "Reloading all shaders";
            shaderManager->reloadAllShaders();
        }
        if( glfwGetKey( GLFW_KEY_ESC ) == GLFW_PRESS )
        {
            break;
        }
        if( glfwGetKey( GLFW_KEY_BACKSPACE ) == GLFW_PRESS )
        {
            LOG_DEBUG(g_log) << "Reset sim.\n";
        }
        if( glfwGetKey( GLFW_KEY_F1 ) == GLFW_PRESS )
        {
            boost::mutex::scoped_lock lock( g_logGuard );
            delete g_log;
            g_log = new cpplog::FilteringLogger( LL_TRACE, g_baseLogger );
        }
        if( glfwGetKey( GLFW_KEY_F2 ) == GLFW_PRESS )
        {
            boost::mutex::scoped_lock lock( g_logGuard );
            delete g_log;
            g_log = new cpplog::FilteringLogger( LL_DEBUG, g_baseLogger );
        }
        if( glfwGetKey( GLFW_KEY_F3 ) == GLFW_PRESS )
        {
            boost::mutex::scoped_lock lock( g_logGuard );
            delete g_log;
            g_log = new cpplog::FilteringLogger( LL_INFO, g_baseLogger );
        }

        //const float eyeSeparationStep = 0.02;
        if( glfwGetKey( GLFW_KEY_UP ) == GLFW_PRESS )
        {
            //SideBySideDisplay* d = dynamic_cast<SideBySideDisplay*>(g_display);
            //if( d )
            //{
            //    float dist = d->getEyeSeparation() + eyeSeparationStep;
            //    d->setEyeSeparation( dist );
            //    LOG_DEBUG(g_log) << "Increasing interoccular distance to " << dist << "\n";
            //}
        }
        if( glfwGetKey( GLFW_KEY_DOWN ) == GLFW_PRESS )
        {
            //SideBySideDisplay* d = dynamic_cast<SideBySideDisplay*>(g_display);
            //if( d )
            //{
            //    float dist = d->getEyeSeparation() - eyeSeparationStep;
            //    d->setEyeSeparation( dist );
            //    LOG_DEBUG(g_log) << "Decreasing interoccular distance to " << dist << "\n";
            //}
        }
    }
    textureManager->releaseAll();
    return 0;
}

int main( int argc, char* argv[] )
{
#if defined(_DEBUG) && defined(WIN32)
    int debugFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    //_CrtSetDbgFlag( debugFlags | _CRTDBG_CHECK_ALWAYS_DF |  _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  // SLOWEST
    //_CrtSetDbgFlag( debugFlags | _CRTDBG_CHECK_EVERY_1024_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); // SLOW
    //_CrtSetDbgFlag( debugFlags | _CRTDBG_DELAY_FREE_MEM_DF );
    _CrtSetDbgFlag( debugFlags |  _CRTDBG_LEAK_CHECK_DF ); // FAST
#endif 
    
    {
#ifdef CPPLOG_THREADING
        boost::mutex::scoped_lock lock( g_logGuard );
        cpplog::BackgroundLogger backgroundLogger( g_baseLogger );
        g_baseLogger = &backgroundLogger;
#endif // CPPLOG_THREADING
        if( argc > 2 && std::string(argv[1]) == std::string("-stderr") )
        {
            g_baseLogger = new cpplog::StdErrLogger;
        } else {
            g_baseLogger = new cpplog::FileLogger( "sparks.log" );
        }
        if( argc > 2 && std::string(argv[1]) == std::string("-trace") )
        {
            g_log = new cpplog::FilteringLogger( LL_TRACE, g_baseLogger );
            LOG_INFO(g_log) << "TRACE level logging.";
        } else {
            g_log = new cpplog::FilteringLogger( LL_DEBUG, g_baseLogger );
            LOG_INFO(g_log) << "DEBUG level logging.";
        }
    }
    runSimulation( argc, argv );
    {
        boost::mutex::scoped_lock lock( g_logGuard );
        delete g_log;
        delete g_baseLogger;
    }
}

