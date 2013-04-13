//
//  Shader.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_Shader_hpp
#define sparks_Shader_hpp

#include "config.hpp"  // Defines DATA_PATH
#include "SoftTestDeclarations.hpp"
#include "Utilities.hpp"
#include "ShaderUniform.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <typeinfo>
#include <map>
#include <memory>

/// Responsible for loading a shader program from
/// files and holding the uniforms settings
/// Universal uniforms
/// mat4 u_projMat
/// mat4 u_modelViewMat
/// vec4 u_lightPos[N_LIGHTS]
/// vec3 u_lightColor[N_LIGHTS]
class Shader
{
public:
    Shader()
    :
    m_shaderProgramIndex(-1)
    {}
    
    ~Shader()
    {
        //TODO -- is 0 a valid shader program handle?
        if( m_shaderProgramIndex > -1 )
        {
            GL_CHECK( glDeleteProgram( m_shaderProgramIndex ) );
        }
        m_shaderProgramIndex = -1;
    }
    
    void use( void )
    {
        GL_CHECK( glUseProgram( m_shaderProgramIndex ) );
        applyShaderUniforms();
    }
    
    void loadFromFiles( const char* vertexShaderFilename,
                       const char* fragmentShaderFilename )
    {
        m_vertexFilePath = vertexShaderFilename;
        m_fragmentFilePath = fragmentShaderFilename;
        reloadFromFiles();
    }
    
    void reloadFromFiles( void )
    {
        if( !m_vertexFilePath.empty() && !m_fragmentFilePath.empty() )
        {
            m_shaderProgramIndex = loadShaderFromFile( m_vertexFilePath.c_str(),
                m_fragmentFilePath.c_str() );
            lookupUniformLocations();
        }
        else
        {
            LOG_WARN(g_log) << "Reloading shaders failed because filenames are blank.";
        }
    }
    
    template< typename T >
    ShaderUniform<T>* operator[]( const std::string& name )
    { return getUniform<T>(name); }
	
    template< typename T >
    const ShaderUniform<T>* operator[]( const std::string& name ) const
    { return getUniform<T>(name); }
	
	template< typename T >
    ShaderUniform<T>* getUniform(const std::string& name)
	{ return m_uniforms[name]->as<T>(); }
    
    template< typename T >
	const ShaderUniform<T>* getUniform(const std::string& name) const
	{
        return m_uniforms.at(name)->as<T>();
    }
    
	template< typename T >
	void setUniform(const std::string& name, const T& val )
	{ m_uniforms[name]->as<T>()->set(val); }
    
    /// Note that compilation will fail if ShaderUniform<T> is uninstanced.
    template <typename T>
    void createUniform( const std::string& name )
    {
        m_uniforms[name] = new ShaderUniform<T>();
    }
    /// Note that compilation will fail if ShaderUniform<T> is uninstanced.
    template <typename T>
    void createUniform( const std::string& name, const T& val )
    {
        m_uniforms[name] = new ShaderUniform<T>( val );
    }
    
private:
    /// Set uniforms on this shader (when dirty)
    /// OpenGL state change:  current program is set.
    void applyShaderUniforms( void )
    {
        GL_CHECK( glUseProgram( m_shaderProgramIndex ) );
        for( auto sumap = m_uniforms.begin(); sumap != m_uniforms.end(); ++sumap )
        {
            (*sumap).second->apply();
        }
    }
    
    /// Must be called after a shader is compiled to set each uniform's
    /// location in that shader.
    void lookupUniformLocations( void )
    {
        GLint loc;
        for( auto sumap = m_uniforms.begin(); sumap != m_uniforms.end(); ++sumap )
        {
            GL_CHECK( loc = glGetUniformLocation( m_shaderProgramIndex, (*sumap).first.c_str() ) );
            if( loc == GL_INVALID_VALUE || loc == GL_INVALID_OPERATION )
            {
                LOG_INFO(g_log) << "Failed to find shader uniform of name \""
                << sumap->first << "\" in shader #"
                << m_shaderProgramIndex << "\n";
            }
            else
            {
                (*sumap).second->m_locationInShader = loc;
            }
        }
    }
    
    std::string m_vertexFilePath;
    std::string m_fragmentFilePath;
    GLint m_shaderProgramIndex;
    std::map< std::string, ShaderUniformInterface* > m_uniforms;
};
typedef std::shared_ptr< Shader > ShaderPtr;

#endif
