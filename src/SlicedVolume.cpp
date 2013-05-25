#include "SlicedVolume.hpp"

using namespace Eigen;

spark::SlicedVolume
::SlicedVolume( TextureManagerPtr tm, ShaderManagerPtr sm,
               unsigned int sliceCount, VolumeDataPtr data )
: Renderable( "SlicedVolume" ),
  m_mesh( new Mesh() ),
  m_volumeData( data ),
  m_dataTextureId( 0 ),
  m_textureManager( tm ),
  m_textureName( "SlicedVolumeTexture3D" )
{
    name( "SlicedVolume" );
    ShaderInstancePtr densityShader = sm->createShaderInstance( "densityShader" );
    MaterialPtr densityMaterial( new Material( tm, densityShader ) );
    tm->load3DTextureFromVolumeData( m_textureName, m_volumeData );
    densityMaterial->addTexture( m_textureName, "s_density3d" );
    setMaterialForPassName( g_colorRenderPassName, densityMaterial );
    LOG_DEBUG(g_log) << "Textures loaded.\n";

    const size_t zSteps = sliceCount;
    const float zMin = -0.5;
    const float zMax =  0.5;
    for( size_t step = 0; step < zSteps; ++step )
    {
        float z = zMin + ((float)step) * (zMax - zMin)/((float)zSteps);
        float t = ((float)step)/(zSteps-1.0); // 0 to 1
        //LOG_DEBUG(g_log) << "slice at z = " << z << "\n";
        
        MeshVertex v;
        v.m_position[0] = -0.5;     v.m_position[1] =  0.5;     v.m_position[2] = z;
        v.m_normal[0]   = 0;        v.m_normal[1] = 0;          v.m_normal[2] = 1;
        v.m_diffuseColor[0] = 0.75;  v.m_diffuseColor[1] = 0.75;
        v.m_diffuseColor[2] = 0.75; v.m_diffuseColor[3] = 0.75;
        v.m_texCoord[0] = 0;        v.m_texCoord[1] = 1;        v.m_texCoord[2] = t;
        const size_t a = m_mesh->addVertex( v );
        
        v.m_position[0] =  0.5;     v.m_position[1] =  0.5;     v.m_position[2] = z;
        v.m_texCoord[0] = 1;        v.m_texCoord[1] = 1;        v.m_texCoord[2] = t;
        const size_t b = m_mesh->addVertex( v );
        
        v.m_position[0] =  0.5;     v.m_position[1] = -0.5;     v.m_position[2] = z;
        v.m_texCoord[0] = 1;        v.m_texCoord[1] = 0;        v.m_texCoord[2] = t;
        const size_t c = m_mesh->addVertex( v );
        
        v.m_position[0] = -0.5;     v.m_position[1] = -0.5;     v.m_position[2] = z;
        v.m_texCoord[0] = 0;        v.m_texCoord[1] = 0;        v.m_texCoord[2] = t;
        const size_t d = m_mesh->addVertex( v );
        
        // TRIANGLES (NOT STRIP)
        //m_mesh->addTriangleByIndex( upperRight, lowerRight, upperLeft );
        //m_mesh->addTriangleByIndex( lowerRight, upperLeft, upperRight );
        m_mesh->addTriangleByIndex( a, b, c );
        m_mesh->addTriangleByIndex( c, d, a );
    }
    m_mesh->bindDataToBuffers();
    LOG_DEBUG(g_log) << "Slices created.\n";
}

void
spark::SlicedVolume
::render( void ) const 
{
    m_mesh->render();
}

void
spark::SlicedVolume
::update( float dt ) 
{
    m_volumeData->update( dt );
    /// Push new density data up to graphics card
    m_textureManager->load3DTextureFromVolumeData( m_textureName, m_volumeData );
    m_mesh->update( dt );
}

void
spark::SlicedVolume
::attachShaderAttributes( GLuint shaderIndex )
{
    m_mesh->attachShaderAttributes( shaderIndex );
}

