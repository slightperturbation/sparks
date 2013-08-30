
#include "TissueMesh.hpp"
#include "TextureManager.hpp"

spark::TissueMesh
::TissueMesh( const RenderableName& name,
              TextureManagerPtr tm,
              float totalLengthMeters,
              size_t heatDim )
: Renderable( name ),
  m_textureName( name + "_TISSUE_TEMPERATURE_TEXTURE" ),
  m_textureManager( tm ),
  m_N( heatDim + 2 ),
  m_voxelDimMeters( totalLengthMeters / (float)heatDim ),
  m_diffusionIters( 20 ),
  m_SORovershoot( 1.00001 )
{
    m_heatMap.resize( m_N * m_N, 0.0 );
    // body temp is 37C
    m_tempMapA.resize( m_N * m_N, 37.0 );
    m_tempMapB.resize( m_N * m_N, 37.0 );
    m_tissueCondition.resize( m_N * m_N, normalTissue );
    // Arbitrarily assign temp maps to current and next
    m_currTempMap = &m_tempMapA;
    m_nextTempMap = &m_tempMapB;
}

spark::TissueMesh
::~TissueMesh()
{
    // Noop
}

void
spark::TissueMesh
::render( const RenderCommand& rc ) const
{
    // Noop
}

void
spark::TissueMesh
::update( float dt )
{
    // Apply accumulated heat to change tissue temp
    // Q = c m dT
    // dT = Q/(cm)
    for( size_t y = 1; y < m_N-1; ++y )
    {
        for( size_t x = 1; x < m_N-1; ++x )
        {
            const size_t i = index( x, y );
            const double c = specificHeat( x, y );    // 3500 J/(kg K) for liver, approx
            const double m = mass( x, y );
            (*m_currTempMap)[i] += dt * m_heatMap[i] / ( c * m );
            // zero heat map
            m_heatMap[i] = 0.0;
        }
    }
    // Diffuse temperature by Fourier's law of thermal conduction
    // q = -k \nabla T
    const float a = 1e-5; // diffusion rate
    const float decayRate = 1e-4f;
    size_t ind = 0;
    float k = dt * a / (m_diffusionIters * m_voxelDimMeters * m_voxelDimMeters );
    if( k > 0.25 ) 
    {
        LOG_WARN(g_log) << "Diffusion likely unstable";
    }
    for( size_t iter = 0; iter < m_diffusionIters; ++iter )
    {
        for( size_t y = 1; y < m_N-1; ++y )
        {
            for( size_t x = 1; x < m_N-1; ++x )
            {
                ind = index(x,y);

                // See: Finite Difference Methods for Differential Equations, LeVeque

                //////////////////
                // SOR w/5-pt Laplacian stencil
                //(*m_nextTempMap)[ind] = (1.0 - m_SORovershoot) * (*m_currTempMap)[ind]
                //+ ( a * m_SORovershoot / (4.0f) )
                //* ( (*m_currTempMap)[index(x+1,y)]
                //   + (*m_nextTempMap)[index(x-1,y)]
                //   + (*m_currTempMap)[index(x,y+1)]
                //   + (*m_nextTempMap)[index(x,y-1)]
                //   );

                //////////////////
                // Explicit euler w/5-pt Laplacian stencil
                //float delta = 
                //    ( dt * a / (m_diffusionIters * m_voxelDimMeters * m_voxelDimMeters) )
                //    * (  (*m_currTempMap)[index(x+1,y)]
                //       + (*m_nextTempMap)[index(x-1,y)]
                //       + (*m_currTempMap)[index(x,y+1)]
                //       + (*m_nextTempMap)[index(x,y-1)]
                //       - (4.0f*(*m_currTempMap)[index(x,y)])
                //      );
                //(*m_nextTempMap)[ind] = (*m_currTempMap)[ind]
                //    + delta 
                //    - ( decayRate * dt * (*m_currTempMap)[ind] )
                //   ;

                //////////////////
                // Explicit euler w/9-pt Laplacian stencil
                // Slowest, but fewest artifacts and explicit timestep
                float delta = 
                    k * ( 4.0f* ( (*m_currTempMap)[index(x+1,y)]
                                + (*m_currTempMap)[index(x-1,y)]
                                + (*m_currTempMap)[index(x,y+1)]
                                + (*m_currTempMap)[index(x,y-1)] 
                                )
                                // Diagonals
                                + (*m_currTempMap)[index(x+1,y+1)]
                                + (*m_currTempMap)[index(x+1,y-1)]
                                + (*m_currTempMap)[index(x-1,y+1)]
                                + (*m_currTempMap)[index(x-1,y-1)]
                                - (20.0f* (*m_currTempMap)[index(x,y)])
                    );
                (*m_nextTempMap)[ind] = (*m_currTempMap)[ind]
                    + delta 
                        - ( decayRate * dt * (*m_currTempMap)[ind] )
                        ;
            }
        }
    } // End diffusion

    // Update tissue condition based on temperature
    for( size_t y = 1; y < m_N-1; ++y )
    {
        for( size_t x = 1; x < m_N-1; ++x )
        {
            ind = index(x,y);
            float temp = (*m_nextTempMap)[ind];
            if( temp > 45 )
            {
                m_tissueCondition[ind] = dessicatedTissue;
            }
            if( (temp >= 100) && (m_tissueCondition[ind] != charredTissue) )
            {
                m_tissueCondition[ind] = vaporizingTissue;
            }
            // as the temp rises above 100, it's vaporizing
            // the total energy (J) needed to complete the vaporization
            // can be represented as a change in temp:
            float vapeHeatAsKelvin 
                = latentHeatOfVaporization(x,y) / specificHeat(x,y);
            if( temp > (100 + vapeHeatAsKelvin) )
            {
                temp = 100;
                m_tissueCondition[ind] = charredTissue;
            }
        }
    } // end condition update

    // push temp data to graphics card
    //m_textureManager->load2DByteTextureFromData( m_textureName, *m_nextTempMap, m_N );
    m_textureManager->load2DByteTextureFromData( m_textureName, m_tissueCondition, m_N );
    swapTempMaps();
}

void
spark::TissueMesh
::accumulateHeat( float x, float y, float heatInJoules )
{
    size_t centerIndex = indexFromUV( x, y );
    m_heatMap[centerIndex] += heatInJoules;
}

const spark::TextureName&
spark::TissueMesh
::getTempMapTextureName( void ) const
{
    return m_textureName;
}

float 
spark::TissueMesh
::totalLengthPerSide( void ) const
{
    return (m_N-2) * m_voxelDimMeters;
}

float 
spark::TissueMesh
::specificHeat( size_t  x, size_t  y ) const
{
    return 3500.0f;
}

float 
spark::TissueMesh
::density( size_t  x, size_t  y ) const
{
    //1060 kg/m3
    return 1060.0f;
}

float 
spark::TissueMesh
::mass( size_t  x, size_t  y ) const
{
    return volumePerElement(x,y) * density(x,y);
}

float 
spark::TissueMesh
::volumePerElement( size_t x, size_t y ) const
{
    return m_voxelDimMeters*m_voxelDimMeters*m_voxelDimMeters;
}

float 
spark::TissueMesh
::latentHeatOfVaporization( size_t x, size_t y ) const
{
    // For water: 
    // http://www.engineeringtoolbox.com/fluids-evaporation-latent-heat-d_147.html
    return 2.257e6;
}
