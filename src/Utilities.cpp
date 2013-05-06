//
//  Utilities.cpp
//  sparks
//
//  Created by Brian Allen on 3/26/13.
//
//

#include "Utilities.hpp"

#include <IL/il.h>

#include <fstream>
#include <sstream>
#include <iostream>



void APIENTRY debugOpenGLMessageCallback( GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    void* userParam )
{
    using namespace std;
    stringstream msg;
    msg << "GL Debug Message- Source: ";
    switch( source )
    {
    case GL_DEBUG_SOURCE_API:
        msg << "GL_DEBUG_SOURCE_API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        msg << "GL_DEBUG_SOURCE_WINDOW_SYSTEM"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        msg << "GL_DEBUG_SOURCE_SHADER_COMPILER"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        msg << "GL_DEBUG_SOURCE_THIRD_PARTY"; break;
    case GL_DEBUG_SOURCE_APPLICATION:
        msg << "GL_DEBUG_SOURCE_APPLICATION"; break;
    case GL_DEBUG_SOURCE_OTHER:
        msg << "GL_DEBUG_SOURCE_OTHER"; break;
    default:
        msg << "UNKNOWN";
    }
    msg << ", Type: ";
    switch( type )
    {
    case GL_DEBUG_TYPE_ERROR:
        msg << "GL_DEBUG_TYPE_ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        msg << "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        msg << "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY:
        msg << "GL_DEBUG_TYPE_PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        msg << "GL_DEBUG_TYPE_PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER:
        msg << "GL_DEBUG_TYPE_OTHER"; break;
    default:
        msg << "UNKNOWN";
    }
    msg << ", Severity: ";
    switch( severity )
    {
    case GL_DEBUG_SEVERITY_LOW:
        msg << "GL_DEBUG_SEVERITY_LOW"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        msg << "GL_DEBUG_SEVERITY_MEDIUM"; break;
    case GL_DEBUG_SEVERITY_HIGH:
        msg << "GL_DEBUG_SEVERITY_HIGH"; break;
    default:
        msg << "UNKNOWN";
    }
    LOG_ERROR(g_log) << msg.str() << ", Message: \"" << message << "\".\n";
    //assert(false);
}



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

/// Startup OpenGL and create the rendering context and window.
OpenGLWindow
::OpenGLWindow( const char* programName )
: m_isOK( false )
{
    LOG_DEBUG(g_log) << "glfwInit...";
    if( !glfwInit() )
    {
        LOG_FATAL(g_log)<< "Failed to initialize GLFW.\n";
    }
    LOG_DEBUG(g_log) << "done.\n";

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

    LOG_DEBUG(g_log) << "glfwOpenWindow...";
    glfwOpenWindow( 800, 600, 0, 0, 0, 0, 0, 0, GLFW_WINDOW ); //
    glfwEnable( GLFW_MOUSE_CURSOR );
    glfwSetWindowTitle( programName );
    checkOpenGLErrors();
    LOG_DEBUG(g_log) << " done.\n";
    LOG_DEBUG(g_log) << "glewInit... ";
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    // according to http://www.opengl.org/wiki/OpenGL_Loading_Library
    // glewInit may throw invalid enumerant error
    glGetError(); // eat glew's spurious error
    if( err != GLEW_OK )
    {
        LOG_DEBUG(g_log) << "glewInit() failed!\n";
    }
    if( !glewIsSupported("GL_VERSION_3_2") )
    {
        LOG_DEBUG(g_log) << "OpenGL Version 3.2 Required!\n";
    }
    checkOpenGLErrors();
    if( GL_MAX_3D_TEXTURE_SIZE < 256 )
    {
        LOG_DEBUG(g_log) << "Max 3d texture size: " << GL_MAX_3D_TEXTURE_SIZE << " is too small for this program.\n";
    }

    if( glewIsSupported( "GL_ARB_debug_output" ) )
    {
#ifdef _DEBUG
        glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
        checkOpenGLErrors();
#else
        glEnable( GL_DEBUG_OUTPUT );
        checkOpenGLErrors();
#endif
        LOG_DEBUG(g_log) << "Enabling OpenGL Debug messages.\n";
#ifndef __APPLE__
        checkOpenGLErrors();
        glDebugMessageCallback( debugOpenGLMessageCallback, 0 );
        checkOpenGLErrors();
        GLuint unusedIds = 0;
        glDebugMessageControl( GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DONT_CARE,
            0,
            &unusedIds,
            true);
        checkOpenGLErrors();
#endif
    }
    checkOpenGLErrors();

    ilInit();
    checkOpenGLErrors();
    GL_CHECK( glEnable( GL_DEPTH_TEST ) );
    LOG_DEBUG(g_log) << "done.\n";
    m_isOK = true;
}

