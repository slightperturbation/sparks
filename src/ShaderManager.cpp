
#include "ShaderManager.hpp"
#include "Utilities.hpp"



ShaderManager
::~ShaderManager() 
{
    for( auto iter = m_registry.begin(); iter != m_registry.end(); ++iter )
    {
        GLuint id = (*iter).second;
        if( id != -1 )
        {
            GL_CHECK( glDeleteProgram( id ) );
        }
        (*iter).second = -1;
    }
}

void 
ShaderManager
::loadShaderFromFiles( const ShaderName& aHandle,
                       const char* aVertexFileName,
                       const char* aFragmentFileName )
{
    std::string vertexFilePath, fragmentFilePath;
    std::string vertexName( aVertexFileName );
    if( !m_finder->findFile( vertexName, vertexFilePath ) )
    {
        LOG_ERROR(g_log) << "FAILED to find vertex shader file \""
            << aVertexFileName << "\".";
        assert(false);
        return;
    }
    if( !m_finder->findFile( aFragmentFileName, fragmentFilePath ) )
    {
        LOG_ERROR(g_log) << "FAILED to find fragment shader file \""
            << aFragmentFileName << "\".";
        assert(false);
        return;
    }
    GLuint shaderProgram = -1;
    GL_CHECK( shaderProgram = glCreateProgram() );
    m_registry[aHandle] = shaderProgram;
    ShaderFilePaths& filePaths = m_files[aHandle];
    filePaths.vertexFilePath = vertexFilePath;
    filePaths.fragmentFilePath = fragmentFilePath; 
    reloadShader( aHandle );
}

void
ShaderManager
::reloadShader( const ShaderName& aHandle )
{
    GLuint shaderProgram = m_registry[aHandle];
    LOG_INFO(g_log) << "Reloading vertex shader from path: " << m_files[aHandle].vertexFilePath;
    std::string vertexShaderString = readFileToString( m_files[aHandle].vertexFilePath.c_str() );
    GLuint vertexShader = createShaderWithErrorHandling( GL_VERTEX_SHADER, vertexShaderString );
    LOG_INFO(g_log) << "Reloading fragment shader from path: " << m_files[aHandle].fragmentFilePath;
    std::string fragmentShaderString = readFileToString( m_files[aHandle].fragmentFilePath.c_str() );
    GLuint fragmentShader = createShaderWithErrorHandling( GL_FRAGMENT_SHADER, fragmentShaderString );

    GL_CHECK( glAttachShader( shaderProgram, vertexShader ) );
    GL_CHECK( glAttachShader( shaderProgram, fragmentShader ) );
    GL_CHECK( glBindFragDataLocation( shaderProgram, 0, "outColor" ) );  // define the output for color buffer-0
    GL_CHECK( glLinkProgram( shaderProgram ) );

    LOG_DEBUG(g_log) << "Loaded vertex shader: " << m_files[aHandle].vertexFilePath 
        << "\tLoaded fragment shader: " << m_files[aHandle].fragmentFilePath 
        << "\tto create shaderID = " << shaderProgram ;
}

void
ShaderManager
::reloadAllShaders( void )
{
    for( auto iter = m_registry.begin(); iter != m_registry.end(); ++iter )
    {
        reloadShader( iter->first );
    }
}

void 
ShaderManager
::releaseAll( void )
{
    for( auto iter = m_registry.begin(); iter != m_registry.end(); ++iter )
    {
        GLuint shaderId = iter->second;
        if( shaderId != -1 )
        {
            glDeleteShader( shaderId );
        }
    }
}


