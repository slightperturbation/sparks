#ifndef MESH_HPP
#define MESH_HPP

#include "SoftTestDeclarations.hpp"

#include "Renderable.hpp"
#include "Projection.hpp"

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

// Forward declaration of AssetImporter class for mesh loading
struct aiMesh;

namespace spark
{
    // Standard vertex type for mesh objects
    class MeshVertex
    {
    public:
        GLfloat m_position[4];
        GLfloat m_normal[4];
        GLfloat m_diffuseColor[4];
        GLfloat m_texCoord[3];

        /// Set some sane initial values.
        MeshVertex( void )
        {
            for( size_t i = 0; i < 4; ++i )
            {
                m_position[i] = 0.0;
                m_normal[i] = 0.0;
                m_diffuseColor[i] = 1.0;
            }
            m_normal[0] = 1.0;
            m_texCoord[0] = 0.5; m_texCoord[1] = 0.5; m_texCoord[2] = 0.5;
        }

        /// Defines the names of the "in" data channels for the vertex shader
        /// By convention, per-vertex attributes start with "v_"
        static void addVertexAttributes( std::vector<VertexAttributePtr>& outShaderAttributes )
        {
            VertexAttributePtr position(new FloatVertexAttribute("v_position",
                3,
                sizeof(MeshVertex),
                (void*)offsetof(MeshVertex, m_position) )
                );
            outShaderAttributes.push_back( position );
        
            VertexAttributePtr normal(new FloatVertexAttribute("v_normal",
                3,
                sizeof(MeshVertex),
                (void*)offsetof(MeshVertex, m_normal) )
                );
            outShaderAttributes.push_back( normal );

            VertexAttributePtr diffuse(new FloatVertexAttribute("v_color",
                4,
                sizeof(MeshVertex),
                (void*)offsetof(MeshVertex, m_diffuseColor) )
                );
            outShaderAttributes.push_back( diffuse );
        
            VertexAttributePtr texCoord3d(new FloatVertexAttribute("v_texCoord",
                3,
                sizeof(MeshVertex),
                (void*)offsetof(MeshVertex, m_texCoord) )
                );
            outShaderAttributes.push_back( texCoord3d );
        }
    };


    //struct LineVertex
    //{
    //    GLfloat m_position[4];
    //    GLfloat m_direction[4];
    //    GLfloat m_diffuseColor[4];
    //
    //    static void addVertexAttributes( std::vector<VertexAttributePtr>&  outShaderAttributes )
    //    {
    //        VertexAttributePtr position(new FloatVertexAttribute("position", 3,
    //            sizeof(LineVertex),
    //            (void*)offsetof(LineVertex, m_position) )
    //            );
    //        outShaderAttributes.push_back( position );
    //        VertexAttributePtr direction(new FloatVertexAttribute("direction",
    //            3,
    //            sizeof(LineVertex),
    //            (void*)offsetof(LineVertex, m_direction) )
    //            );
    //        outShaderAttributes.push_back( direction );
    //        VertexAttributePtr diffuse(new FloatVertexAttribute("inColor",
    //            3,
    //            sizeof(LineVertex),
    //            (void*)offsetof(LineVertex, m_diffuseColor) )
    //            );
    //        outShaderAttributes.push_back( diffuse );
    //    }
    //};



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
    class Mesh : public Renderable, public Updatable
    {
    public:
        Mesh( void );
        virtual ~Mesh();

        /// Renderable
        virtual void render( void ) const override;
        // Updatable
        virtual void update( float dt ) override;
        void clearGeometry( void );
        void resizeVertexArray( size_t newSize );
        void setVertex( size_t i, const Eigen::Vector3f& a, 
            const Eigen::Vector2f& textureCoords, 
            const Eigen::Vector4f& color, 
            const Eigen::Vector3f& norm );
        size_t addVertex( const Eigen::Vector3f& a, 
                          const Eigen::Vector2f& textureCoords, 
                          const Eigen::Vector4f& color, 
                          const Eigen::Vector3f& norm );
        size_t addVertex( const MeshVertex& v );
        void addTriangleByIndex( unsigned int a, unsigned int b, unsigned int c );

        void addQuad( const Eigen::Vector3f& a, const Eigen::Vector2f& aCoord, 
                      const Eigen::Vector3f& b, const Eigen::Vector2f& bCoord, 
                      const Eigen::Vector3f& c, const Eigen::Vector2f& cCoord, 
                      const Eigen::Vector3f& d, const Eigen::Vector2f& dCoord, 
                      const Eigen::Vector3f& norm );
    
        /// Bind geometry data to buffers
        void bindDataToBuffers( void );
    
        /// Bind shader to vertex data
        virtual void attachShaderAttributes( GLuint aShaderProgramIndex );

        /// Construction methods
        void unitCube( void );
    
        /// Load the mesh and associated materials from the given filename
        bool createMeshFromFile( const std::string& filename,
                                 TextureManagerPtr tm,
                                 ShaderManagerPtr sm,
                                 const RenderPassName& renderPassName );

        static MeshPtr createMeshFromAiMesh( const aiMesh* meshNode, 
                                             float scale = 1.0 );

        static RenderablePtr createBox( TextureManagerPtr tm, 
                                        ShaderManagerPtr sm,
                                        const RenderPassName& renderPassName );
    protected:
        GLuint m_vertexArrayObjectId;
        GLuint m_vertexBufferId;
        GLuint m_elementBufferId;
        std::vector< VertexAttributePtr > m_attributes;
        std::vector< MeshVertex > m_vertexData;
        std::vector< GLuint > m_vertexIndicies;
        glm::mat4 m_modelTransform;
    };
    typedef std::shared_ptr< Mesh > MeshPtr;
}
#endif

