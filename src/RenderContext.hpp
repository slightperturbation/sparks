#ifndef CAMERA_HPP
#define CAMERA_HPP

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <memory>
#include <iostream>
#include <string>

//////////////////////////////////////////////////////////////////////////
// Utilities

/// Wraps glBufferData to load from a std::vector
/// E.g.:
/// std::vector< MeshVertex > verts;
/// glBufferDataFromVector( GL_ARRAY_BUFFER, verts, GL_STATIC_DRAW );
template <typename VertexType>
inline void glBufferDataFromVector( GLenum targetBufferObjectType, const std::vector<VertexType>& data, GLenum usagePattern )
{
    glBufferData( targetBufferObjectType,
                 data.size() * sizeof(VertexType), // size in bytes of entire vertex vector
                 &(data.front()), // address of first elem
                 usagePattern );
}

std::string readFileToString( const char* filename );
GLuint createShaderWithErrorHandling( GLuint shaderType, const std::string& shaderSource );
GLuint loadShaderFromFile( const char* vertexShaderFilepath, const char* fragmentShaderFilepath );

// END Utilities
//////////////////////////////////////////////////////////////////////////



/// ShaderAttribute ties together a channel in the vertex data stream
/// with a name that will be referenced in corresponding shaders.
/// This class is abstract, subclasses are for the concrete data types (e.g. float)
/// TODO  Should be owned by a Shader class
class ShaderAttribute
{
public:
    ShaderAttribute(const std::string& attributeNameInShader,
                    GLsizei attributeSizeInBytes,
                    GLsizei sizeofVertexInBytes,
                    void* offsetVertexAttribute )
    :
    m_name( attributeNameInShader ),
    m_size( attributeSizeInBytes ),
    m_stride( sizeofVertexInBytes ),
    m_offset( offsetVertexAttribute )
    {}
    std::string m_name;
    GLsizei m_size;
    GLsizei m_stride; // sizeof( Vertex )
    void*   m_offset; // offset( Vertex, position/normal/texcoord/... )
    
    virtual ~ShaderAttribute() {}
    void enableByNameInShader( GLuint shaderProgramIndex )
    {
        GLint index = glGetAttribLocation( shaderProgramIndex, m_name.c_str() );  checkOpenGLErrors();
        if( index == -1 )
        {
            std::cerr << "ShaderAttribute::enableByNameInShader() | Failed to find shader attribute of name \""
            << m_name << "\" in shader #" << shaderProgramIndex << "\n";
        } else {
            glEnableVertexAttribArray( index );  checkOpenGLErrors();
        }
    }
    void defineByNameInShader( GLuint shaderProgramIndex )
    {
        GLint index = glGetAttribLocation( shaderProgramIndex, m_name.c_str() );  checkOpenGLErrors();
        if( index == -1 )
        {
            std::cerr << "ShaderAttribute::defineByNameInShader() | Failed to find shader attribute of name \""
            << m_name << "\" in shader #" << shaderProgramIndex << "\n";
        } else {
            assignPointerByIndex( index );
        }
    }
    void disableByNameInShader( GLuint shaderProgramIndex )
    {
        GLint index = glGetAttribLocation( shaderProgramIndex, m_name.c_str() );  checkOpenGLErrors();
        if( index == -1 )
        {
            std::cerr << "UniformShaderAttribute::disableByNameInShader() | Failed to find shader attribute of name \""
            << m_name << "\" in shader #" << shaderProgramIndex << "\n";
        } else {
            glDisableVertexAttribArray( index );  checkOpenGLErrors();
        }
    }
protected:
    
    /// PRE if attribute has an offset, ARRAY_BUFFER must be bound
    virtual void assignPointerByIndex(GLuint attribIndex) = 0;
};
typedef std::shared_ptr<ShaderAttribute> ShaderAttributePtr;

class FloatShaderAttribute : public ShaderAttribute
{
public:
    FloatShaderAttribute(const std::string& attributeNameInShader,
                         GLsizei attributeSizeInBytes,
                         GLsizei sizeofVertexInBytes,
                         void* offsetVertexAttribute )
    : ShaderAttribute(attributeNameInShader,
                      attributeSizeInBytes,
                      sizeofVertexInBytes,
                      offsetVertexAttribute)
    {}
    virtual ~FloatShaderAttribute() {}
    virtual void assignPointerByIndex(GLuint attribIndex)
    {
        //std::cerr << "Assigning float VertexAttribPointer \"" << m_name << "\".\n";
        glVertexAttribPointer( attribIndex, m_size, GL_FLOAT, GL_FALSE, m_stride, m_offset );  checkOpenGLErrors();
    }
};

