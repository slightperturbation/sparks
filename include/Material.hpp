//
//  Material.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_Material_hpp
#define sparks_Material_hpp

#include "ShaderInstance.hpp"
#include "TextureManager.hpp"

#include <set>

namespace spark
{
    /// Material is a description of how to render one or more objects.  
    /// Note that one material corresponds to one type of render-pass, 
    /// so a color pass would use a different Material object than 
    /// a depth-write pass or a G-buffer pass.
    class Material
    {
    public:
        Material( TextureManagerPtr tm );
        Material( TextureManagerPtr tm, ShaderInstancePtr aShader );
        ~Material();

        /// Specifies the shader this material is based on.  All materials
        /// must have a shader.
        void setShader( ShaderInstancePtr aShader );

        /// Returns the OpenGL Shader ID used by this material.
        GLuint getGLShaderIndex( void ) const;

        /// Returns the name of the **shader** used by this material.
        const std::string& name( void ) const;

        /// Set the debugging name for this material.  Use with restraint, 
        /// as its often nicer to leave it as the shader name.
        void name( const std::string& aName );

        /// Set OpenGLs state to use the material
        void use( void ) const;

        /// Usage examples:
        ///   materialPtr->shaderUniforms()["u_projMat"] = glm::mat4();
        ///   materialPtr->shaderUniforms()["u_color"] = glm::vec4( 1,1,1,1 );
        ///   materialPtr->setShaderUniform<float>( "u_rate", 1.0f );
        ShaderUniformHolderPtr shaderUniforms( void )
        { return m_shader; }
        ConstShaderUniformHolderPtr shaderUniforms( void ) const
        { return m_shader; }
        /// Use a texture with this material.  Must be called prior to rendering
        /// with a shader that uses the texture.
        void addTexture( const ShaderUniformName& samplerName, const TextureName& textureName );

        /// Set shader uniform properties
        template< typename T >
        void setShaderUniform( ShaderUniformName aName, T arg )
        {
            if( g_log->isTrace() )
            {
                LOG_TRACE(g_log) << "Setting Shader Uniform \""
                                 << aName << "\" for Material \""
                                 << name() << "\".";
            }
            m_shader->setUniform<T>( aName, arg );
        }
        /// Send to logger all of the shader uniforms actually applied.
        void dumpShaderUniforms( void ) const;
    private:
        std::string m_name;

        /// Holds each texture used by this material.  Maps from the name of
        /// the texture in the TextureManager (TextureName, e.g., "skin_normals")
        /// to the uniform name in m_shader (ShaderUniformName, e.g., "s_normalMap")
        std::set< std::pair< const TextureName, const ShaderUniformName > > m_textures;

        /// The instance of the shader used by this material.
        ShaderInstancePtr m_shader;

        /// The texture manager holding this material's textures.
        TextureManagerPtr m_textureManager;
    };
}
#endif
