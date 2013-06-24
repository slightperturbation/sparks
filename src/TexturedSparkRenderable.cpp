
#include "TexturedSparkRenderable.hpp"
#include "TextureManager.hpp"
#include "RenderCommand.hpp"

using namespace std;
using namespace Eigen;

// "texturedSpark"
const std::string g_defaultVertexShaderFilename = "colorVertexShader.glsl"; 
const std::string g_defaultFragmentShaderFilename = "colorFragmentShader.glsl";

spark::TexturedSparkRenderable
::TexturedSparkRenderable( LSparkPtr a_spark )
: Renderable( "TexturedSparkRenderable" ),
  m_mesh( new Mesh() ),
  m_spark( a_spark ) 
{
    m_mesh->name( "TexturedSparkMesh" );
    setViewProjection( m_spark->viewProjection() );
}

Eigen::Vector3f 
spark::TexturedSparkRenderable
::upOffset( const Eigen::Vector3f& pos,
            const Eigen::Vector3f& parentPos,
            const Eigen::Vector3f& camDir,
            float width )
{
    const Vector3f sparkDir = pos - parentPos;
    const Vector3f upDir = (camDir.cross( sparkDir )).normalized();
    return upDir * width;
}
    
void 
spark::TexturedSparkRenderable
::render( const RenderCommand& rc ) const
{
    const float width = 0.005f * m_spark->length();
    m_mesh->clearGeometry();
    Segments& segments = m_spark->segments();
    // two verts per segment, so vertexIndex = (2i, 2i+1)  (bottom, top)
    // plus two at the end to close the last segment
    m_mesh->resizeVertexArray( 2 * segments.size() );
    LOG_TRACE(g_log) << "Spark render:  assuming " << 2*segments.size()
                     << " verts for " << segments.size() << " segments.\n";
    for( size_t i=0; i < segments.size(); ++i )
    {
        // First, create the vertices, 0 to 2*(segments.size()-1)
        Segment& s = segments[i];
        LOG_TRACE(g_log) << "Rendering segment[" << i << "] (parent=" << s.m_parentIndex
            << ") verts=" << 2*i << ", " << 2*i+1 
            << "  p = " << s.m_pos(0) << ", " << s.m_pos(1) << ", " << s.m_pos(2)
            << "\n";

        if( s.m_parentIndex == -1 )
        {
            continue;
        }
        Vector3f camDir;
        if( m_camera )
        {
            glm::vec3 dir = m_camera->lookAtDirection( glm::vec3(s.m_pos[0],
                                                                 s.m_pos[1],
                                                                 s.m_pos[2]) );
            camDir = Vector3f( dir[0], dir[1], dir[2] ).normalized();
        }
        else
        {
            LOG_ERROR(g_log) << "Spark::splitSegment called without camera set.";
            camDir[0] = 0; camDir[1] = 0; camDir[2] = -1;
        }
        const Vector3f offset = upOffset( s.m_pos,
                                          s.parentPos(segments),
                                          camDir,
                                          width );
        Vector4f color( 1.0f, 1.0f, 1.0f, s.m_intensity );
        // texture coords assume a radially symmetric texture
        // (ie, circle in middle of texture)
        m_mesh->setVertex( 2*i,
                           s.m_pos - offset,
                           Vector2f( 0.5f, 0.0f ),
                           color,
                           camDir );
        m_mesh->setVertex( 2*i+1,
                           s.m_pos + offset,
                           Vector2f( 0.5f, 1.0f ),
                           color,
                           camDir );
        // add triangles between this segment and parents
        m_mesh->addTriangleByIndex( 2*i, 2*i+1, 2*s.m_parentIndex+1  );
        m_mesh->addTriangleByIndex( 2*s.m_parentIndex+1, 2*s.m_parentIndex, 2*i );

        if( segments[s.m_parentIndex].m_parentIndex == -1 )
        {
            Segment& daddy = segments[s.m_parentIndex];
            // add verts for parent too
            m_mesh->setVertex( 2*s.m_parentIndex,
                               daddy.m_pos - offset,
                               Vector2f( 0.0f, 0.0f ),
                               color,
                               camDir );
            m_mesh->setVertex( 2*s.m_parentIndex+1,
                               daddy.m_pos + offset,
                               Vector2f( 0.0f, 1.0f ),
                               color,
                               camDir );
        }
    }
    m_mesh->bindDataToBuffers();    
    // Render mesh
    m_mesh->render( rc );
}

void
spark::TexturedSparkRenderable
::attachShaderAttributes( GLuint shaderIndex )
{
    m_mesh->attachShaderAttributes( shaderIndex );
}

void 
spark::TexturedSparkRenderable
::update( float dt )
{
    m_spark->update( dt );
    m_mesh->update( dt );
}

void
spark::TexturedSparkRenderable
::setViewProjection( ConstProjectionPtr aCamera )
{
    m_camera = aCamera;
}

