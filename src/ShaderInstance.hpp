//
//  ShaderInstance.hpp
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
#include "ShaderManager.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <typeinfo>
#include <map>
#include <memory>

/// ShaderUniformHolder keeps track of the uniforms used by a shader.
class ShaderUniformHolder
{
public:
    virtual ~ShaderUniformHolder();
    template< typename T >
    ShaderUniform<T>* operator[]( const ShaderUniformName& name )
    {
        return getUniform<T>(name);
    }

    template< typename T >
    const ShaderUniform<T>* operator[]( const ShaderUniformName& name ) const
    { return getUniform<T>(name); }

    template< typename T >
    ShaderUniform<T>* getUniform( const ShaderUniformName& name)
    {
        auto iter = m_uniforms.find( name );
        if( iter == m_uniforms.end() )
        {
            createUniform<T>( name );
        }
        return m_uniforms[name]->as<T>(); 
    }

    /// Throws exception if name hasn't been created yet.
    template< typename T >
    const ShaderUniform<T>* getUniform( const ShaderUniformName& name) const
    {
        return m_uniforms.at(name)->as<T>();
    }

    template< typename T >
    void setUniform( const ShaderUniformName& name, const T& val )
    { 
        LOG_TRACE(g_log) << "Setting uniform " << name ;
        if( m_uniforms.find( name ) == m_uniforms.end() ) 
        {
            createUniform<T>( name );
        }
        ShaderUniformInterface* uniform = m_uniforms[name];
        ShaderUniform<T>* uniformT = uniform->as<T>();
        if( !uniformT )
        {
            LOG_ERROR(g_log) << "ShaderUniform type conflict on uniform \""
                << name << "\" trying to request type " << typeid(T).name();
            assert( false );
            return;
        }
        uniformT->set( val );
    }
    /// Note that compilation will fail if ShaderUniform<T> is uninstantiated.
    template <typename T>
    void createUniform( const ShaderUniformName& name )
    {
        LOG_TRACE(g_log) << "Creating uniform " << name << " with default value.";
        m_uniforms[name] = new ShaderUniform<T>();
    }

protected:
    /// Set uniforms on this shader (when dirty)
    /// OpenGL state change:  current program is set.
    void applyShaderUniforms( GLint a_shaderProgramIndex ) const
    {
        lookupUniformLocations( a_shaderProgramIndex );
        //GL_CHECK( glUseProgram( a_shaderProgramIndex ) );
        for( auto sumap = m_uniforms.begin(); sumap != m_uniforms.end(); ++sumap )
        {
            LOG_TRACE(g_log) << "Applying ShaderUniform " << (*sumap).first;
            (*sumap).second->apply();
        }
    }

    /// Must be called after a shader is compiled to set each uniform's
    /// location in that shader.
    void lookupUniformLocations( GLint a_shaderProgramIndex ) const
    {
        GLint loc;
        for( auto sumap = m_uniforms.begin(); sumap != m_uniforms.end(); ++sumap )
        {
            GL_CHECK( loc = glGetUniformLocation( a_shaderProgramIndex, 
                                                  (*sumap).first.c_str() ) );
            if( loc == GL_INVALID_VALUE || loc == GL_INVALID_OPERATION )
            {
                LOG_DEBUG(g_log) << "Failed to find shader uniform of name \""
                                << sumap->first << "\" in shader #"
                                << a_shaderProgramIndex << "\n";
            }
            else
            {
                (*sumap).second->m_locationInShader = loc;
            }
        }
    }
    // stores the uniform's name, locationInShader, and value.
    std::map< ShaderUniformName, ShaderUniformInterface* > m_uniforms;
};
typedef std::shared_ptr< ShaderUniformHolder > ShaderUniformHolderPtr;
typedef std::shared_ptr< const ShaderUniformHolder > ConstShaderUniformHolderPtr;

/// Responsible for holding the uniforms settings
/// Caches uniform settings to minimize chatter.
class ShaderInstance : public ShaderUniformHolder
{
public:
    ShaderInstance( const ShaderName& name, ShaderManagerPtr manager )
    : m_name( name ),
      m_manager( manager )
    {
    }

    //TODO$$$$ Need to be notified by ShaderManager when 
    // our shader is reloaded so we can call lookupUniformLocations()
    
    virtual ~ShaderInstance() { }
    const ShaderName& name( void ) const { return m_name; }

    GLuint getGLProgramIndex( void ) const
    {
        return m_manager->getProgramIndexForShaderName( m_name );
    }
    
    void use( void ) const
    {
        GLuint shaderIndex = getGLProgramIndex();
        LOG_TRACE(g_log) << "Use Shader Program " << shaderIndex;
        for( auto iter = m_uniforms.begin(); iter != m_uniforms.end(); ++iter )
        { LOG_TRACE(g_log) << "\tUniform: " << iter->first; }
        GL_CHECK( glUseProgram( shaderIndex ) );
        applyShaderUniforms( shaderIndex);
    }

protected:
    ShaderName m_name;
    ShaderManagerPtr m_manager;
};

#endif
