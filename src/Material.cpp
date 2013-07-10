//
//  spark::Material.hpp
//  sparks
//
//  Created by Brian Allen on 4/26/13.
//
//
#include "Material.hpp"
#include "Projection.hpp"
#include "Renderable.hpp"

spark::Material
::Material( TextureManagerPtr tm ) : m_textureManager( tm )
{ }

spark::Material
::Material( TextureManagerPtr tm, ShaderInstancePtr aShader )
    : m_textureManager( tm )
{ setShader(aShader); }

void 
spark::Material
::setShader( ShaderInstancePtr aShader )
{ m_shader = aShader; m_name = m_shader->name(); }

GLuint 
spark::Material
::getGLShaderIndex( void ) const 
{ return m_shader->getGLProgramIndex(); }

const std::string& 
spark::Material
::name( void ) const 
{ return m_name; }

void
spark::Material
::name( const std::string& arg )
{ m_name = arg; }

void 
spark::Material
::use( void ) const
{
    if( !m_shader ) 
    {
        LOG_ERROR(g_log) << "spark::Material used without a shader\n";
        assert( false );
        return;
    }
    LOG_TRACE(g_log) << "Using Material \"" << name() << "\".";
    // setup texture uniforms
    for( auto texIter = m_textures.begin();
        texIter != m_textures.end(); 
        ++texIter )
    {
        const TextureName& textureName = texIter->first;
        const ShaderUniformName& samplerNameInShader = texIter->second;
        GLint texUnit = m_textureManager->getTextureUnitForHandle( textureName );
        if( texUnit == -1 )
        {
            LOG_ERROR(g_log) << "Unable to bind texture \"" << textureName 
                << "\" in spark::Material \"" << name() << "\".";
        }
        m_shader->setUniform( samplerNameInShader, texUnit );
        LOG_TRACE(g_log) << "spark::Material setting texture sampler uniform \""
                         << samplerNameInShader << "\" = " << texUnit
                         << " bound to texture \"" << textureName << "\".";
    }
    m_shader->use();
}

void spark::Material::addTexture( const ShaderUniformName& samplerName, const TextureName& textureName )
{ 
    if( !m_textureManager->isTextureReady( textureName ) )
    {
        LOG_WARN(g_log) << "Adding texture \"" 
                        << textureName
                        << "\" to spark::Material, but texture has not been loaded.";
    }
    m_textures.insert( make_pair( textureName, samplerName ) );
}

void
spark::Material
::dumpShaderUniforms( void ) const
{
    GLint programHandle = m_shader->getGLProgramIndex();
    GLint nUniforms, maxLen;
    glGetProgramiv( programHandle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
    glGetProgramiv( programHandle, GL_ACTIVE_UNIFORMS, &nUniforms);
    GLchar* name = new GLchar[ maxLen ];
    GLint size, location;
    GLsizei written;
    GLenum type;
    LOG_TRACE(g_log) << "Shader \"" << m_shader->name() << "\" Uniforms:";
    for( size_t i = 0; i < nUniforms; ++i ) {
        glGetActiveUniform( programHandle, i, maxLen, &written,
                           &size, &type, name );
        location = glGetUniformLocation(programHandle, name);
        LOG_TRACE(g_log) << "\t\"" << name << "\" at location: " << location;
    }
    delete[] name;
}
