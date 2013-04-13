#include "SoftTestDeclarations.hpp"

#include "Display.hpp"
#include "Renderable.hpp"
#include "Perspective.hpp"
#include "Viewport.hpp"
#include "TextureManager.hpp"
#include "RenderTarget.hpp"

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
cpplog::FileLogger g_log( "sparks.log" );


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

void APIENTRY debugOpenGLMessageCallback( GLenum source,
                                    GLenum type,
                                    GLuint id,
                                    GLenum severity,
                                    GLsizei length,
                                    const GLchar* message,
                                    void* userParam )
{
    using namespace std;
    LOG_INFO(g_log) << "GL Debug Message\n"
    << "Source: ";
    switch( source )
    {
        case GL_DEBUG_SOURCE_API:
            LOG_INFO(g_log) << "GL_DEBUG_SOURCE_API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            LOG_INFO(g_log) << "GL_DEBUG_SOURCE_WINDOW_SYSTEM"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            LOG_INFO(g_log) << "GL_DEBUG_SOURCE_SHADER_COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            LOG_INFO(g_log) << "GL_DEBUG_SOURCE_THIRD_PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION:
            LOG_INFO(g_log) << "GL_DEBUG_SOURCE_APPLICATION"; break;
        case GL_DEBUG_SOURCE_OTHER:
            LOG_INFO(g_log) << "GL_DEBUG_SOURCE_OTHER"; break;
        default:
            LOG_INFO(g_log) << "UNKNOWN";
    }
    LOG_INFO(g_log) << "\nType: ";
    switch( type )
    {
        case GL_DEBUG_TYPE_ERROR:
            LOG_INFO(g_log) << "GL_DEBUG_TYPE_ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            LOG_INFO(g_log) << "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            LOG_INFO(g_log) << "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY:
            LOG_INFO(g_log) << "GL_DEBUG_TYPE_PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            LOG_INFO(g_log) << "GL_DEBUG_TYPE_PERFORMANCE"; break;
        case GL_DEBUG_TYPE_OTHER:
            LOG_INFO(g_log) << "GL_DEBUG_TYPE_OTHER"; break;
        default:
            LOG_INFO(g_log) << "UNKNOWN";
    }
    LOG_INFO(g_log) << "\nSeverity: ";
    switch( severity )
    {
        case GL_DEBUG_SEVERITY_LOW:
            LOG_INFO(g_log) << "GL_DEBUG_SEVERITY_LOW"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            LOG_INFO(g_log) << "GL_DEBUG_SEVERITY_MEDIUM"; break;
        case GL_DEBUG_SEVERITY_HIGH:
            LOG_INFO(g_log) << "GL_DEBUG_SEVERITY_HIGH"; break;
        default:
            LOG_INFO(g_log) << "UNKNOWN";
    }
    LOG_INFO(g_log) << "\nMessage: \"" << message << "\".\n";
}


void GLFWCALL resizeWindowCallback( int width, int height )
{
    if( g_frameBufferTarget )
    {
        g_frameBufferTarget->resizeViewport( 0, 0, width, height );
    }
}

/// Startup OpenGL and create the rendering context and window.
int initOpenGL( InteractionVars* vars )
{
    LOG_INFO(g_log) << "glfwInit...";
    if( !glfwInit() )
    {
        LOG_FATAL(g_log)<< "Failed to initialize GLFW.\n";
    }
    LOG_INFO(g_log) << "done.\n";
    
#ifdef _DEBUG
    glfwOpenWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE );
#endif
    
    // OpenGL 3.2 or higher only
    glfwOpenWindowHint( GLFW_FSAA_SAMPLES, 8 ); // 8x anti aliasing
#ifdef __APPLE__
    // Need to force the 3.2 for mac -- note
    // that this breaks the AntTweak menus
    glfwOpenWindowHint( GLFW_OPENGL_VERSION_MAJOR, 3 );
    glfwOpenWindowHint( GLFW_OPENGL_VERSION_MINOR, 2 );
    //glfwOpenWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );  // Core profile not compatible with GLEW(?)
#endif
    glfwOpenWindowHint( GLFW_WINDOW_NO_RESIZE, GL_FALSE );
    
    LOG_INFO(g_log) << "glfwOpenWindow...";
    glfwOpenWindow( 800, 600, 0, 0, 0, 0, 0, 0, GLFW_WINDOW ); //
    glfwEnable( GLFW_MOUSE_CURSOR );
    glfwSetWindowTitle( "softTest" );
    checkOpenGLErrors();
    LOG_INFO(g_log) << " done.\n";
    LOG_INFO(g_log) << "glewInit... ";
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    // according to http://www.opengl.org/wiki/OpenGL_Loading_Library
    // glewInit may throw invalid enumerant error
    glGetError(); // eat glew's spurious error
    if( err != GLEW_OK )
    {
        LOG_INFO(g_log) << "glewInit() failed!\n";
        return -1;
    }
    if( !glewIsSupported("GL_VERSION_3_2") )
    {
        LOG_INFO(g_log) << "OpenGL Version 3.2 Required!\n";
        return -1;
    }
    checkOpenGLErrors();
    if( GL_MAX_3D_TEXTURE_SIZE < 256 )
    {
        LOG_INFO(g_log) << "Max 3d texture size: " << GL_MAX_3D_TEXTURE_SIZE << " is too small for this program.\n";
        return -1;
    }

    if( GL_ARB_debug_output ) //glewIsSupported( "GL_ARB_debug_output" ) )
    {
#ifdef _DEBUG
        glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
#else
        glEnable( GL_DEBUG_OUTPUT );
#endif
        LOG_INFO(g_log) << "Enabling OpenGL Debug messages.\n";
#ifndef __APPLE__
        glDebugMessageCallback( debugOpenGLMessageCallback, 0 );
        GLuint unusedIds = 0;
        glDebugMessageControl( GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DONT_CARE,
            0,
            &unusedIds,
            true);
#endif
    }
    
    ilInit();
    
    
    // Set GLFW event callbacks
    // - Redirect window size changes to the callback function WindowSizeCB
    glfwSetWindowSizeCallback(resizeWindowCallback);
