#include "ShaderUniform.hpp"
#include "Utilities.hpp"


template<>
void ShaderUniform<float>::applyImpl( void )
{
    GL_CHECK( glUniform1f( m_locationInShader, m_val ) );
}
template<>
void ShaderUniform<int>::applyImpl( void )
{
    GL_CHECK( glUniform1i( m_locationInShader, m_val ) );
}
template<>
void ShaderUniform<unsigned int>::applyImpl( void )
{
    GL_CHECK( glUniform1ui( m_locationInShader, m_val ) );
}
template<>
void ShaderUniform<glm::vec3>::applyImpl( void )
{
    GL_CHECK( glUniform3fv( m_locationInShader, 1, glm::value_ptr(m_val) ) );
}
template<>
void ShaderUniform<glm::vec4>::applyImpl( void )
{
    GL_CHECK( glUniform4fv( m_locationInShader, 1, glm::value_ptr(m_val) ) );
}
template<>
void ShaderUniform<glm::mat3>::applyImpl( void )
{
    GL_CHECK( glUniformMatrix3fv( m_locationInShader,
        1 /*count*/,
        GL_FALSE /*transpose*/,
        glm::value_ptr(m_val) ) );
}
template<>
void ShaderUniform<glm::mat4>::applyImpl( void )
{
    GL_CHECK( glUniformMatrix4fv( m_locationInShader,
        1 /*count*/,
        GL_FALSE /*transpose*/,
        glm::value_ptr(m_val) ) );
}