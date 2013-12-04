#include "ShaderUniform.hpp"
#include "Utilities.hpp"
#include "ShaderInstance.hpp"

template<>
void 
spark::ShaderUniform<float>
::applyImpl( void ) const
{
    GL_CHECK( glUniform1f( m_locationInShader, m_val ) );
}
template<>
void 
spark::ShaderUniform<double>
::applyImpl( void ) const
{
    GL_CHECK( glUniform1d( m_locationInShader, m_val ) );
}
template<>
void 
spark::ShaderUniform<int>
::applyImpl( void ) const
{
    GL_CHECK( glUniform1i( m_locationInShader, m_val ) );
}
template<>
void 
spark::ShaderUniform<unsigned int>
::applyImpl( void ) const
{
    GL_CHECK( glUniform1ui( m_locationInShader, (GLuint)m_val ) );
}
template<>
void 
spark::ShaderUniform<bool>
::applyImpl( void ) const
{
    // Note, there's no way to pass a bool per se.
    // and the uniform bool takes 32-bits just as the int does,
    // so there wouldn't be any savings to passing a bool.
    GL_CHECK( glUniform1i( m_locationInShader, (GLint)m_val ) );
}
template<>
void 
spark::ShaderUniform<glm::vec2>
::applyImpl( void ) const
{
    GL_CHECK( glUniform2fv( m_locationInShader, 1, glm::value_ptr(m_val) ) );
}
template<>
void 
spark::ShaderUniform<glm::vec3>
::applyImpl( void ) const
{
    GL_CHECK( glUniform3fv( m_locationInShader, 1, glm::value_ptr(m_val) ) );
}
template<>
void 
spark::ShaderUniform<glm::vec4>
::applyImpl( void ) const
{
    GL_CHECK( glUniform4fv( m_locationInShader, 1, glm::value_ptr(m_val) ) );
}
template<>
void
spark::ShaderUniform<glm::mat3>
::applyImpl( void ) const
{
    GL_CHECK( glUniformMatrix3fv( m_locationInShader,
        1 /*count*/,
        GL_FALSE /*transpose*/,
        glm::value_ptr(m_val) ) );
}
template<>
void 
spark::ShaderUniform<glm::mat4>
::applyImpl( void ) const
{
    GL_CHECK( glUniformMatrix4fv( m_locationInShader,
        1 /*count*/,
        GL_FALSE /*transpose*/,
        glm::value_ptr(m_val) ) );
}
