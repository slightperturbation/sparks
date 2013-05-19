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
#include "Fluid.hpp"

#include "Scene.hpp"
#include "ArcBall.hpp"

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

// Define the global Logger
cpplog::BaseLogger* g_log = NULL;
boost::mutex g_logGuard;
cpplog::BaseLogger* g_baseLogger = NULL;

// Need globals to allow GLFW callbacks access.
// Why don't GLFW callbacks have a void* userData? (Apparently coming in V3)
static RenderTargetPtr g_frameBufferTarget;
static spark::ArcBall* g_arcBall = nullptr;

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
    if( g_frameBufferTarget )
    {
        g_frameBufferTarget->resizeViewport( 0, 0, width, height );
    }
    if( g_arcBall )
    {
        g_arcBall->setExtents( 0, 0, width, height );
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
    if( button == GLFW_MOUSE_BUTTON_LEFT ) // MIDDLE )
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
    glGetError(); // eat spurious OpenGL errors on OSX/iOS
#endif
}

/// Example of creating a full-screen quad for HUD-style overlay
MeshPtr createOverlayQuad( TextureManagerPtr tm,
                           ShaderManagerPtr sm,
                           const TextureName& textureName,
                           const RenderPassName& renderPassName )
{
    using namespace Eigen;
    MeshPtr overlay( new Mesh() );
    overlay->name( "Overlay_Quad" );
    overlay->addQuad( Vector3f(0,0,0), Vector2f(0,1), // Lower Left
                      Vector3f(1,0,0), Vector2f(1,1), // Lower Right
                      Vector3f(0,1,0), Vector2f(0,0), // Upper Left
                      Vector3f(1,1,0), Vector2f(1,0), // Upper Right
                      Vector3f(0,0,-1) ); // Normal points according to right-hand system

    ShaderName colorShaderName = overlay->name() + "_ColorShader";
    sm->loadShaderFromFiles( colorShaderName, 
                            "colorVertexShader.glsl",
                            "colorFragmentShader.glsl" );
    //TODO - Overlay Quad -- add texture and transparency

    ShaderInstancePtr colorShader( new ShaderInstance( colorShaderName, sm ) );
    colorShader->setUniform( "u_color", glm::vec4(1,1,1,1) );
    
    MaterialPtr colorMaterial( new Material( tm, colorShader ) );
    colorMaterial->addTexture( textureName, "s_color" );
    overlay->setMaterialForPassName( renderPassName, colorMaterial );
    return overlay;
}

