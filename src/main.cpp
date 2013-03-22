#include "Display.hpp"
#include "Renderable.hpp"
#include "RenderContext.hpp"
#include "Viewport.hpp"

#include "Spark.hpp"
#include "PointSparkRenderable.hpp"

#include "LSpark.hpp"
#include "TexturedSparkRenderable.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

Display* g_display;

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
        lightColor[0] = lightColor[0] = 0.3;
    }
    float lightPos[3];
    float lightColor[3];
    
    float fps;
    bool isSavingFrames;
};

void GLFWCALL resizeWindowCallback( int width, int height )
{
    if( g_display )
    {
        g_display->resizeWindow( width, height );
    }
}

/// Startup OpenGL and create the rendering context and window.
int initOpenGL( InteractionVars* vars )
{
    cerr << "glfwInit...";
    if( !glfwInit() )
    {
        cerr << "Failed to initialize GLFW.\n";
    }
    cerr << "done.\n";

    // OpenGL 3.2 or higher only
    glfwOpenWindowHint( GLFW_FSAA_SAMPLES, 16 ); // 8x anti aliasing
//#ifdef __APPLE__
//    // Need to force the 3.2 for mac -- note
//    // that this breaks the AntTweak menus
//    glfwOpenWindowHint( GLFW_OPENGL_VERSION_MAJOR, 3 );
//    glfwOpenWindowHint( GLFW_OPENGL_VERSION_MINOR, 2 );
//#endif
    glfwOpenWindowHint( GLFW_WINDOW_NO_RESIZE, GL_FALSE );
    
    cerr << "glfwOpenWindow...";
    glfwOpenWindow( 800, 400, 0, 0, 0, 0, 32, 0, GLFW_WINDOW ); //
    glfwEnable(GLFW_MOUSE_CURSOR);
    glfwSetWindowTitle( "softTest" );

    cerr << " done.\n";
    cerr << "glewInit... ";
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if( err != GLEW_OK )
    {
        cerr << "glewInit() failed!\n";
        return -1;
    }
    if( !glewIsSupported("GL_VERSION_3_2") )
    {
        cerr << "OpenGL Version 3.2 Required!\n";
        return -1;
    }
    if( GL_MAX_3D_TEXTURE_SIZE < 256 )
    {
        cerr << "Max 3d texture size: " << GL_MAX_3D_TEXTURE_SIZE << " is too small for this program.\n";
        return -1;
    }
    
    // Set GLFW event callbacks
    // - Redirect window size changes to the callback function WindowSizeCB
    glfwSetWindowSizeCallback(resizeWindowCallback);
    
    glEnable( GL_DEPTH_TEST );
    cerr << "done.\n";
    return 0;
}

void setupSimpleDisplay( RenderContext& renderContext )
{
    delete g_display;
    g_display = new SimpleDisplay(renderContext);
    int height = 0; int width = 0;
    glfwGetWindowSize( &width, &height );
    g_display->resizeWindow( width, height );
}

void setup3dDisplay( RenderContext& renderContext )
{
    delete g_display;
    g_display = new SideBySideDisplay(renderContext);
    int height = 0; int width = 0;
    glfwGetWindowSize( &width, &height );
    g_display->resizeWindow( width, height );
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

    glPixelStorei( GL_PACK_ALIGNMENT, 1 ); // align start of pixel row on byte

    std::stringstream frameFileName;
    frameFileName << frameBaseFileName << std::setfill('0') << std::setw(4) << frameNumber++ << ".ppm";
    std::ofstream frameFile( frameFileName.str().c_str(), std::ios::binary | std::ios::trunc );
    glReadBuffer( GL_FRONT ); 
    glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, frameBuffer );

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
    using namespace std;
    InteractionVars vars;

    RenderContext renderContext;

    int retVal = initOpenGL( &vars );
    if( retVal ) return retVal;

    setupSimpleDisplay(renderContext);
    //setup3dDisplay(renderContext);

    Renderables scene;
    
    //SparkPtr spark( new Spark );
    //spark->aggregate().push_back( PointCharge( 0, 0, 0 ) );
    
    //PointSparkRenderablePtr pointSparkRenderable( new PointSparkRenderable( spark ) );
    //scene.push_back( pointSparkRenderable );
    
    LSparkPtr spark( new LSpark() );
    spark->create( Vector3f(0,0,0), Vector3f(1,0,0), 1.0, 5 );
    
    float angle = 0.0f;

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
        renderContext.setModelMatrix(
            glm::translate( glm::rotate( glm::mat4(), angle, glm::vec3( 0.0f, 0.0f, 1.0f ) ), glm::vec3(-0.5, -0.5, -0.5) )
            );
        renderContext.setLightPosition( vars.lightPos );

        // UPDATE
        const float dt = 1.0f/60.0f;
        spark->update( dt );
        
        glClearColor( 0.5, 0.3, 0.3, 1 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        g_display->render( scene );
        
        if( vars.isSavingFrames ) writeFrameBufferToFile( "sparks_" );
        
        //////////////////////////////////////////////////////////////////////////
        // Process Inputs
        if( glfwGetKey( GLFW_KEY_ENTER ) == GLFW_PRESS )
        {
            //smokeVolume->loadShaders();
        }
        if( glfwGetKey( GLFW_KEY_ESC ) == GLFW_PRESS )
        {
            break;
        }
        if( glfwGetKey( GLFW_KEY_BACKSPACE ) == GLFW_PRESS )
        {
            std::cerr << "Reset sim.\n";
        }
        if( glfwGetKey( GLFW_KEY_F2 ) == GLFW_PRESS )
        {
            std::cerr << "Switching to 3d\n";
            setup3dDisplay(renderContext);
        }
        if( glfwGetKey( GLFW_KEY_F1 ) == GLFW_PRESS )
        {
            std::cerr << "Switching to simple\n";
            setupSimpleDisplay(renderContext);
        }

        const float eyeSeparationStep = 0.02;
        if( glfwGetKey( GLFW_KEY_UP ) == GLFW_PRESS )
        {
            SideBySideDisplay* d = dynamic_cast<SideBySideDisplay*>(g_display);
            if( d )
            {
                float dist = d->getEyeSeparation() + eyeSeparationStep;
                d->setEyeSeparation( dist );
                std::cerr << "Increasing interoccular distance to " << dist << "\n";
            }
        }
        if( glfwGetKey( GLFW_KEY_DOWN ) == GLFW_PRESS )
        {
            SideBySideDisplay* d = dynamic_cast<SideBySideDisplay*>(g_display);
            if( d )
            {
                float dist = d->getEyeSeparation() - eyeSeparationStep;
                d->setEyeSeparation( dist );
                std::cerr << "Decreasing interoccular distance to " << dist << "\n";
            }
        }
        glfwSwapBuffers();
    }
    glfwTerminate();
    return 0;
}

int main( int argc, char* argv[] )
{
    g_display = NULL;
    return runSimulation( argc, argv );
}

