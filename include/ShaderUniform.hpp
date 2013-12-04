//
//  ShaderUniform.hpp
//  sparks
//
//  Created by Brian Allen on 4/8/13.
//
//

#ifndef sparks_ShaderUniform_hpp
#define sparks_ShaderUniform_hpp

#include "Spark.hpp"

#include "Utilities.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <typeinfo>
#include <map>

namespace spark
{
    class ShaderInstance;
    class ShaderUniformHolder;

    template<typename T>
    class ShaderUniform;

    /// Abstract base class for ShaderUniforms, allowing abstract pointers.
    /// Responsible for whether the shader's state needs to be set to a new value.
    class ShaderUniformInterface
    {
    public:
        ShaderUniformInterface() : m_locationInShader(-1), m_dirty( true ) {}
        virtual ~ShaderUniformInterface() {}
        /// Return a cast of the ShaderUniform type.
        template<typename T> ShaderUniform<T>* as( void )
        {
            ShaderUniform<T>* out = dynamic_cast< ShaderUniform<T>* > ( this );
            if( !out )
            {
                LOG_ERROR(g_log) << "ShaderUniform typecast failed--  Unable to cast \""
                    << typeid( *this ).name() << "\" into \"" 
                    << typeid( ShaderUniform<T> ).name() << "\".";
            }
            return out;
        }
        /// Set this uniform to the stored value.
        void apply( void ) const
        {
            if( m_dirty && (m_locationInShader != -1) )
            {
                applyImpl();
                //TODO if used in multi-threaded environment -- surround with mutex
                //(const_cast<ShaderUniformInterface*>(this))->m_dirty = false;
            }
        }
        virtual std::string toString( void ) const = 0;
        friend class ShaderInstance;
        friend class ShaderUniformHolder;
    protected:
        virtual void applyImpl( void ) const = 0;
        GLint m_locationInShader;
    private:
        bool m_dirty;
    };

    /// Concrete class holding the data for a Shader Uniform value.
    /// Typical usage:
    ///   shader.getUniform<int>("count")->set( 2 );
    /// And used by shader to call:
    ///   m_shaderUniforms["count"].apply();
    template<typename T>
    class ShaderUniform : public ShaderUniformInterface
    {
        T m_val;
    public:
        ShaderUniform( void ) { }
        ShaderUniform( const T& val ) : m_val( val ) { }
        virtual ~ShaderUniform() {}
        void set( const T& val ) { m_val = val; }
        virtual std::string toString( void ) const
        { std::stringstream ss; ss << m_val; return ss.str(); }
    protected:
        /// No default implementation, only specializations (below)
        /// So unrecognized types will cause a compile error.
        virtual void applyImpl( void ) const;
    };

    // Concrete specializations for supported types.
    template<>
    void ShaderUniform<float>::applyImpl( void ) const;
    template<>
    void ShaderUniform<double>::applyImpl( void ) const;
    template<>
    void ShaderUniform<int>::applyImpl( void ) const;
    template<>
    void ShaderUniform<unsigned int>::applyImpl( void ) const;
    template<>
    void ShaderUniform<bool>::applyImpl( void ) const;
    template<>
    void ShaderUniform<glm::vec2>::applyImpl( void ) const;
    template<>
    void ShaderUniform<glm::vec3>::applyImpl( void ) const;
    template<>
    void ShaderUniform<glm::vec4>::applyImpl( void ) const;
    template<>
    void ShaderUniform<glm::mat3>::applyImpl( void ) const;
    template<>
    void ShaderUniform<glm::mat4>::applyImpl( void ) const;
} // end namespace spark
#endif
