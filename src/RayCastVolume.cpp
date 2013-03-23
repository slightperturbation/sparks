#include "RayCastVolume.hpp"
#include <iomanip>


RayCastVolume::RayCastVolume( VolumeDataPtr data )
    : Mesh( DATA_PATH "/shaders/rayCastVertexShader.glsl",
            DATA_PATH "/shaders/rayCastFragmentShader.glsl" ),
    m_volumeData( data ),
    m_scalarDataSource( DensityScalar ),
    m_vectorDataSource( NoneVector )
{
    Mesh::unitCube();
    m_dataTextureRGBA.resize( 4 * m_volumeData->dimX() * m_volumeData->dimY() * m_volumeData->dimZ() );
}

void RayCastVolume::setupRenderState( void )
{
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    //glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA ); // assumes pre-multiplied alpha in texture
}

void RayCastVolume::setupShaderState( const RenderContext& renderContext )
{
    // Setup projection matrix
    std::string projName = "projMat";  // Depends on the name in the shader program
    GLint pm = glGetUniformLocation( m_shaderProgramIndex, projName.c_str() );
    if( pm == -1 )
    {
        std::cerr << "Renderable::render | Failed to find uniform shader attribute of name \""
            << projName << "\" in shader #" << m_shaderProgramIndex << "\n";
    } else {
        glUniformMatrix4fv( pm, 1, GL_FALSE, glm::value_ptr( renderContext.projectionMatrix() ) );
    }

    GLint mvmID = glGetUniformLocation( m_shaderProgramIndex, "modelViewMat" );
    if( mvmID == GL_INVALID_VALUE || mvmID == GL_INVALID_OPERATION )
    {
        std::cerr << "Renderable::render | Failed to find uniform shader attribute of name \"modelViewMat\" in shader #" << m_shaderProgramIndex << "\n";
    } else {
        glProgramUniformMatrix4fv( m_shaderProgramIndex, mvmID, 1, GL_FALSE, glm::value_ptr( renderContext.modelViewMatrix() ) );
    }

    // give the light position in voxel space to the vertex shader
    GLint lightPosID = glGetUniformLocation( m_shaderProgramIndex, "g_worldLightPosition" );
    if( lightPosID != GL_INVALID_VALUE && lightPosID != GL_INVALID_OPERATION )
    {
        glProgramUniform3fv( m_shaderProgramIndex, lightPosID, 1, glm::value_ptr(renderContext.lightPosition()) );
    } else {
        std::cerr << "Error, can't find uniform g_worldLightPosition\n";
    }
    GLint lightColorID = glGetUniformLocation( m_shaderProgramIndex, "g_lightColor" );
    if( lightColorID != GL_INVALID_VALUE && lightColorID != GL_INVALID_OPERATION )
    {
        glProgramUniform3fv( m_shaderProgramIndex, lightColorID, 1, glm::value_ptr(renderContext.lightColor()) );
    }
    GLint absorptionID = glGetUniformLocation( m_shaderProgramIndex, "g_absorption" );
    if( absorptionID != GL_INVALID_VALUE && absorptionID != GL_INVALID_OPERATION )
    {        
        glProgramUniform1f( m_shaderProgramIndex, absorptionID, m_volumeData->absorption() );
    }
    GLint samplesID = glGetUniformLocation( m_shaderProgramIndex, "g_numSamples" );
    if( samplesID != GL_INVALID_ENUM && samplesID != GL_INVALID_OPERATION )
    {
        glProgramUniform1i( m_shaderProgramIndex, samplesID, m_numVolumeSamples );
    }
    GLint lightSamplesID = glGetUniformLocation( m_shaderProgramIndex, "g_numLightSamples" );
    if( lightSamplesID != GL_INVALID_ENUM && lightSamplesID != GL_INVALID_OPERATION )
    {
        glProgramUniform1i( m_shaderProgramIndex, lightSamplesID, m_numLightSamples );
    }
}

void RayCastVolume::teardownRenderState( void )
{
    glUseProgram( 0 );
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

