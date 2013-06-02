#include "RayCastVolume.hpp"
#include <iomanip>


spark::RayCastVolume
::RayCastVolume( const RenderableName& aName,
                 TextureManagerPtr tm,
                 ShaderManagerPtr sm,
                 VolumeDataPtr data )
: Renderable( aName ),
  m_mesh( new Mesh() ),
  m_volumeData( data ),
  m_textureManager( tm ),
  m_textureName( "RayCastTexture3D" )
{
    //    : Mesh( SHADER_DIR "rayCastVertexShader.glsl",
    //       SHADER_DIR "rayCastFragmentShader.glsl" ),

    m_mesh->unitCube();
    ShaderInstancePtr shader = sm->createShaderInstance( "rayCastVolumeShader" );
    m_material = MaterialPtr( new Material( tm, shader ) );
    tm->load3DTextureFromVolumeData( m_textureName, m_volumeData );
    m_material->addTexture( m_textureName, "s_density3d" );
    
    m_material->setShaderUniform( "u_numSamples", 96 );
    m_material->setShaderUniform( "u_numLightSamples", 72 );
    m_material->setShaderUniform( "u_absorption", 0.25f );
    m_material->setShaderUniform( "u_lightColor",
                                  glm::vec3( 0.7f, 0.3f, 0.3f ) );
    m_material->setShaderUniform( "u_lightPosition_world",
                                  glm::vec3( 0.0f, 0.5f, 0.5f ) );
    setMaterialForPassName( g_transparencyRenderPassName, m_material );
}

void
spark::RayCastVolume
::render( void ) const
{
    m_mesh->render();
}

void
spark::RayCastVolume
::update( float dt )
{
    /// Push new density data up to graphics card
    m_textureManager->load3DTextureFromVolumeData( m_textureName, m_volumeData );
    m_mesh->update( dt );
}

void
spark::RayCastVolume
::attachShaderAttributes( GLuint shaderIndex )
{
    m_mesh->attachShaderAttributes( shaderIndex );
}

//    glEnable( GL_DEPTH_TEST );
//    glEnable( GL_BLEND );
//    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    //glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    // assumes pre-multiplied alpha in texture

