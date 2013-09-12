//
//  VertexAttribute.hpp
//  sparks
//
//  Created by Brian Allen on 4/8/13.
//
//

#ifndef sparks_VertexAttribute_hpp
#define sparks_VertexAttribute_hpp

#include "Spark.hpp"

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
namespace spark
{
    /// VertexAttribute ties together a channel in the vertex data stream
    /// with a name that will be referenced in corresponding shaders.
    /// This class is abstract, subclasses are for the concrete data types (e.g. float)
    /// TODO  Should be owned by a Shader class
    class VertexAttribute
    {
    public:
        VertexAttribute(const std::string& attributeNameInShader,
                        GLsizei attributeComponentCount,
                        GLsizei sizeofVertexInBytes,
                        void* offsetVertexAttribute )
        :
        m_name( attributeNameInShader ),
        m_size( attributeComponentCount ),
        m_stride( sizeofVertexInBytes ),
        m_offset( offsetVertexAttribute )
        {}
        std::string m_name;
        GLsizei m_size;
        GLsizei m_stride; // sizeof( Vertex )
        void*   m_offset; // offset( Vertex, position/normal/texcoord/... )
    
        virtual ~VertexAttribute() {}
        void enableByNameInShader( GLuint shaderProgramIndex )
        {
            GLint index;
            GL_CHECK( index = glGetAttribLocation( shaderProgramIndex, m_name.c_str() ) );
            if( index == GL_INVALID_OPERATION )
            {
                LOG_DEBUG(g_log) << "VertexAttribute::enableByNameInShader() | Failed to find shader attribute of name \""
                << m_name << "\" in shader #" << shaderProgramIndex << "\n";
            }
            else if( index == -1 )
            {
                LOG_TRACE(g_log) << "VertexAttribute \"" << m_name << "\" not found in shader program "
                << shaderProgramIndex << "\n";
            }
            else
            {
                GL_CHECK( glEnableVertexAttribArray( index ) );
            }
        }
        void defineByNameInShader( GLuint shaderProgramIndex )
        {
            GLint index;
            GL_CHECK( index = glGetAttribLocation( shaderProgramIndex, m_name.c_str() ) );
            if( index == GL_INVALID_OPERATION )
            {
                LOG_DEBUG(g_log) << "VertexAttribute::defineByNameInShader() | Failed to find shader attribute of name \""
                << m_name << "\" in shader #" << shaderProgramIndex << "\n";
            }
            else if( index == -1 )
            {
                LOG_TRACE(g_log) << "VertexAttribute \"" << m_name << "\" not found in shader program "
                << shaderProgramIndex << "\n";
            }
            else
            {
                assignPointerByIndex( index );
            }
        }
        void disableByNameInShader( GLuint shaderProgramIndex )
        {
            GLint index;
            GL_CHECK( index = glGetAttribLocation( shaderProgramIndex, m_name.c_str() ) );
            if( index == GL_INVALID_OPERATION )
            {
                LOG_DEBUG(g_log) << "UniformShaderAttribute::disableByNameInShader() | Failed to find shader attribute of name \""
                << m_name << "\" in shader #" << shaderProgramIndex << "\n";
            }
            else if( index == -1 )
            {
                LOG_TRACE(g_log) << "VertexAttribute \"" << m_name << "\" not found in shader program "
                << shaderProgramIndex << "\n";
            }
            else
            {
                GL_CHECK( glDisableVertexAttribArray( index ) );
            }
        }
    protected:
    
        /// PRE if attribute has an offset, ARRAY_BUFFER must be bound
        virtual void assignPointerByIndex(GLuint attribIndex) = 0;
    };
    typedef spark::shared_ptr<VertexAttribute> VertexAttributePtr;

    class FloatVertexAttribute : public VertexAttribute
    {
    public:
        FloatVertexAttribute(const std::string& attributeNameInShader,
                             GLsizei attributeComponentCount,
                             GLsizei sizeofVertexInBytes,
                             void* offsetVertexAttribute )
        : VertexAttribute(attributeNameInShader,
                          attributeComponentCount,
                          sizeofVertexInBytes,
                          offsetVertexAttribute)
        {}
        virtual ~FloatVertexAttribute() {}
        virtual void assignPointerByIndex(GLuint attribIndex)
        {
            LOG_TRACE(g_log) << "Assigning float VertexAttribPointer \"" << m_name << "\" size="
            << m_size << ", stride=" << m_stride << ", offset=" << m_offset 
            << " to current GL_ARRAY_BUFFER. [ glVertexAttribPointer ]";
            GL_CHECK( glVertexAttribPointer( attribIndex, m_size, GL_FLOAT, GL_FALSE, m_stride, m_offset ) );
        }
    };

    class UByteVertexAttribute : public VertexAttribute
    {
    public:
        UByteVertexAttribute(const std::string& attributeNameInShader,
                             GLsizei attributeComponentCount,
                             GLsizei sizeofVertexInBytes,
                             void* offsetVertexAttribute )
        : VertexAttribute(attributeNameInShader,
                          attributeComponentCount,
                          sizeofVertexInBytes,
                          offsetVertexAttribute)
        {}
        virtual ~UByteVertexAttribute() {}
        virtual void assignPointerByIndex(GLuint attribIndex)
        {
            GL_CHECK( glVertexAttribPointer( attribIndex, m_size, GL_UNSIGNED_BYTE, GL_FALSE, m_stride, m_offset ) );
        }
    };
} // end namespace spark
#endif
