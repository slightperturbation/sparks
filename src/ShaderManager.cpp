
#include "ShaderManager.hpp"
#include "Utilities.hpp"
#include "ShaderInstance.hpp"
#include "Exceptions.hpp"

spark::ShaderManager
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

spark::ShaderInstancePtr 
spark::ShaderManager
::createShaderInstance( const ShaderName& name )
{
    ShaderManagerPtr manager = shared_from_this();
    ShaderInstancePtr out( new ShaderInstance( name, manager ) );
    spark::weak_ptr< ShaderInstance > weakOut( out );
    m_shaderInstances.push_back( weakOut );
    return out;
}

unsigned int 
spark::ShaderManager
::getProgramIndexForShaderName( const ShaderName& name )
{
    auto iter = m_registry.find( name );
    if( iter == m_registry.end() )
    {
        LOG_ERROR(g_log) << "Failed to find shader by name \"" << name
            << "\".  Not loaded?";
        return -1;
    }
    return iter->second;
}

void 
spark::ShaderManager
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
    ShaderFilePaths& filePaths = m_files[aHandle];
    filePaths.vertexFilePath = vertexFilePath;
    filePaths.fragmentFilePath = fragmentFilePath; 
    reloadShader( aHandle );
}

void
spark::ShaderManager
::reloadShader( const ShaderName& aHandle )
{
    GLuint shaderProgram = -1;
    auto shaderItr = m_registry.find( aHandle );
    if( shaderItr != m_registry.end() ) 
    {
        // already exists
        if(    glIsProgram( shaderProgram ) 
            && shaderProgram != getErrorShader() )
        {
            LOG_DEBUG(g_log) << "Deleting existing shader for handle \""
                             << aHandle << "\" during reloading.";
            glDeleteShader( shaderProgram );
        }
    }
    GL_CHECK( shaderProgram = glCreateProgram() );
    m_registry[aHandle] = shaderProgram;

    LOG_DEBUG(g_log) << "Loading vertex shader from path: "
        << m_files[aHandle].vertexFilePath;
    std::string vertexShaderString
        = readFileToString( m_files[aHandle].vertexFilePath.c_str() );
    LOG_DEBUG(g_log) << "Loading fragment shader from path: "
        << m_files[aHandle].fragmentFilePath;
    std::string fragmentShaderString
        = readFileToString( m_files[aHandle].fragmentFilePath.c_str() );

    //TODO-- Have files loaded OK?
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    try
    {
        vertexShader = createShaderWithErrorHandling( GL_VERTEX_SHADER,
            vertexShaderString );
        fragmentShader = createShaderWithErrorHandling( GL_FRAGMENT_SHADER,
            fragmentShaderString );

        GL_CHECK( glAttachShader( shaderProgram, vertexShader ) );
        GL_CHECK( glAttachShader( shaderProgram, fragmentShader ) );

        // Define the fixed shader variable "outColor" receive the color channel
        GL_CHECK( glBindFragDataLocation( shaderProgram, 0, "outColor" ) );

        GLint linkStatus = GL_FALSE;
        GL_CHECK( glLinkProgram( shaderProgram ) );
        glGetProgramiv( shaderProgram, GL_LINK_STATUS, &linkStatus );
        if( linkStatus == GL_FALSE )
        {
            const size_t logSize = 2048;
            GLchar errorLog[ logSize ] = {0};
            glGetProgramInfoLog(shaderProgram, logSize, NULL, errorLog);
            LOG_ERROR(g_log) << "Failed to link shader \"" << aHandle
                             << "\" with vertex shader: \""
                             << m_files[aHandle].vertexFilePath
                             << "\", and fragment shader: \""
                             << m_files[aHandle].fragmentFilePath
                             << "\".";
            LOG_ERROR(g_log) << "Shader Link error: " << errorLog;
            throw ShaderCompilationException( errorLog, 
                vertexShaderString + "\n------\n" + fragmentShaderString );
        }

        LOG_INFO(g_log) << "Loaded shader \"" << aHandle
            << "\" with vertex shader: \""
            << m_files[aHandle].vertexFilePath
            << "\", and fragment shader: \""
            << m_files[aHandle].fragmentFilePath
            << "\", to create shaderID = " << shaderProgram ;
    }
    catch( ShaderCompilationException& )
    {
        if( vertexShader ) glDeleteShader( vertexShader );
        if( fragmentShader ) glDeleteShader( fragmentShader );
        LOG_WARN(g_log) << "Using Error Shader in place of shader \"" 
                        << aHandle << "\".";
        glDeleteProgram( shaderProgram );
        /// Exception loading, so show the error shader in place
        m_registry[aHandle] = getErrorShader();
    }
    // Error or not, mark all instances of this shader as dirty
    for( auto shaderIter = m_shaderInstances.begin();
         shaderIter != m_shaderInstances.end();
         ++shaderIter )
    {
        if( ShaderInstancePtr shader = shaderIter->lock() )
        {
            if( shader->name() == aHandle )
            {
                shader->markDirty();
            }
        }
    }
}

void
spark::ShaderManager
::reloadAllShaders( void )
{
    for( auto iter = m_registry.begin(); iter != m_registry.end(); ++iter )
    {
        reloadShader( iter->first );
    }
    refreshUniformLocations();
}

void
spark::ShaderManager
::refreshUniformLocations( void )
{
    for( auto shaderIter = m_shaderInstances.begin();
         shaderIter != m_shaderInstances.end();
         ++shaderIter )
    {
        if( ShaderInstancePtr shader = shaderIter->lock() )
        {
            shader->markDirty();
        }
    }
}

void
spark::ShaderManager
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