/// Online simulation and display of fluid
int runSimulation(int argc, char** argv)
{
    OpenGLWindow window( "Spark" );
    {
        using namespace std;
        InteractionVars vars;
        g_arcBall = new spark::ArcBall;
        setGLFWCallbacks();
        
        // Tell managers where to find file resources
        FileAssetFinderPtr finder( new FileAssetFinder );
        finder->addRecursiveSearchPath( DATA_PATH );
        ShaderManagerPtr shaderManager( new ShaderManager );
        shaderManager->setAssetFinder( finder );
        TextureManagerPtr textureManager( new TextureManager );
        textureManager->setAssetFinder( finder );
        
        int width = 0; int height = 0; glfwGetWindowSize( &width, &height );
        RenderTargetPtr frameBufferTarget( new FrameBufferRenderTarget( width, height ) );
        // Expose the frameBufferTarget as a global to allow GLFW callbacks access
        g_frameBufferTarget = frameBufferTarget;
        frameBufferTarget->initialize( textureManager );

        //////////////////////////////////////////////////////////////////////////
        // Define render passes
        ScenePtr scene( new Scene );

        //TODO define "common" render pass and shader names for pre-load
        const RenderPassName directRenderPassName( "DirectRenderPass" );
        RenderPassPtr directRenderPass( new RenderPass( directRenderPassName ) );
        PerspectiveProjectionPtr cameraPerspective( new PerspectiveProjection );
        cameraPerspective->cameraPos( 1.0f, 4.0f, -5.0f );
        cameraPerspective->cameraTarget( 0.5f, 0.5f, 0.0f );
        directRenderPass->initialize( frameBufferTarget, cameraPerspective, 1.0f );
        scene->add( directRenderPass );

        RenderPassName overlayRenderPassName( "OverlayRenderPass" );
        RenderPassPtr overlayRenderPass( new RenderPass( overlayRenderPassName ) );
        OrthogonalProjectionPtr overlayPerspective( new OrthogonalProjection );
        overlayRenderPass->initialize( frameBufferTarget, overlayPerspective, 100.0f );
        scene->add( overlayRenderPass );

        //////////////////////////////////////////////////////////////////////////
        // Create Objects
        
        const TextureName overlayTextureName( "overlay_color" );
        textureManager->loadTextureFromImageFile( "ESU_alpha_square.png", overlayTextureName );
        //tm->loadTextureFromImageFile( "sample.png", "overlay2" );
        textureManager->loadCheckerTexture( "checker" );
        textureManager->loadTestTexture( "test" );

        MeshPtr overlay = createOverlayQuad( textureManager,
                                             shaderManager,
                                             overlayTextureName,
                                             overlayRenderPassName );
        glm::mat4 xform = glm::translate( glm::mat4(1.0f), glm::vec3( 0.5f, 0.5f, 0.0f ) );
        xform = glm::rotate( xform, 30.0f, glm::vec3( 0.0f, 0.0f, 1.0f ) ) ;
        xform = glm::translate( xform, glm::vec3( -0.5f, -0.5f, 0.0f ) );
        xform = glm::scale( xform, glm::vec3(0.5f, 0.5f, 1.0f) );
        overlay->setTransform( xform );
        scene->add( overlay );

        // Load assets
        {
            const ShaderName colorShaderName = "colorShader";
            shaderManager->loadShaderFromFiles( colorShaderName, "colorVertexShader.glsl", "colorFragmentShader.glsl");
            ShaderInstancePtr colorShader = shaderManager->createShaderInstance( colorShaderName );
            MaterialPtr colorMaterial( new Material( textureManager, colorShader ) );
            colorMaterial->addTexture( "checker", "s_color" );
        }
        {
            const ShaderName shaderName = "phongShader";
            shaderManager->loadShaderFromFiles( shaderName,
                                               "baseVertexShader.glsl",
                                               "phongFragmentShader.glsl" );
            ShaderInstancePtr phongShader = shaderManager->createShaderInstance( shaderName );
            MaterialPtr phongMaterial( new Material( textureManager, phongShader ) );
            const TextureName colorTexture = "color_skin";
            textureManager->loadTextureFromImageFile( "skin_tile.jpg",
                                                      colorTexture );
            // Bind texture to the color sampler (s_color is used in shader)
            phongMaterial->addTexture( "checker", "s_color" );
            //Todo uniforms should be set by illumination in scene
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
#ifdef WIN32
            std::vector< MeshPtr > meshes;
            createMeshesFromFile( "liver.3ds", finder, meshes );
            for( auto meshItr = meshes.begin(); meshItr != meshes.end(); ++meshItr )
            {
                MeshPtr mesh = *meshItr;
                mesh->setMaterialForPassName( directRenderPassName, phongMaterial );
                scene->add( mesh );
            }
#else
            MeshPtr box( new Mesh() );
            box->unitCube();
            box->name("TestBox");
            box->setMaterialForPassName( directRenderPassName, phongMaterial );

            glm::mat4 xform( 1.0f );
            xform = glm::translate( xform, glm::vec3( -10.0f, 0.0f, -10.0f ) );
            xform = glm::scale( xform, glm::vec3(20.0f, 0.5f, 20.0f) );
            xform = glm::rotate( xform, 90.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) ) ;
            box->setTransform( xform );
            scene->add( box );
#endif
        }

        
        //RenderPassPtr primaryRenderPass( new RenderPass( g_colorRenderPassName ) );
        //primaryRenderPass->initialize( frameBufferTarget, camera );
        //scene->add( primaryRenderPass );

        //LSparkPtr spark( new LSpark );
        //spark->create( Vector3f(0.9f,0,0), Vector3f(-0.9f,0,0), 1.0f, 5 );
        //LOG_DEBUG(g_log) << "Finished creating spark.\n";
        //TexturedSparkRenderablePtr sparkRenderable( new TexturedSparkRenderable(spark, textureManager, shaderManager ) );
        //scene->add( sparkRenderable );

        //VolumeDataPtr data( new Fluid(16) );
        //MeshPtr slices( new SlicedVolume( 32, data ) );
        //scene->add( slices );


        // Setup shared rendering state
        // Enable alpha blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        
        float angle = 0.0f;
        float rotRate = 0.5f;
        std::string sliceBaseName("densityYSlice");
        std::string velSliceBaseName( "velocityYSlice");

        const double startTime = glfwGetTime();
        double currTime = startTime;
        double lastTime = startTime;
        while( window.isRunning() )
        {
            glm::mat4 viewTransform;

            lastTime = currTime;
            currTime = glfwGetTime();
            vars.fps = 1.0f/(currTime - lastTime);

            // UPDATE
            //const float dt = 1.0f/60.0f;
            //spark->update( dt );
            
            if( g_arcBall )
            {
                g_arcBall->updatePerspective( cameraPerspective );
            }

            scene->prepareRenderCommands();

            glClearColor( 0.5f, 0.3f, 0.3f, 0.0f );
            //glClearDepth( -10000.0f );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            LOG_TRACE(g_log) << "Scene start - glClear(COLOR|DEPTH)";

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
    }
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
//        boost::mutex::scoped_lock lock( g_logGuard );
//#ifdef CPPLOG_THREADING
//        cpplog::BackgroundLogger backgroundLogger( g_baseLogger );
//        g_baseLogger = &backgroundLogger;
//#endif // CPPLOG_THREADING
        if( argc > 2 && std::string(argv[1]) == std::string("-stderr") )
        {
            g_baseLogger = new cpplog::StdErrLogger;
        } else {
            g_baseLogger = new cpplog::FileLogger( "sparks.log" );
        }
        if( argc > 2 && std::string(argv[1]) == std::string("-trace") )
        {
            g_log = new cpplog::FilteringLogger( LL_TRACE, g_baseLogger );
        } else {
            g_log = new cpplog::FilteringLogger( LL_DEBUG, g_baseLogger );
        }
    }
    
    runSimulation( argc, argv );
    {
//        boost::mutex::scoped_lock lock( g_logGuard );
        delete g_log;
        delete g_baseLogger;
    }
}

