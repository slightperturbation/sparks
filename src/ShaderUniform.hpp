//
//  ShaderUniform.hpp
//  sparks
//
//  Created by Brian Allen on 4/8/13.
//
//

#ifndef sparks_ShaderUniform_hpp
#define sparks_ShaderUniform_hpp

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <typeinfo>
#include <map>

class Shader;

template<typename T>
class ShaderUniform;

/// Abstract, unused super-class for managing ShaderUniforms
/// Responsible for whether the shader's state needs to be set to a new value.
class ShaderUniformInterface
{
public:
    ShaderUniformInterface() : m_locationInShader(-1), m_dirty( true ) {}
    virtual ~ShaderUniformInterface() {}
    /// Return a cast of the ShaderUniform type.
    template<typename T> ShaderUniform<T>* as( void )
    {
        return dynamic_cast< ShaderUniform<T>* > ( this );
    }
    /// Set this uniform to the stored value.
    void apply( void )
    {
        if( m_dirty && (m_locationInShader != -1) )
        {
            applyImpl();
            m_dirty = false;
        }
    }
    friend class Shader;
protected:
    virtual void applyImpl( void ) = 0;
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
protected:
    /// No default implementation, only specializations (below)
    /// So unrecognized types will cause a compile error.
    virtual void applyImpl( void );
};

// Specializations for supported types.
template<>
void ShaderUniform<float>::applyImpl( void );
template<>
void ShaderUniform<int>::applyImpl( void );
template<>
void ShaderUniform<unsigned int>::applyImpl( void );
template<>
void ShaderUniform<glm::vec3>::applyImpl( void );
template<>
void ShaderUniform<glm::vec4>::applyImpl( void );
template<>
void ShaderUniform<glm::mat3>::applyImpl( void );
template<>
void ShaderUniform<glm::mat4>::applyImpl( void );



#endif