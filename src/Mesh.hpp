#ifndef MESH_HPP
#define MESH_HPP

#include "Renderable.hpp"
#include "RenderContext.hpp"

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

    static void addShaderAttributes( std::vector<ShaderAttributePtr>&  outShaderAttributes )
    {
        ShaderAttributePtr position(new FloatShaderAttribute("position", 3,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, m_position) )
            );
        outShaderAttributes.push_back( position );
        ShaderAttributePtr diffuse(new FloatShaderAttribute("inVertexColor",
            3,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, m_diffuseColor) )
            );
        outShaderAttributes.push_back( diffuse );
        ShaderAttributePtr texCoord3d(new FloatShaderAttribute("texCoord3d",
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

    static void addShaderAttributes( std::vector<ShaderAttributePtr>&  outShaderAttributes )
    {
        ShaderAttributePtr position(new FloatShaderAttribute("position", 3,
            sizeof(LineVertex),
            (void*)offsetof(LineVertex, m_position) )
            );
        outShaderAttributes.push_back( position );
        ShaderAttributePtr direction(new FloatShaderAttribute("direction",
            3,
            sizeof(LineVertex),
            (void*)offsetof(LineVertex, m_direction) )
            );
        outShaderAttributes.push_back( direction );
        ShaderAttributePtr diffuse(new FloatShaderAttribute("inColor",
            3,
            sizeof(LineVertex),
            (void*)offsetof(LineVertex, m_diffuseColor) )
            );
        outShaderAttributes.push_back( diffuse );
    }
};




// TODO -- templatize over MeshVertex
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
    virtual void setupShaderState( const RenderContext& renderContext );
    virtual void teardownRenderState( void );
    virtual void render( const RenderContext& renderContext );
    virtual void loadShaders();
    virtual void loadTextures()  {}
    void unitCube();
    
    void clearGeometry( void );
    void addQuad( const Eigen::Vector3f& a,
                  const Eigen::Vector3f& b,
                  const Eigen::Vector3f& c,
                  const Eigen::Vector3f& d,
                  const Eigen::Vector3f& norm );

    /// Construction methods
    static RenderablePtr createBox( void );

protected:
    std::string m_vertexShaderFilepath;
    std::string m_fragmentShaderFilepath;

    GLuint m_dataTextureId;
    GLuint m_vertexArrayObjectId;
    GLuint m_vertexBufferId;
    GLuint m_elementBufferId;
    GLuint m_shaderProgramIndex;
    std::vector< ShaderAttributePtr > m_attributes;
    std::vector< MeshVertex > m_vertexData;
    std::vector< unsigned int > m_vertexIndicies;
    glm::mat4 m_modelTransform;

};
typedef std::shared_ptr< Mesh > MeshPtr;

#endif

