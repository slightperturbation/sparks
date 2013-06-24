
#include "SlicedVolume.hpp"
#include "TextureManager.hpp"
#include "Material.hpp"

using namespace Eigen;

spark::SlicedVolume
::SlicedVolume( TextureManagerPtr tm, ShaderManagerPtr sm,
               const RenderPassName& renderPassName,
               unsigned int sliceCount, VolumeDataPtr data )
: Renderable( "SlicedVolume" ),
  m_mesh( new Mesh() ),
  m_volumeData( data ),
  m_textureManager( tm ),
  m_textureName( "SlicedVolumeTexture3D" ),
  m_cameraDir( 0,0,1 )
{
    ShaderInstancePtr densityShader = sm->createShaderInstance( "densityShader" );
    MaterialPtr densityMaterial( new Material( tm, densityShader ) );
    tm->load3DTextureFromVolumeData( m_textureName, m_volumeData );
    densityMaterial->addTexture( "s_density3d", m_textureName );
    setMaterialForPassName( renderPassName, densityMaterial );
    
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
}

void
spark::SlicedVolume
::render( const RenderCommand& rc ) const
{
    m_mesh->render( rc );
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

void
spark::SlicedVolume
::setCameraDirection( const glm::vec3& dir )
{
    // previously we've rotated our model to point to the old cameraDir
    // so invert that rotation, then re-align to new direction
    float angle = std::acos( glm::dot( m_cameraDir, glm::vec3(0,0,1) ) );
    const float epsilon = 1e-10f;
    if( std::fabs( angle ) > epsilon )
    {
        glm::vec3 axis = glm::cross( m_cameraDir, glm::vec3(0,0,1 ) ) ;
        axis = glm::normalize( axis );
        m_mesh->rotate( glm::degrees(-angle), axis );
    }
    // now align to new direction
    m_mesh->alignZAxisWithVector( dir );
    m_cameraDir = glm::normalize( dir );
}


