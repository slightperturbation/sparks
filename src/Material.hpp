//
//  Material.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_Material_hpp
#define sparks_Material_hpp

#include "Shader.hpp"
#include "Projection.hpp"
#include "TextureManager.hpp"

#include <set>

/// Material is one way to render the object.  Note that one material 
/// corresponds to one type of render-pass, so a color pass would use
/// a different Material object than a depth-write pass or a G-buffer pass.
/// 
/// Universal uniforms
/// mat4 u_projMat
/// mat4 u_modelViewMat
/// mat4 u_modelViewProjMat
class Material
{
public:

    //TODO ctor --- load material from file
    Material( TextureManagerPtr tm ) : m_textureManager( tm )
    { }
    Material( TextureManagerPtr tm, ShaderPtr aShader )
    : m_shader( aShader ), m_textureManager( tm )
    { }
    void setShader( ShaderPtr aShader )
    {
        m_shader = aShader;
    }
    GLuint getGLShaderIndex( void ) const 
    {
        return m_shader->getGLProgramIndex();
    }
    const std::string& name( void ) const { return m_shader->name(); }

    /// Set OpenGLs state to use the material
    void use( void ) const
    {
        if( !m_shader ) LOG_ERROR(g_log) << "Material used without a shader\n";
        m_shader->use();
        
        // setup texture uniforms
        for( auto texIter = m_textures.begin();
             texIter != m_textures.end(); 
             ++texIter )
        {
            const TextureName& textureName = *texIter;
            const ShaderUniformName& samplerInShaderName = textureName;
            GLuint texUnit = m_textureManager->getTextureUnitForTexture( textureName );
            // Shader looks for the sampler of the same name as the texture
            m_shader->setUniform( samplerInShaderName, texUnit );
        }
    }

    /// Sets shader uniforms for the given perspective.
    void usePerspective( ConstPerspectivePtr aPerspective )
    {
        if( !m_shader )
        {
            LOG_ERROR(g_log) << "Material::usePerspective without a shader\n";
            assert( false );
        }
        ShaderUniform<glm::mat4>* modelView = m_shader->getUniform<glm::mat4>("u_modelViewMat");
        modelView->set( aPerspective->modelViewMatrix() );
        ShaderUniform<glm::mat4>* modelViewProj = m_shader->getUniform<glm::mat4>("u_modelViewProjMat");
        modelViewProj->set( aPerspective->getModelViewProjMatrix() );
        ShaderUniform<glm::mat4>* proj = m_shader->getUniform<glm::mat4>("u_projMat");
        proj->set( aPerspective->projectionMatrix() );
    }

    /// Usage examples:  materialPtr->shaderUniforms()["u_projMat"] = glm::mat4();
    /// materialPtr->shaderUniforms()["u_color"] = glm::vec4( 1,1,1,1 );
    ShaderUniformHolderPtr shaderUniforms( void )
    {
        return m_shader;
    }
    ConstShaderUniformHolderPtr shaderUniforms( void ) const
    {
        return m_shader;
    }
    
    /// Use a texture with this material.  Must be called prior to rendering
    /// with a shader that uses the texture.
    void addTexture( const TextureName& textureName )
    { m_textures.insert( textureName ); }

    /// Set shader uniform properties
    template< typename T >
    void setShaderUniform( ShaderUniformName aName, T arg )
    {
        m_shader->setUniform<T>( aName, arg );
    }
private:
    std::set< const TextureName > m_textures;
    ShaderPtr m_shader;
    TextureManagerPtr m_textureManager;
};

#endif
