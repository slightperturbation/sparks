//
//  ShaderInstance.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_ShaderInstance_hpp
#define sparks_ShaderInstance_hpp

#include "config.hpp"  // Defines DATA_PATH
#include "Spark.hpp"
#include "Utilities.hpp"
#include "ShaderUniform.hpp"
#include "ShaderManager.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <typeinfo>
#include <map>
#include <memory>

namespace spark
{
    /// ShaderUniformHolder keeps track of the uniforms used by a shader.
    class ShaderUniformHolder
    {
    public:
        ShaderUniformHolder() : m_isDirty( true ) { }
        virtual ~ShaderUniformHolder()
        {
            for( auto sumap = m_uniforms.begin(); sumap != m_uniforms.end(); ++sumap )
            {
                delete sumap->second;
            }
        }
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
            if( g_log->isTrace() )
            {
                LOG_TRACE(g_log) << "Setting uniform " << name << " = " << val;
            }
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
            if( g_log->isTrace() )
            {
                LOG_TRACE(g_log) << "Creating uniform " << name << " with default value.";
            }
            m_uniforms[name] = new ShaderUniform<T>();
            m_isDirty = true;
        }
        
        /// ShaderManager markDirty() when the underlying shader code is reloaded.
        friend class ShaderManager;
    protected:
        /// Mark the uniform locations as dirty, needing to be looked up.
        /// Should only be called by ShaderManager.
        void markDirty( void ) { m_isDirty = true; }

        /// Set uniforms on this shader (when dirty)
        /// OpenGL state change:  current program is set.
        void applyShaderUniforms( GLint a_shaderProgramIndex ) const
        {
            if( m_isDirty )
            {
                lookupUniformLocations( a_shaderProgramIndex );
            }
            for( auto sumap = m_uniforms.begin(); sumap != m_uniforms.end(); ++sumap )
            {
                if( g_log->isTrace() )
                {
                    LOG_TRACE(g_log) << "Applying ShaderUniform " << (*sumap).first
                        << " = " << (*sumap).second->toString();
                }
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
            m_isDirty = false;
        }
    private:
        /// m_isDirty tracks if the shader's knowledge of the uniform *locations*
        /// are new, requiring lookupUniformLocations()
        /// m_isDirty is set if a new uniform is created, not if a uniform value changes.
        /// Of course, locations also change if the shader is recompiled,
        /// but that is the responsibility of the owning ShaderManager.
        mutable bool m_isDirty;
        // stores the uniform's name, locationInShader, and value.
        std::map< ShaderUniformName, ShaderUniformInterface* > m_uniforms;
    };
    typedef spark::shared_ptr< ShaderUniformHolder > ShaderUniformHolderPtr;
    typedef spark::shared_ptr< const ShaderUniformHolder > ConstShaderUniformHolderPtr;

    /// A ShaderInstance is a combination of a GLSL shader (with ShaderName)
    /// and the set of uniforms that specify how this shader is rendered.
    /// The uniforms are stored in the superclass ShaderUniformHolder.
    /// 
    /// ShaderInstace caches uniform settings to minimize OpenGL chatter.
    class ShaderInstance : public ShaderUniformHolder
    {
    public:
        /// Create the shader instance based on the shader name that has been
        /// or will be loaded into manager.
        ShaderInstance( const ShaderName& name, ShaderManagerPtr manager )
        : m_name( name ),
          m_manager( manager )
        { }
        virtual ~ShaderInstance() { }
        
        /// Notified by ShaderManager when
        /// our shader is reloaded so we can call lookupUniformLocations()
        void refreshUniformLocations( void )
        {
            LOG_DEBUG(g_log) << "Refresh \"" << m_name << "\" shader uniform locations.";
            lookupUniformLocations( getGLProgramIndex() );
        }
    
        const ShaderName& name( void ) const { return m_name; }
        
        /// Use shader program. Call sets the GL program (glUseProgram) and
        /// sets the shader uniforms.
        void use( void ) const
        {
            GLuint shaderIndex = getGLProgramIndex();
            if( g_log->isTrace() )
            {
                LOG_TRACE(g_log) << "Use Shader Program " << shaderIndex
                                 << " \"" << name() << "\".";
            }
            GL_CHECK( glUseProgram( shaderIndex ) );
            applyShaderUniforms( shaderIndex);
        }
    
        /// Check the shader and associated uniforms for errors.
        void preDrawValidation( void ) const
        {
            GLuint shaderIndex = getGLProgramIndex();
            bool isDebugTests = true;
            if( isDebugTests )
            {
                glValidateProgram( shaderIndex );
                GLint status = 0;
                glGetProgramiv( shaderIndex, GL_VALIDATE_STATUS, &status );
                if( status == GL_FALSE )
                {
                    const size_t logSize = 2048;
                    GLchar errorLog[ logSize ] = {0};
                    glGetProgramInfoLog( shaderIndex, logSize, NULL, errorLog );
                    LOG_ERROR(g_log) << "Bad shader program: \""
                    << name() << "\":\n" << errorLog;
                }
            }
        }
        
        /// Returns the OpenGL program index for this shader.
        GLuint getGLProgramIndex( void ) const
        {
            return m_manager->getProgramIndexForShaderName( m_name );
        }

    protected:
        /// The name of the shader as registered with m_manager.
        ShaderName m_name;
        ShaderManagerPtr m_manager;
    };
} // end namespace spark
#endif
