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

/// Write a 24-bit color binary PPM image file for the current frame buffer
/// files are named sequentially starting at 1, padded to 4 digits.
void writeFrameBufferToFile( const std::string& frameBaseFileName ) 
{
    static unsigned int frameNumber = 1;
    
    int width = 0;
    int height = 0;
    glfwGetWindowSize( &width, &height );

    unsigned char* frameBuffer = new unsigned char[ 3 * width * height * sizeof(unsigned char) ];

    GL_CHECK( glPixelStorei( GL_PACK_ALIGNMENT, 1 ) ); // align start of pixel row on byte

    std::stringstream frameFileName;
    frameFileName << frameBaseFileName << std::setfill('0') << std::setw(4) << frameNumber++ << ".ppm";
    std::ofstream frameFile( frameFileName.str().c_str(), std::ios::binary | std::ios::trunc );
    GL_CHECK( glReadBuffer( GL_FRONT ) ); 
    GL_CHECK( glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, frameBuffer ) );

    // PPM header.  P6 is binary RGB
    frameFile << "P6\n" << width << " " << height << "\n255\n";
    for( int j = height-1; j>=0; --j )  // opengl vs image is swapped top-bottom
    {
        for( int i = 0; i < width; ++i )
        {
            frameFile << (char)frameBuffer[3*width*j + 3*i + 0] 
            << (char)frameBuffer[3*width*j + 3*i + 1] 
            << (char)frameBuffer[3*width*j + 3*i + 2] 
            ;
        }
    }
    frameFile.close();
    delete[] frameBuffer;
}

MeshPtr createOverlayQuad( TextureManagerPtr tm, ShaderManagerPtr sm, const RenderPassName& renderPassName )
{
    using namespace Eigen;
    MeshPtr overlay( new Mesh() );
    overlay->name( "Overlay_Quad" );
    overlay->addQuad( Vector3f(0,0,0), Vector2f(0,0),
                      Vector3f(1,0,0), Vector2f(1,0), 
                      Vector3f(0,1,0), Vector2f(0,1), 
                      Vector3f(1,1,0), Vector2f(1,1),
                      Vector3f(0,0,-1) );
    ShaderName colorShaderName = overlay->name() + "_ColorShader";
    sm->loadShaderFromFiles( colorShaderName, 
        "colorVertexShader.glsl",
        "colorFragmentShader.glsl" );

    ShaderPtr colorShader( new Shader( colorShaderName, sm ) );
    colorShader->createUniform( "u_color", glm::vec4(1,0,0,0) );
    MaterialPtr colorMaterial( new Material( colorShader ) );
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
        setGLFWCallbacks();
        
        // Tell managers where to find file resources
        FileAssetFinderPtr finder( new FileAssetFinder( ) );
        finder->addRecursiveSearchPath( DATA_PATH );
        ShaderManagerPtr shaderManager( new ShaderManager( ) );
        shaderManager->setAssetFinder( finder );
        TextureManagerPtr textureManager( new TextureManager() );
        textureManager->setAssetFinder( finder );
        
        int width = 0; int height = 0; glfwGetWindowSize( &width, &height );
        //PerspectiveProjectionPtr camera( new PerspectiveProjection );
        //camera->aspectRatio( (float)width/height );
        //camera->cameraPos( glm::vec3( 3, 3, 3 ) );
        //camera->cameraTarget( glm::vec3( 0, 0, 0 ) );
        RenderTargetPtr frameBufferTarget( new FrameBufferRenderTarget( width, height ) );
        g_frameBufferTarget = frameBufferTarget;
        frameBufferTarget->initialize( textureManager );

        // Setup a simple render pipeline out to the framebuffer
        ScenePtr scene( new Scene );
        
        RenderPassName overlayRenderPassName( "OverlayRenderPass" );
        RenderPassPtr overlayRenderPass( new RenderPass( overlayRenderPassName ) );
        OrthogonalProjectionPtr overlayPerspective( new OrthogonalProjection );
        overlayRenderPass->initialize( frameBufferTarget, overlayPerspective, 10.0f );
        scene->add( overlayRenderPass );

        MeshPtr overlay = createOverlayQuad( textureManager, shaderManager, overlayRenderPassName );
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

        //bool useBox = false;
        //if( useBox )
        //{
        //    MeshPtr box( new Mesh() );
        //    box->unitCube();
        //    scene->add( box );
        //}
        float angle = 0.0f;
        float rotRate = 0.5f;
        std::string sliceBaseName("densityYSlice");
        std::string velSliceBaseName( "velocityYSlice");

        const double startTime = glfwGetTime();
        double currTime = startTime;
        double lastTime = startTime;
        while( glfwGetWindowParam( GLFW_OPENED ) )
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
                //sparkRenderable->loadShaders();
                //box->loadShaders();
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
        //boost::mutex::scoped_lock lock( g_logGuard );
        g_baseLogger = new cpplog::FileLogger( "sparks.log" );
        //g_baseLogger = new cpplog::StdErrLogger;
//#ifdef CPPLOG_THREADING
//        cpplog::BackgroundLogger backgroundLogger( g_baseLogger );
//        g_baseLogger = &backgroundLogger;
//#endif // CPPLOG_THREADING
        g_log = new cpplog::FilteringLogger( LL_TRACE, g_baseLogger );
    }
    
    runSimulation( argc, argv );

    //boost::mutex::scoped_lock lock( g_logGuard );
    delete g_log;
    delete g_baseLogger;
}

