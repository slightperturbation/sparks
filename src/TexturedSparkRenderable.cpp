
#include "TexturedSparkRenderable.hpp"

using namespace std;
using namespace Eigen;

const std::string g_defaultVertexShaderFilename = SHADER_DIR "TexturedSparkVertex.glsl";
const std::string g_defaultFragmentShaderFilename = SHADER_DIR "TexturedSparkFragment.glsl";

TexturedSparkRenderable
::TexturedSparkRenderable( LSparkPtr a_spark )
: m_mesh( new Mesh(g_defaultVertexShaderFilename.c_str(), g_defaultFragmentShaderFilename.c_str()) ), 
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
    glm::vec3 c = renderContext.cameraTarget() - renderContext.cameraPos();
    const Eigen::Vector3f camDir( camDir[0], camDir[1], camDir[2] );
    const Segments& segments = m_spark->segments();
    for( size_t i=0; i<segments.size(); ++i )
    {
        const Segment& s = segments[i];
        m_mesh->addQuad( s.m_begin )
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
