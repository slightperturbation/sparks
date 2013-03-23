
#include "TexturedSparkRenderable.hpp"

using namespace std;
using namespace Eigen;

const std::string g_defaultVertexShaderFilename = DATA_PATH "/shaders/TexturedSparkVertex.glsl";
const std::string g_defaultFragmentShaderFilename = DATA_PATH "/shaders/TexturedSparkFragment.glsl";

TexturedSparkRenderable
::TexturedSparkRenderable( LSparkPtr a_spark )
: m_mesh( new Mesh( g_defaultVertexShaderFilename.c_str(), 
                    g_defaultFragmentShaderFilename.c_str() ) ), 
  m_spark( a_spark )
{

}

void 
TexturedSparkRenderable
::setTextureState( const RenderContext& renderContext )
{

}

void 
TexturedSparkRenderable
::render( const RenderContext& renderContext )
{
    // copy new spark data to mesh
    glm::vec3 glmCamPos = renderContext.cameraPos();
    const Vector3f camPos(glmCamPos[0], glmCamPos[1], glmCamPos[2] );

    const float halfAspectRatio = 0.05;

    //m_mesh->clearGeometry();
 
    const Segments& segments = m_spark->segments();
    for( size_t i=0; i<segments.size(); ++i )
    {
        const Segment& s = segments[i];
        const Vector3f len = s.m_end-s.m_begin;
        const Vector3f dir = len.normalized();
        const Vector3f midpoint = 0.5f*len + s.m_begin;
        const Vector3f camDir = (camPos - midpoint).normalized();
        const Vector3f upDir = camDir.cross( dir );
        const Vector3f upOffset = upDir * ( len.norm()*halfAspectRatio );
        m_mesh->addQuad( s.m_begin - upOffset,
                         s.m_end   - upOffset,
                         s.m_begin + upOffset,
                         s.m_end   + upOffset,
                         camDir ); // normal of the quad
    }

    
    // Setup texture
    //glUniform1i( m_sampler_location, m_textureShaderUnit );
    // 
    // Render mesh
    // (mesh knows to load its shader)
    m_mesh->render( renderContext );
}

void 
TexturedSparkRenderable
::update( float dt )
{
    m_spark->update( dt );
    //m_mesh->update( dt );
}

void
TexturedSparkRenderable
::loadTextures()
{
    glEnable( GL_TEXTURE_2D );
    glGenTextures( 1, &m_textureId );
    
    m_textureShaderUnit = 0;
    glActiveTexture( GL_TEXTURE0 + m_textureShaderUnit );
    glBindTexture( GL_TEXTURE_2D, m_textureId );
    glfwLoadTexture2D( "spark.tga", GLFW_BUILD_MIPMAPS_BIT );
}

void 
TexturedSparkRenderable
::loadShaders()
{
    /// Mesh doesn't have any, but delegate anyway
    m_mesh->loadShaders();
}