class UByteShaderAttribute : public ShaderAttribute
{
public:
    UByteShaderAttribute(const std::string& attributeNameInShader,
                         GLsizei attributeSizeInBytes,
                         GLsizei sizeofVertexInBytes,
                         void* offsetVertexAttribute )
    : ShaderAttribute(attributeNameInShader,
                      attributeSizeInBytes,
                      sizeofVertexInBytes,
                      offsetVertexAttribute)
    {}
    virtual ~UByteShaderAttribute() {}
    virtual void assignPointerByIndex(GLuint attribIndex)
    {
        glVertexAttribPointer( attribIndex, m_size, GL_UNSIGNED_BYTE, GL_FALSE, m_stride, m_offset );  checkOpenGLErrors();
    }
};



/// Simple Point lights only for now
class Light
{
public:
    Light()
        : m_intensity( 1.0 ),
          m_color(  1.0f, 0.2f, 0.2f ),
          m_position( 0.5f, 0.5f, 1.0f )
    {
        m_position = glm::normalize( m_position );
    }
    float m_intensity;
    glm::vec3 m_color;
    glm::vec3 m_position;
};

/// RenderContext manages state for the Model, View and Projection transforms.
class RenderContext
{
public:
    RenderContext() : 
      m_cameraPos( 1.5f, 0.5f, 0.0f ),
      m_cameraTarget( 0, 0, 0 ),
      m_cameraUp( 0.0f, 0.0f, 1.0f ),
      m_fov( 50.0f ),
      m_aspectRatio( 800.0f/600.0f ),
      m_nearPlaneDist( 0.01f ),
      m_farPlaneDist( 20.0f )
    {
    }

    RenderContext( const RenderContext& that ) : 
      m_modelMatrix( that.m_modelMatrix ),
      m_cameraPos( that.m_cameraPos ),
      m_cameraTarget( that.m_cameraTarget ),
      m_cameraUp( that.m_cameraUp ),
      m_fov( that.m_fov ),
      m_aspectRatio( that.m_aspectRatio ),
      m_nearPlaneDist( that.m_nearPlaneDist ),
      m_farPlaneDist( that.m_farPlaneDist )
    {

    }

          glm::vec3& cameraUp( void )        { return m_cameraUp; }
    const glm::vec3& cameraUp( void ) const  { return m_cameraUp; }
          glm::vec3& cameraPos( void )       { return m_cameraPos; }
    const glm::vec3& cameraPos( void ) const { return m_cameraPos; }
          glm::vec3& cameraTarget( void )       { return m_cameraTarget; }
    const glm::vec3& cameraTarget( void ) const { return m_cameraTarget; }

    void setCameraPos( const glm::vec3& pos ) { m_cameraPos = pos; }
    void setCameraTarget( const glm::vec3& target ) { m_cameraTarget = target; }
    void setModelMatrix( const glm::mat4& modelMatrix ) { m_modelMatrix = modelMatrix; }
    void setAspectRatio( float ratio ) { m_aspectRatio = ratio; }
    
    const glm::mat4& modelMatrix( void ) const { return m_modelMatrix; }
    glm::mat4&       modelMatrix( void )       { return m_modelMatrix; }

    glm::mat4 getModelViewProjMatrix( void ) const 
    {
        return getViewProjMatrix() * m_modelMatrix;
    }

    glm::mat4 getViewProjMatrix( void ) const
    {
        return projectionMatrix() * viewMatrix();
    }
    
    glm::mat4 viewMatrix( void ) const
    {
        return glm::lookAt(m_cameraPos,
                           m_cameraTarget,
                           m_cameraUp );
    }
    
    glm::mat4 modelViewMatrix( void ) const 
    {
        return viewMatrix() * modelMatrix();
    }

    glm::mat4 projectionMatrix( void ) const
    {
        return glm::perspective( m_fov, m_aspectRatio, m_nearPlaneDist, m_farPlaneDist );
    }

    void setLightPosition( const float* dir )
    {
        m_shadowCastingLight.m_position = glm::vec3( dir[0], dir[1], dir[2] );
    }
    
    const glm::vec3& lightPosition( void ) const { return m_shadowCastingLight.m_position; }
          glm::vec3& lightPosition( void )       { return m_shadowCastingLight.m_position; }
    
    const glm::vec3& lightColor( void ) const { return m_shadowCastingLight.m_color; }
          glm::vec3& lightColor( void )       { return m_shadowCastingLight.m_color; }
    

private:
    glm::mat4 m_modelMatrix;
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraTarget;
    glm::vec3 m_cameraUp;
    float m_fov;
    float m_aspectRatio;
    float m_nearPlaneDist;
    float m_farPlaneDist;
    Light m_shadowCastingLight;
};
typedef std::shared_ptr< RenderContext > RenderContextPtr;
typedef std::shared_ptr< const RenderContext > ConstRenderContextPtr;

#endif