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

class TextureUnit;
//
//template< typename T >
//class MaterialProperty;
//
//class MaterialPropertyInterface
//{
//public:
//    template<typename T> MaterialProperty<T>* as( void )
//    {
//        return dynamic_cast< MaterialProperty<T>* >( this );
//    }
//};
//template< typename T >
//class MaterialProperty
//{
//public:
//    T m_val;
//};


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
    Material() {}
    Material( ShaderPtr aShader ) : m_shader( aShader )
    {
        
    }
    
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
        for( auto texIter = m_textureUnits.begin(); 
             texIter != m_textureUnits.end(); 
             ++texIter )
        {
            const TextureUnit& texUnit = *texIter;

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

    /// Set shader uniform properties
    template< typename T >
    void setShaderUniform( ShaderUniformName aName, T arg )
    {
        m_shader->setUniform<T>( aName, arg );
    }
private:
    std::vector< TextureUnit > m_textureUnits;
    ShaderPtr m_shader;
    
    //std::map< std::string, MaterialPropertyInterface* > m_materialProperties;

    //glm::vec4 m_ambientColor;
    //glm::vec4 m_specularColor;
    //float m_specularity;

    //bool m_receivesShadows;
    //bool m_castsShadows;
    //bool m_isTranslucent;
};

class TextureUnit
{
public:
    // texture
    // minfilter, magfilter, mipmap, wrapping
    
    TextureManagerPtr m_textureManager;
};

#endif
