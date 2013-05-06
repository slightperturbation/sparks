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

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

// Need this global to allow GLFW callbacks access.
// Why don't GLFW callbacks have a void* userData.  Explain to me.
static RenderTargetPtr g_frameBufferTarget;

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
}

void setGLFWCallbacks( void )
{
    // Set GLFW event callbacks
    // - Redirect window size changes to the callback function WindowSizeCB
    glfwSetWindowSizeCallback(resizeWindowCallback);
#ifdef __APPLE__
    glGetError(); // clear spurious OpenGL errors
#endif
}



/// Example of creating a full-screen quad for HUD-style overlay
MeshPtr createOverlayQuad( TextureManagerPtr tm, ShaderManagerPtr sm, const RenderPassName& renderPassName )
{
    using namespace Eigen;
    MeshPtr overlay( new Mesh() );
    overlay->name( "Overlay_Quad" );
    overlay->addQuad( Vector3f(0,0,0), Vector2f(0,1), // Lower Left
                      Vector3f(1,0,0), Vector2f(1,1), // Lower Right
                      Vector3f(0,1,0), Vector2f(0,0), // Upper Left
                      Vector3f(1,1,0), Vector2f(1,0), // Upper Right
                      Vector3f(0,0,-1) ); // Normal points according to right-hand system
    ///////////////////////

    ShaderName colorShaderName = overlay->name() + "_ColorShader";
    sm->loadShaderFromFiles( colorShaderName, 
        "colorVertexShader.glsl",
        "colorFragmentShader.glsl" );
    //TODO - Overlay Quad -- add texture and transparency

    ShaderInstancePtr colorShader( new ShaderInstance( colorShaderName, sm ) );
    colorShader->setUniform( "u_color", glm::vec4(1,1,1,1) );
    
    MaterialPtr colorMaterial( new Material( tm, colorShader ) );
    tm->loadTextureFromImageFile( "ESU_alpha.png", "ESU" );
    tm->loadTextureFromImageFile( "skin_tile.jpg", "skin" );
    colorMaterial->addTexture( "ESU", "s_color" );
    colorMaterial->addTexture( "skin", "s_color2" );
    overlay->setMaterialForPassName( renderPassName, colorMaterial );
    return overlay;
}

//RenderPassName createPostEffectRenderPass( TextureManagerPtr tm, 
//    ShaderManagerPtr sm, const std::string& name, ShaderInstancePtr shader )
//{
//    using namespace Eigen;
//    MeshPtr quad( new Mesh() );
//    quad->name( "postEffect_" + name );
//    quad->addQuad( Vector3f(0,0,0), Vector2f(0,1), // Lower Left
//        Vector3f(1,0,0), Vector2f(1,1), // Lower Right
//        Vector3f(0,1,0), Vector2f(0,0), // Upper Left
//        Vector3f(1,1,0), Vector2f(1,0), // Upper Right
//        Vector3f(0,0,-1) ); // Normal points according to right-hand system
//
//}


RenderablePtr createTestBox( TextureManagerPtr tm, ShaderManagerPtr sm, const RenderPassName& renderPassName )
{
    RenderablePtr box = Mesh::createBox( tm, sm, renderPassName );
    return box;
}

