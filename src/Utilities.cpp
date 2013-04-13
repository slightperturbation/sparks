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

bool loadTextureFromFile( const char* filepath )
{
    ILuint texId = 0;
    ilGenImages( 1, &texId );
    ilBindImage( texId );
    ILboolean isLoaded = ilLoadImage( filepath );
    if( !isLoaded )
    {
        LOG_INFO(g_log) << "Failed to load image \"" << filepath << "\".";
        return false;
    }
    ILboolean isconverted = ilConvertImage( IL_RGBA, IL_UNSIGNED_BYTE );
    if( !isconverted )
    {
        LOG_INFO(g_log) << "failed to convert image \"" << filepath << "\".";
        return false;
    }
    if( IL_NO_ERROR != ilGetError() )
    {
        LOG_INFO(g_log) << "Error loading image!\n";
        assert( false );
    }
    GLint components = ilGetInteger(IL_IMAGE_BPP);
    GLint width = ilGetInteger(IL_IMAGE_WIDTH);
    GLint height = ilGetInteger(IL_IMAGE_HEIGHT);
    GLint imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
    GLint imageType = ilGetInteger(IL_IMAGE_TYPE);
    LOG_INFO(g_log) << "TexImage -- components=" << components << ", width=" << width << ", height=" << height << ", imageFormat=" << imageFormat
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
        LOG_INFO(g_log) << "OpenGL Error[" << errCode << "] \"" << errString 
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
        LOG_INFO(g_log) << "OpenGL Error[" << errCode << "] \"" << errString << "\"\n";
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
    LOG_INFO(g_log) << "---------------------------------------\nError.  Unable to read file \"" << filename << "\"\n---------------------------------------\n";
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
            LOG_INFO(g_log) << "Shader Compilation message:\n------------\n"
            << buff << "\n------------\n";
        }
        if( shaderStatus != GL_TRUE )
        {
            LOG_INFO(g_log) << "Failed to compile shader:\n-------------\n"
            << shaderSource << "\n-------------\n";
            assert( false );
        }
        else
        {
            LOG_INFO(g_log) << "Shader loaded successfully.\n";
        }
    }
    return shader;
}

GLuint loadShaderFromFile( const char* vertexShaderFilepath, const char* fragmentShaderFilepath )
{
    // Read file
    std::string vertexShaderString = readFileToString( vertexShaderFilepath );
    GLuint vertexShader = createShaderWithErrorHandling( GL_VERTEX_SHADER, vertexShaderString );
    std::string fragmentShaderString = readFileToString( fragmentShaderFilepath );
    GLuint fragmentShader = createShaderWithErrorHandling( GL_FRAGMENT_SHADER, fragmentShaderString );
    
    GLuint shaderProgram = -1;
    GL_CHECK( shaderProgram = glCreateProgram() );
    GL_CHECK( glAttachShader( shaderProgram, vertexShader ) );
    GL_CHECK( glAttachShader( shaderProgram, fragmentShader ) );
    GL_CHECK( glBindFragDataLocation( shaderProgram, 0, "outColor" ) );  // define the output for color buffer-0
    GL_CHECK( glLinkProgram( shaderProgram ) );
    
    LOG_INFO(g_log) << "Loaded vertex shader: " << vertexShaderFilepath << "\n"
    << "\tLoaded fragment shader: " << fragmentShaderFilepath << "\n"
    << "\tto create shaderID = " << shaderProgram << "\n";
    return shaderProgram;
}
//////////////////////////////////////////////////////////////////////////