#ifdef __APPLE__
    glGetError(); // clear spurious OpenGL errors
#endif
    checkOpenGLErrors();
    GL_CHECK( glEnable( GL_DEPTH_TEST ) );
    LOG_INFO(g_log) << "done.\n";
    return 0;
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

/// Online simulation and display of fluid
int runSimulation(int argc, char** argv)
{
    {
        using namespace std;
        InteractionVars vars;

        int retVal = initOpenGL( &vars );
        if( retVal ) return retVal;

        TextureManagerPtr textureManager( new TextureManager() );
        PerspectivePtr camera( new Perspective() );
        int width = 0; int height = 0; glfwGetWindowSize( &width, &height );
        RenderTargetPtr frameBufferTarget( new FrameBufferRenderTarget( width, height ) );
        g_frameBufferTarget = frameBufferTarget;
        frameBufferTarget->initialize( textureManager );

        // Setup a simple render pipeline out to the framebuffer
        ScenePtr scene( new Scene );
        
        RenderPassPtr primaryRenderPass( new RenderPass );
        primaryRenderPass->initialize( frameBufferTarget, camera );
        scene->add( primaryRenderPass );




        LSparkPtr spark( new LSpark );
        spark->create( Vector3f(0.9f,0,0), Vector3f(-0.9f,0,0), 1.0f, 5 );
        LOG_INFO(g_log) << "Finished creating spark.\n";
        TexturedSparkRenderablePtr sparkRenderable( new TexturedSparkRenderable(spark) );
        sparkRenderable->loadShaders();
        sparkRenderable->loadTextures();
        scene->add( sparkRenderable );

        VolumeDataPtr data( new Fluid(16) );
        MeshPtr slices( new SlicedVolume( 32, data ) );
        scene->add( slices );

        bool useBox = false;
        if( useBox )
        {
            MeshPtr box( new Mesh(DATA_PATH "/shaders/TexturedSparkVertex.glsl",
                DATA_PATH "/shaders/TexturedSparkFragment.glsl" ) );
            box->unitCube();
            box->loadShaders();
            scene->add( box );
        }
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
            camera->setModelMatrix(
                glm::rotate( glm::mat4(), angle, glm::vec3( 0.0f, 1.0f, 0.0f ) ) ); 
            //    glm::translate( 
            //       glm::vec3(-0.5, -0.5, -0.5) )
            //    );
            //camera->setLightPosition( vars.lightPos );

            // UPDATE
            //const float dt = 1.0f/60.0f;
            //spark->update( dt );


            scene->render();


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
                LOG_INFO(g_log) << "Reset sim.\n";
            }
            if( glfwGetKey( GLFW_KEY_F2 ) == GLFW_PRESS )
            {
                //LOG_INFO(g_log) << "Switching to 3d\n";
            }
            if( glfwGetKey( GLFW_KEY_F1 ) == GLFW_PRESS )
            {
                //LOG_INFO(g_log) << "Switching to simple\n";
            }

            const float eyeSeparationStep = 0.02;
            if( glfwGetKey( GLFW_KEY_UP ) == GLFW_PRESS )
            {
                //SideBySideDisplay* d = dynamic_cast<SideBySideDisplay*>(g_display);
                //if( d )
                //{
                //    float dist = d->getEyeSeparation() + eyeSeparationStep;
                //    d->setEyeSeparation( dist );
                //    LOG_INFO(g_log) << "Increasing interoccular distance to " << dist << "\n";
                //}
            }
            if( glfwGetKey( GLFW_KEY_DOWN ) == GLFW_PRESS )
            {
                //SideBySideDisplay* d = dynamic_cast<SideBySideDisplay*>(g_display);
                //if( d )
                //{
                //    float dist = d->getEyeSeparation() - eyeSeparationStep;
                //    d->setEyeSeparation( dist );
                //    LOG_INFO(g_log) << "Decreasing interoccular distance to " << dist << "\n";
                //}
            }
            glfwSwapBuffers();
        }
        textureManager->releaseAll();
    }
    glfwTerminate();
    return 0;
}

int main( int argc, char* argv[] )
{
#if defined(_DEBUG) && defined(WIN32)
    int debugFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    _CrtSetDbgFlag( debugFlags | _CRTDBG_CHECK_ALWAYS_DF |  _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  // SLOWEST
    //_CrtSetDbgFlag( debugFlags | _CRTDBG_CHECK_EVERY_1024_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); // SLOW
    //_CrtSetDbgFlag( debugFlags | _CRTDBG_DELAY_FREE_MEM_DF );
    //_CrtSetDbgFlag( debugFlags |  _CRTDBG_LEAK_CHECK_DF ); // FAST
#endif 
    
    return runSimulation( argc, argv );
}

