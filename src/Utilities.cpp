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
    LOG_DEBUG(g_log) << "GL Debug Message\n"
        << "Source: ";
    switch( source )
    {
    case GL_DEBUG_SOURCE_API:
        LOG_DEBUG(g_log) << "GL_DEBUG_SOURCE_API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        LOG_DEBUG(g_log) << "GL_DEBUG_SOURCE_WINDOW_SYSTEM"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        LOG_DEBUG(g_log) << "GL_DEBUG_SOURCE_SHADER_COMPILER"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        LOG_DEBUG(g_log) << "GL_DEBUG_SOURCE_THIRD_PARTY"; break;
    case GL_DEBUG_SOURCE_APPLICATION:
        LOG_DEBUG(g_log) << "GL_DEBUG_SOURCE_APPLICATION"; break;
    case GL_DEBUG_SOURCE_OTHER:
        LOG_DEBUG(g_log) << "GL_DEBUG_SOURCE_OTHER"; break;
    default:
        LOG_DEBUG(g_log) << "UNKNOWN";
    }
    LOG_DEBUG(g_log) << "\nType: ";
    switch( type )
    {
    case GL_DEBUG_TYPE_ERROR:
        LOG_DEBUG(g_log) << "GL_DEBUG_TYPE_ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        LOG_DEBUG(g_log) << "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        LOG_DEBUG(g_log) << "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY:
        LOG_DEBUG(g_log) << "GL_DEBUG_TYPE_PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        LOG_DEBUG(g_log) << "GL_DEBUG_TYPE_PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER:
        LOG_DEBUG(g_log) << "GL_DEBUG_TYPE_OTHER"; break;
    default:
        LOG_DEBUG(g_log) << "UNKNOWN";
    }
    LOG_DEBUG(g_log) << "\nSeverity: ";
    switch( severity )
    {
    case GL_DEBUG_SEVERITY_LOW:
        LOG_DEBUG(g_log) << "GL_DEBUG_SEVERITY_LOW"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        LOG_DEBUG(g_log) << "GL_DEBUG_SEVERITY_MEDIUM"; break;
    case GL_DEBUG_SEVERITY_HIGH:
        LOG_DEBUG(g_log) << "GL_DEBUG_SEVERITY_HIGH"; break;
    default:
        LOG_DEBUG(g_log) << "UNKNOWN";
    }
    LOG_DEBUG(g_log) << "\nMessage: \"" << message << "\".\n";
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

    if( GL_ARB_debug_output ) //glewIsSupported( "GL_ARB_debug_output" ) )
    {
#ifdef _DEBUG
        glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
#else
        glEnable( GL_DEBUG_OUTPUT );
#endif
        LOG_DEBUG(g_log) << "Enabling OpenGL Debug messages.\n";
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
    checkOpenGLErrors();
    GL_CHECK( glEnable( GL_DEPTH_TEST ) );
    LOG_DEBUG(g_log) << "done.\n";
    m_isOK = true;
}

OpenGLWindow
::~OpenGLWindow()
{
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
    LOG_DEBUG(g_log) << "TexImage -- components=" << components << ", width=" << width << ", height=" << height << ", imageFormat=" << imageFormat
    << ( GL_RGBA == imageFormat ? " GL_RGBA" : " No Alpha") << "\n";
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

///
void checkOpenGLErrors( const char* aCodeStatement, 
                        const char* aFileName, 
                        int aLineNumber )
{
    GLenum errCode;
    const GLubyte *errString;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        LOG_DEBUG(g_log) << "OpenGL Error[" << errCode << "] \"" << errString 
            << "\", at " << aFileName << ":" << aLineNumber << " -- " 
            << aCodeStatement << "\n";
        assert( false );
    }
}

void checkOpenGLErrors( void )
{
    GLenum errCode;
    const GLubyte *errString;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        LOG_DEBUG(g_log) << "OpenGL Error[" << errCode << "] \"" << errString << "\"\n";
        assert( false );
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
