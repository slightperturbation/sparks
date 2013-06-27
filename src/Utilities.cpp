//
//  Utilities.cpp
//  sparks
//
//  Created by Brian Allen on 3/26/13.
//
//

#include "Utilities.hpp"
#include "FileAssetFinder.hpp"
#include "Mesh.hpp"
#include "Exceptions.hpp"

#include <IL/il.h>
#include <IL/ilu.h>

#include <assimp/Importer.hpp>
#include <assimp/PostProcess.h>
#include <assimp/Scene.h>

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
    // ignore some warnings
    switch( id )
    {
    case 131218:  
        // Source: GL_DEBUG_SOURCE_API, 
        // Type: GL_DEBUG_TYPE_PERFORMANCE, 
        // Severity: GL_DEBUG_SEVERITY_MEDIUM[131218] 
        // Program/shader state performance warning: 
        // Fragment Shader is going to be recompiled because the shader 
        // key based on GL state mismatches.
        return;
    }
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
    LOG_ERROR(g_log) << msg.str() << "[" << id 
                     << "] Message: \"" << message << "\".\n";
    //assert(false);
}

void 
spark
::writeFrameBufferToFile( const std::string& frameBaseFileName ) 
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
spark::OpenGLWindow
::OpenGLWindow( const char* programName, bool enableLegacyOpenGlLogging )
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
    //glfwOpenWindowHint( GLFW_FSAA_SAMPLES, 8 ); // 8x anti aliasing
#ifdef __APPLE__
    // Need to force the 3.2 for mac -- note
    // that this breaks the AntTweak menus
    glfwOpenWindowHint( GLFW_OPENGL_VERSION_MAJOR, 3 );
    glfwOpenWindowHint( GLFW_OPENGL_VERSION_MINOR, 2 );
    //glfwOpenWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );  // Core profile not compatible with GLEW(?)
#endif
    glfwOpenWindowHint( GLFW_WINDOW_NO_RESIZE, GL_FALSE );

    LOG_DEBUG(g_log) << "glfwOpenWindow...";

    if( glfwOpenWindow( 800, 600, 0, 0, 0, 0, 24, 0, GLFW_WINDOW ) != GL_TRUE )
    {
        LOG_FATAL(g_log) << "Unable to open GLFW window.";
        glfwTerminate();
        return;
    }
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
        // Debug messages not supported by OSX
        checkOpenGLErrors();

        // Must be disabled for gDebugger and similar tools
        if( enableLegacyOpenGlLogging )
        {
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
        }
#endif
    }
    checkOpenGLErrors();
    ilInit();
    iluInit();
    checkOpenGLErrors();
    LOG_DEBUG(g_log) << "OpenGL initialization complete.";
    m_isOK = true;
}

spark::OpenGLWindow
::~OpenGLWindow()
{
    ilShutDown();
    glfwTerminate();
}

bool
spark::OpenGLWindow
::isRunning( void )
{
    return glfwGetWindowParam( GLFW_OPENED );
}

bool
spark
::loadCheckerTexture( void )
{
    float pixels[] = {
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f,
    };
    GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_FLOAT, pixels ) );
    return true;
}

bool
spark
::loadTestTexture( void )
{
    ILuint texId = 0;
    ILuint devilError = 0;
    ilGenImages( 1, &texId );
    ilBindImage( texId );
    ilDefaultImage();
    devilError = ilGetError();
    if( devilError != IL_NO_ERROR )
    {
        LOG_ERROR(g_log) << "Devil Error: ilLoadImage: "
        << iluErrorString( devilError ) ;
        assert( false );
        return false;
    }
    GLint internalFormat = GL_RGB;
    GLint width = ilGetInteger(IL_IMAGE_WIDTH);
    GLint height = ilGetInteger(IL_IMAGE_HEIGHT);
    GLint imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
    GLint imageType = ilGetInteger(IL_IMAGE_TYPE);
    LOG_DEBUG(g_log) << "Texture loaded: internal format = "
                     << ( GL_RGBA == internalFormat ? " GL_RGBA" : " GL_RGB")
                     << ", width=" << width << ", height=" << height
                     << ", imageFormat=(" << imageFormat << "="
                     << ( GL_RGBA == imageFormat ? " GL_RGBA" : " GL_RGB)");
    GL_CHECK( glTexImage2D(GL_TEXTURE_2D,
                           0, // level-of-detail number
                           internalFormat,
                           width,
                           height,
                           0, // "border", must be zero
                           imageFormat,
                           imageType,
                           ilGetData() ) );
    
    devilError = ilGetError();
    if( devilError != IL_NO_ERROR )
    {
        LOG_ERROR(g_log) << "Devil Error: ilDeleteImages: "
        << iluErrorString( devilError ) ;
        assert( false );
        return false;
    }
    // Data has now been copied to OpenGL, delete temp copy
    ilDeleteImages( 1, &texId );
    devilError = ilGetError();
    if( devilError != IL_NO_ERROR )
    {
        LOG_ERROR(g_log) << "Devil Error: ilDeleteImages: "
        << iluErrorString( devilError ) ;
        assert( false );
        return false;
    }
    // Success
    return true;
}