/// Online simulation and display of fluid
int runSimulation(int argc, char** argv)
{
    OpenGLWindow window( "Spark" );
    {
        using namespace std;
        InteractionVars vars;
        setGLFWCallbacks();
        
        // Tell managers where to find file resources
        FileAssetFinderPtr finder( new FileAssetFinder );
        finder->addRecursiveSearchPath( DATA_PATH );
        ShaderManagerPtr shaderManager( new ShaderManager );
        shaderManager->setAssetFinder( finder );
        TextureManagerPtr textureManager( new TextureManager );
        textureManager->setAssetFinder( finder );
        
        int width = 0; int height = 0; glfwGetWindowSize( &width, &height );
        //PerspectiveProjectionPtr camera( new PerspectiveProjection );
        //camera->aspectRatio( (float)width/height );
        //camera->cameraPos( glm::vec3( 3, 3, 3 ) );
        //camera->cameraTarget( glm::vec3( 0, 0, 0 ) );
        RenderTargetPtr frameBufferTarget( new FrameBufferRenderTarget( width, height ) );
        g_frameBufferTarget = frameBufferTarget;
        frameBufferTarget->initialize( textureManager );

        // Setup a simple render pipeline
        ScenePtr scene( new Scene );
        
        //////////////////////////////////////////////////////////////////////////
        // Define render passes

        //TODO define "common" render pass and shader names for pre-load
        const RenderPassName directRenderPassName( "DirectRenderPass" );
        RenderPassPtr directRenderPass( new RenderPass( directRenderPassName ) );
        PerspectiveProjectionPtr cameraPerspective( new PerspectiveProjection );
        cameraPerspective->cameraPos( 1.0f, 1.0f, -5.0f );
        cameraPerspective->cameraTarget( 0.5f, 0.5f, 0.0f );
        directRenderPass->initialize( frameBufferTarget, cameraPerspective, 1.0f );
        scene->add( directRenderPass );

        RenderPassName overlayRenderPassName( "OverlayRenderPass" );
        RenderPassPtr overlayRenderPass( new RenderPass( overlayRenderPassName ) );
        OrthogonalProjectionPtr overlayPerspective( new OrthogonalProjection );
        overlayRenderPass->initialize( frameBufferTarget, overlayPerspective, 10.0f );
        scene->add( overlayRenderPass );


        //////////////////////////////////////////////////////////////////////////
        // Create Objects
        MeshPtr overlay = createOverlayQuad( textureManager, shaderManager, overlayRenderPassName );
        glm::mat4 xform = glm::translate( glm::mat4(1.0f), glm::vec3( 0.5f, 0.5f, 0.0f ) );
        xform = glm::rotate( xform, 30.0f, glm::vec3( 0.0f, 0.0f, 1.0f ) ) ;
        xform = glm::translate( xform, glm::vec3( -0.5f, -0.5f, 0.0f ) );
        overlay->setTransform( xform );
        scene->add( overlay );


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

        bool useBox = true;
        if( useBox )
        {
            MeshPtr box( new Mesh() );
            box->unitCube();
            box->name("TestBox");

            /// TODO -- shader creation should be easier!
            const ShaderName shaderName = "colorShader";
            //shaderManager->loadShaderFromFiles( shaderName, "colorVertexShader.glsl", "colorFragmentShader.glsl" );
            //ShaderInstancePtr colorShader = shaderManager->createShaderInstance( shaderName );


            shaderManager->loadShaderFromFiles( shaderName, "colorVertexShader.glsl", "colorFragmentShader.glsl" );
            ShaderInstancePtr colorShader( new ShaderInstance( "colorShader", shaderManager ) );
            MaterialPtr colorMaterial( new Material( textureManager, colorShader ) );
            box->setMaterialForPassName( directRenderPassName, colorMaterial );
            scene->add( box );
        }
        float angle = 0.0f;
        float rotRate = 0.5f;
        std::string sliceBaseName("densityYSlice");
        std::string velSliceBaseName( "velocityYSlice");

        const double startTime = glfwGetTime();
        double currTime = startTime;
        double lastTime = startTime;
        while( window.isRunning() )
        {
            lastTime = currTime;
            currTime = glfwGetTime();
            vars.fps = 1.0f/(currTime - lastTime);
            angle += rotRate;
            //camera->setModelMatrix(
            //    glm::rotate( glm::mat4(), angle, glm::vec3( 0.0f, 1.0f, 0.0f ) ) ); 

            //    glm::translate( 
            //       glm::vec3(-0.5, -0.5, -0.5) )
            //    );
            //camera->setLightPosition( vars.lightPos );

            // UPDATE
            //const float dt = 1.0f/60.0f;
            //spark->update( dt );

            scene->prepareRenderCommands();
            scene->render();

            glfwSwapBuffers();

            if( vars.isSavingFrames ) writeFrameBufferToFile( "sparks_" );

            //////////////////////////////////////////////////////////////////////////
            // Process Inputs
            if( glfwGetKey( GLFW_KEY_ENTER ) == GLFW_PRESS )
            {
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
            glfwSwapBuffers();
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
        g_baseLogger = new cpplog::FileLogger( "sparks.log" );
        //g_baseLogger = new cpplog::StdErrLogger;
//#ifdef CPPLOG_THREADING
//        cpplog::BackgroundLogger backgroundLogger( g_baseLogger );
//        g_baseLogger = &backgroundLogger;
//#endif // CPPLOG_THREADING
        //g_log = new cpplog::FilteringLogger( LL_DEBUG, g_baseLogger );
        g_log = new cpplog::FilteringLogger( LL_TRACE, g_baseLogger );
    }
    
    runSimulation( argc, argv );
    {
//        boost::mutex::scoped_lock lock( g_logGuard );
        delete g_log;
        delete g_baseLogger;
    }
}

