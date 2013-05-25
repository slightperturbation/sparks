#include "ShaderUniform.hpp"
#include "Utilities.hpp"
#include "ShaderInstance.hpp"


spark::ShaderUniformHolder
::~ShaderUniformHolder()
{
    // Noop
}

template<>
void 
spark::ShaderUniform<float>
::applyImpl( void ) const
{
    GL_CHECK( glUniform1f( m_locationInShader, m_val ) );
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
