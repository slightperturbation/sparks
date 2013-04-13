#ifndef MESH_HPP
#define MESH_HPP

#include "SoftTestDeclarations.hpp"

#include "Renderable.hpp"
#include "Perspective.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <Eigen/Dense>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <memory>
#include <iostream>
#include <vector>

#include "config.hpp"  // Defines DATA_PATH
#include "VertexAttribute.hpp"


// Standard vertex type for mesh objects
class MeshVertex
{
public:
    GLfloat m_position[4];
    GLfloat m_normal[4];
    GLfloat m_texCoord[3];
    GLfloat m_diffuseColor[4];
    GLfloat m_shininess;
    GLbyte  m_specular[4];

    static void addVertexAttributes( std::vector<VertexAttributePtr>&  outShaderAttributes )
    {
        VertexAttributePtr position(new FloatVertexAttribute("position",
            3,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, m_position) )
            );
        outShaderAttributes.push_back( position );

        VertexAttributePtr diffuse(new FloatVertexAttribute("inVertexColor",
            4,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, m_diffuseColor) )
            );
        outShaderAttributes.push_back( diffuse );
        
        VertexAttributePtr texCoord3d(new FloatVertexAttribute("texCoord3d",
            3,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, m_texCoord) )
            );
        outShaderAttributes.push_back( texCoord3d );
    }
};


struct LineVertex
{
    GLfloat m_position[4];
    GLfloat m_direction[4];
    GLfloat m_diffuseColor[4];

    static void addVertexAttributes( std::vector<VertexAttributePtr>&  outShaderAttributes )
    {
        VertexAttributePtr position(new FloatVertexAttribute("position", 3,
            sizeof(LineVertex),
            (void*)offsetof(LineVertex, m_position) )
            );
        outShaderAttributes.push_back( position );
        VertexAttributePtr direction(new FloatVertexAttribute("direction",
            3,
            sizeof(LineVertex),
            (void*)offsetof(LineVertex, m_direction) )
            );
        outShaderAttributes.push_back( direction );
        VertexAttributePtr diffuse(new FloatVertexAttribute("inColor",
            3,
            sizeof(LineVertex),
            (void*)offsetof(LineVertex, m_diffuseColor) )
            );
        outShaderAttributes.push_back( diffuse );
    }
};



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



// TODO -- template-tize over MeshVertex
/// Mesh supports rendering of a indexed set of triangles
class Mesh : public Renderable
{
public:
    Mesh( const char* vertexShaderFilepath = DATA_PATH "/shaders/volumeVertexShader.glsl",
         const char* fragmentShaderFilepath = DATA_PATH "/shaders/volumeFragmentShader.glsl" );

    virtual ~Mesh();

    /// Example of changing the mesh geometry.
    virtual void update( float dt );
    virtual void setupRenderState( void );
    virtual void setupShaderState( PerspectivePtr renderContext );
    virtual void teardownRenderState( void );
    virtual void render( PerspectivePtr renderContext );
    virtual void loadShaders();
    virtual void loadTextures();
    void setShaderUniformMatrix( const char* uniformShaderName, const glm::mat4& mat);
    void setShaderUniformVector( const char* uniformShaderName, const glm::vec3& vec );
    void setShaderUniformInt( const char* uniformShaderName, GLint vec );

    void unitCube();
    
    void clearGeometry( void );
    void resizeVertexArray( size_t newSize );
    void setVertex( size_t i, const Eigen::Vector3f& a, 
        const Eigen::Vector2f& textureCoords, 
        const Eigen::Vector4f color, 
        const Eigen::Vector3f norm );
    size_t addVertex( const Eigen::Vector3f& a, 
                      const Eigen::Vector2f& textureCoords, 
                      const Eigen::Vector4f color, 
                      const Eigen::Vector3f norm );
    void addTriangleByIndex( unsigned int a, unsigned int b, unsigned int c );

    void addQuad( const Eigen::Vector3f& a, const Eigen::Vector2f& aCoord, 
                  const Eigen::Vector3f& b, const Eigen::Vector2f& bCoord, 
                  const Eigen::Vector3f& c, const Eigen::Vector2f& cCoord, 
                  const Eigen::Vector3f& d, const Eigen::Vector2f& dCoord, 
                  const Eigen::Vector3f& norm );
    
    /// Bind geometry data to buffers
    void bindDataToBuffers( void );

    /// Construction methods
    static RenderablePtr createBox( void );

protected:
    void attachShaderAttributes( void );

    GLuint m_vertexArrayObjectId;
    GLuint m_vertexBufferId;
    GLuint m_elementBufferId;
    GLuint m_shaderProgramIndex;
    std::vector< VertexAttributePtr > m_attributes;
    std::vector< MeshVertex > m_vertexData;
    std::vector< unsigned int > m_vertexIndicies;
    glm::mat4 m_modelTransform;
    std::string m_vertexShaderFilepath;
    std::string m_fragmentShaderFilepath;
};
typedef std::shared_ptr< Mesh > MeshPtr;

#endif

