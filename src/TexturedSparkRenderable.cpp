
#include "TexturedSparkRenderable.hpp"

using namespace std;
using namespace Eigen;

const std::string g_defaultVertexShaderFilename = DATA_PATH "/shaders/TexturedSparkVertex.glsl";
const std::string g_defaultFragmentShaderFilename = DATA_PATH "/shaders/TexturedSparkFragment.glsl";

TexturedSparkRenderable
::TexturedSparkRenderable( LSparkPtr a_spark )
: m_mesh( new Mesh( g_defaultVertexShaderFilename.c_str(), 
                    g_defaultFragmentShaderFilename.c_str() ) ), 
  m_spark( a_spark ),
  m_textureShaderUnit( 0 )
{

}

void 
TexturedSparkRenderable
::setTextureState( const RenderContext& renderContext )
{
    //glUniform1i( m_sampler_location, m_textureShaderUnit );
    glBindTexture( GL_TEXTURE_2D, m_textureId ); checkOpenGLErrors();
    // TODO -- make a normal attribute
    //glUniform1i( glGetUniformLocation( m_mesh->m_shaderProgramIndex, "tex2d" ), 0 );
    // TODO -- for now, defaults ot the zero-th shader (m_textureId == 0)
    //assert( m_textureId == 0 );
}

void 
TexturedSparkRenderable
::render( const RenderContext& renderContext )
{
    // copy new spark data to mesh
    glm::vec3 glmCamPos = renderContext.cameraPos();
    const Vector3f camPos(glmCamPos[0], glmCamPos[1], glmCamPos[2] );

    const float halfAspectRatio = 0.16;

    m_mesh->clearGeometry();
 
    const Segments& segments = m_spark->segments();
    for( size_t i=0; i<segments.size(); ++i )
    {
        const Segment& s = segments[i];
        std::cerr << "Rendering segment: (" << s.m_begin << ") -> (" << s.m_end << ")\n";
        const Vector3f len = s.m_end-s.m_begin;
        const Vector3f dir = len.normalized();
        const Vector3f midpoint = 0.5f*len + s.m_begin;
        const Vector3f camDir = (camPos - midpoint).normalized();
        const Vector3f upDir = camDir.cross( dir );
        const Vector3f upOffset = upDir * ( len.norm()*halfAspectRatio );

        std::cerr << "\toffset: (" << upOffset << ")\n";
        m_mesh->addQuad( s.m_begin - upOffset,
                         s.m_end   - upOffset,
                         s.m_begin + upOffset,
                         s.m_end   + upOffset,
                         camDir ); // normal of the quad
    }
    m_mesh->bindDataToBuffers();
    
    // Setup texture
    setTextureState( renderContext );

    // Render mesh
    // (mesh knows to load its shader)
    m_mesh->render( renderContext );
}

void 
TexturedSparkRenderable
::update( float dt )
{
    m_spark->update( dt );
    m_mesh->update( dt );
}

void
TexturedSparkRenderable
::loadTextures()
{
    const std::string m_textureFilename = DATA_PATH "/data/textures/spark.tga";

    glGenTextures( 1, &m_textureId ); checkOpenGLErrors();
    
    m_textureShaderUnit = 0;
    glActiveTexture( GL_TEXTURE0 + m_textureShaderUnit ); checkOpenGLErrors();
    glBindTexture( GL_TEXTURE_2D, m_textureId ); checkOpenGLErrors();
    glfwLoadTexture2D( m_textureFilename.c_str(), GLFW_BUILD_MIPMAPS_BIT ); checkOpenGLErrors();
}



void
TexturedSparkRenderable
::loadShaders()
{
    m_mesh->loadShaders();
}