bool 
spark
::loadTextureFromFile( const char* filepath )
{
    ILuint texId = 0;
    ILuint devilError = 0;

    ilGenImages( 1, &texId );
    ilBindImage( texId );
    ILboolean isLoaded = ilLoadImage( filepath );
    if( !isLoaded )
    {
        LOG_ERROR(g_log) << "Failed to load image \"" << filepath << "\".";
        return -1;
    }
    devilError = ilGetError();
    if( devilError != IL_NO_ERROR )
    {
        LOG_ERROR(g_log) << "Devil Error: ilLoadImage: "
                         << iluErrorString( devilError ) ;
        assert( false );
        return false;
    }
    
    GLint format = GL_RGB;
    ILboolean isConverted = false;
    int bitDepth = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL );
    if( bitDepth == 32 )
    {
        format = GL_RGBA;
    }
    isConverted = ilConvertImage( format, IL_UNSIGNED_BYTE );
    if( !isConverted )
    {
        LOG_ERROR(g_log) << "Failed to convert image \"" << filepath
        << "\" with " << bitDepth << " bit depth.";
        return false;
    }
    else
    {
        LOG_DEBUG(g_log) << "Converted image \"" << filepath
        << "\" with " << bitDepth << " bit depth.";
    }
    devilError = ilGetError();
    if( devilError != IL_NO_ERROR )
    {
        LOG_ERROR(g_log) << "Devil Error: ilConvertImage: "
        << iluErrorString( devilError ) ;
        assert( false );
        return false;
    }
    
    GLint width = ilGetInteger(IL_IMAGE_WIDTH);
    GLint height = ilGetInteger(IL_IMAGE_HEIGHT);
    GLint imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
    GLint imageType = ilGetInteger(IL_IMAGE_TYPE);
    LOG_DEBUG(g_log) << "Texture loaded: internal format = "
                     << ( GL_RGBA == imageFormat ? " GL_RGBA" : " GL_RGB")
                     << ", width=" << width << ", height=" << height
                     << ", imageFormat=(" << imageFormat << "="
                     << ( GL_RGBA == imageFormat ? " GL_RGBA" : " GL_RGB)")
                     << " from path \"" << filepath << "\"";
    GL_CHECK( glTexImage2D(GL_TEXTURE_2D,
                           0,              // level-of-detail number
                           format,         // *internal* format in device memory, GL_RGB or GL_RGBA
                           width, height,  // size of image
                           0,              // "border", must be zero
                           imageFormat,    // order of *incoming* data colors, GL_RGB or GL_RGA
                           imageType,      // type of incoming data
                           ilGetData()     // actual data
                            ) );
    
    devilError = ilGetError();
    if( devilError != IL_NO_ERROR )
    {
        LOG_ERROR(g_log) << "Devil Error: ilDeleteImages: "
        << iluErrorString( devilError ) ;
        assert( false );
        return false;
    }
    // Data has now been copied to OpenGL, delete temp copy
    ilDeleteImages( 1, &texId );
    devilError = ilGetError();
    if( devilError != IL_NO_ERROR )
    {
        LOG_ERROR(g_log) << "Devil Error: ilDeleteImages: "
        << iluErrorString( devilError ) ;
        assert( false );
        return false;
    }
    // Success
    return true;
}

std::ostream& spark::operator<<( std::ostream& out, const glm::vec2& v )
{
    out << v.x << ", " << v.y;
    return out;
}
std::ostream& spark::operator<<( std::ostream& out, const glm::vec3& v )
{
    out << v.x << ", " << v.y << ", " << v.z ;
    return out;
}
std::ostream& spark::operator<<( std::ostream& out, const glm::vec4& v )
{
    out << v.x << ", " << v.y << ", " << v.z << ", " << v.w;
    return out;
}
std::ostream& spark::operator<<( std::ostream& out, const glm::mat3& m )
{
    out << glm::to_string(m);
    return out;
}
std::ostream& spark::operator<<( std::ostream& out, const glm::mat4& m )
{
    out << glm::to_string(m);
    return out;
}

