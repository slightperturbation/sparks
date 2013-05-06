//
//  Material.hpp
//  sparks
//
//  Created by Brian Allen on 4/26/13.
//
//
#include "Material.hpp"
#include "Projection.hpp"
#include "Renderable.hpp"

Material
::Material( TextureManagerPtr tm ) : m_textureManager( tm )
{ }

Material
::Material( TextureManagerPtr tm, ShaderInstancePtr aShader )
    : m_shader( aShader ), m_textureManager( tm )
{ }

void 
Material
::setShader( ShaderInstancePtr aShader )
{ m_shader = aShader; }

GLuint 
Material
::getGLShaderIndex( void ) const 
{ return m_shader->getGLProgramIndex(); }

const std::string& 
Material
::name( void ) const 
{ return m_shader->name(); }

void 
Material
::use( void ) const
{
    if( !m_shader ) 
    {
        LOG_ERROR(g_log) << "Material used without a shader\n";
        assert( false );
        return;
    }
    // setup texture uniforms
    for( auto texIter = m_textures.begin();
        texIter != m_textures.end(); 
        ++texIter )
    {
        const TextureName& textureName = texIter->first;
        const ShaderUniformName& samplerNameInShader = texIter->second;
        GLint texUnit = m_textureManager->getTextureUnitForTexture( textureName );
        m_shader->setUniform( samplerNameInShader, texUnit );
    }
    m_shader->use();
}

void 
Material
::addTexture( const TextureName& textureName, 
              const ShaderUniformName& samplerName )
{ 
    if( !m_textureManager->isTextureNameReady( textureName ) )
    {
        LOG_WARN(g_log) << "Adding texture \"" 
            << textureName << "\" to material, but that texture has not been loaded.";
    }
    m_textures.insert( make_pair( textureName, samplerName ) );
    GLint unit = m_textureManager->getTextureUnitForTexture( textureName );
    m_shader->setUniform( samplerName, unit );
}

void
Material
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
    delete name;
}