OpenGLWindow
::~OpenGLWindow()
{
    ilShutDown();
    glfwTerminate();
}

bool loadTextureFromFile( const char* filepath )
{
    ILuint texId = 0;
    ilGenImages( 1, &texId );
    ilBindImage( texId );
    ILboolean isLoaded = ilLoadImage( filepath );
    if( !isLoaded )
    {
        LOG_DEBUG(g_log) << "Failed to load image \"" << filepath << "\".";
        return false;
    }
    ILboolean isconverted = ilConvertImage( IL_RGBA, IL_UNSIGNED_BYTE );
    if( !isconverted )
    {
        LOG_DEBUG(g_log) << "failed to convert image \"" << filepath << "\".";
        return false;
    }
    if( IL_NO_ERROR != ilGetError() )
    {
        LOG_DEBUG(g_log) << "Error loading image!\n";
        assert( false );
    }
    GLint components = ilGetInteger(IL_IMAGE_BPP);
    GLint width = ilGetInteger(IL_IMAGE_WIDTH);
    GLint height = ilGetInteger(IL_IMAGE_HEIGHT);
    GLint imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
    GLint imageType = ilGetInteger(IL_IMAGE_TYPE);
    LOG_DEBUG(g_log) << "Texture loaded: components=" << components 
        << ", width=" << width << ", height=" << height 
        << ", imageFormat=" << imageFormat
        << ( GL_RGBA == imageFormat ? " GL_RGBA" : " No Alpha")
        << " from path \"" << filepath << "\"";
    GL_CHECK( glTexImage2D(GL_TEXTURE_2D,
                 0, // level-of-detail number
                 components,
                 width,
                 height,
                 0, // "border", must be zero
                 imageFormat,
                 imageType,
                 ilGetData() ) );
    // Data has now been copied to OpenGL, delete temp copy
    ilDeleteImages( 1, &texId );
    return true;
}

std::ostream& operator<<( std::ostream& out, glm::vec3 v )
{
    out << v.x << ", " << v.y << ", " << v.z ;
    return out;
}
std::ostream& operator<<( std::ostream& out, glm::vec4 v )
{
    out << v.x << ", " << v.y << ", " << v.z << ", " << v.w;
    return out;
}
std::ostream& operator<<( std::ostream& out, glm::mat3 m )
{
    out << glm::to_string(m);
    return out;
}
std::ostream& operator<<( std::ostream& out, glm::mat4 m )
{
    out << glm::to_string(m);
    return out;
}

///
void checkOpenGLErrors( const char* aCodeStatement, 
                        const char* aFileName, 
                        int aLineNumber )
{
    GLenum errCode;
    const GLubyte *errString;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        LOG_ERROR(g_log) << "OpenGL Error[" << errCode << "] \"" << errString 
            << "\", at " << aFileName << ":" << aLineNumber << " -- " 
            << aCodeStatement << "\n";
        //assert( false );
    }
}

void checkOpenGLErrors( void )
{
    GLenum errCode;
    const GLubyte *errString;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        LOG_DEBUG(g_log) << "OpenGL Error[" << errCode << "] \"" << errString << "\"\n";
        //assert( false );
    }
}

std::string readFileToString( const char* filename )
{
    using namespace std;
    ifstream srcFile(filename, std::ios::in );
    if( srcFile )
    {
        ostringstream contents;
        contents << srcFile.rdbuf();
        srcFile.close();
        return contents.str();
    }
    LOG_DEBUG(g_log) << "---------------------------------------\nError.  Unable to read file \"" << filename << "\"\n---------------------------------------\n";
    throw(errno);
}

GLuint createShaderWithErrorHandling( GLuint shaderType, const std::string& shaderSource )
{
    //////////////////
    // Load the Shader with error handling
    GLuint shader;
    GL_CHECK( shader = glCreateShader( shaderType ) );
    {
        const char* shaderSourceCStr = shaderSource.c_str();
        GL_CHECK( glShaderSource( shader, 1, &(shaderSourceCStr), NULL ) );
        GL_CHECK( glCompileShader( shader ) );
        GLint shaderStatus;
        GL_CHECK( glGetShaderiv( shader, GL_COMPILE_STATUS, &shaderStatus ) );
        const int buffSize = 1024;
        char buff[buffSize+1];
        GL_CHECK( glGetShaderInfoLog( shader, buffSize, NULL, buff ) );
        if( strnlen(buff, buffSize) )
        {
            LOG_WARN(g_log) << "Shader Compilation message:\n------------\n"
            << buff << "\n------------\n";
        }
        if( shaderStatus != GL_TRUE )
        {
            LOG_ERROR(g_log) << "Failed to compile shader:\n-------------\n"
            << shaderSource << "\n-------------\n";
            assert( false );
        }
    }
    return shader;
}