void 
spark
::checkOpenGLErrors( const char* aCodeStatement, 
                        const char* aFileName, 
                        int aLineNumber )
{
    GLenum errCode;
    const GLubyte *errString;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        LOG_ERROR(g_log) << "OpenGL Error[" << errCode << "] \"" 
            << ( errString ? (const char*)errString : "UNKNOWN" )
            << "\", at " << aFileName << ":" << aLineNumber << " -- " 
            << aCodeStatement << "\n";
        //assert( false );
    }
}

void 
spark
::checkOpenGLErrors( void )
{
    GLenum errCode;
    const GLubyte *errString;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        LOG_DEBUG(g_log) << "OpenGL Error[" << errCode << "] \"" << errString << "\"\n";
        //assert( false );
    }
}

std::string 
spark
::readFileToString( const char* filename )
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
    LOG_ERROR(g_log) << "Unable to read file \"" << filename << "\".";
    throw(errno);
}


GLuint 
spark
::createShaderWithErrorHandling( GLuint shaderType, const std::string& shaderSource )
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
            throw( ShaderCompilationException( buff, shaderSource ) );
        }
    }
    return shader;
}

GLuint 
spark::getErrorShader( void )
{
    static GLuint shaderProgram = 0;
    if( shaderProgram == 0 )
    {
        std::string vertexSource =
            "#version 150\n"
            "in vec3 v_position;\n"   
            "uniform mat4 u_projViewModelMat;\n"
            "void main() {\n"
            " gl_Position = u_projViewModelMat * vec4( v_position, 1.0 );\n"
            "}\n";
        std::string fragmentSource = 
            "#version 150\n"
            "out vec4 outColor;\n"
            "void main() {\n"
            " outColor = vec4( 1.0, 1.0, 0.2, 1.0 );\n"
            "}\n";

        shaderProgram = glCreateProgram();
        GLuint v = createShaderWithErrorHandling( GL_VERTEX_SHADER, vertexSource );
        GLuint f = createShaderWithErrorHandling( GL_FRAGMENT_SHADER, fragmentSource );
        glAttachShader( shaderProgram, v );
        glAttachShader( shaderProgram, f );
        glLinkProgram( shaderProgram );
    }
    return shaderProgram;
}

void 
spark
::createMeshesFromFile( const char* filePath, 
                        FileAssetFinderPtr finder,
                        std::vector< MeshPtr >& outMeshes )
{
    std::string foundPath;
    if( !finder->findFile( filePath, foundPath ) )
    {
        LOG_ERROR(g_log) << "Failed to find mesh file \"" << filePath << "\".";
        assert(false);
        return;
    }
    Assimp::Importer importer;
    //
    
    int processing;
    if( true )
    {
        processing = aiProcessPreset_TargetRealtime_MaxQuality;
    }
    else
    {
        processing =
              aiProcess_Triangulate
            | aiProcess_CalcTangentSpace
            | aiProcess_GenSmoothNormals
            | aiProcess_GenUVCoords
            | aiProcess_PreTransformVertices // Eliminate the hierarchy; also removes animations
            | aiProcess_TransformUVCoords // Bake in UV coord transforms
            ;
    }
    const aiScene* scene = importer.ReadFile( foundPath, processing );
    if( !scene )
    {
        LOG_ERROR(g_log) << "Unable to read file \"" << filePath 
            << "\" using found path \"" << foundPath << "\".";
        assert(false);
        return;
    }
    const aiNode* root = scene->mRootNode;
    LOG_INFO(g_log) << "Processing scene root node \"" << root->mName.C_Str() << "\".";
    if( !root )
    {
        LOG_ERROR(g_log) << "No scene root found in file \"" << filePath << "\".";
        return;
    }
    if( root->mNumMeshes < 1 )
    {
        LOG_WARN(g_log) << "No mesh found when loading scene file \"" 
            << filePath << "\" using found path \"" << foundPath << "\".";
    }
    for( size_t meshIndex = 0; meshIndex < root->mNumMeshes; ++meshIndex )
    {
        const aiMesh* aimesh = scene->mMeshes[ root->mMeshes[meshIndex] ];
        if( !aimesh ) continue;
        LOG_INFO(g_log) << "Processing mesh node \"" << aimesh->mName.C_Str() << "\".";        
        //const aiMaterial* aimaterial = scene->mMaterials[ aimesh->mMaterialIndex ];
        outMeshes.push_back( Mesh::createMeshFromAiMesh( aimesh, 1.0 ) );
    }
    LOG_INFO(g_log) << "Loaded " << outMeshes.size() << " new meshes.";
}
