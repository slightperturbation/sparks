
#include "TissueMesh.hpp"
#include "TextureManager.hpp"

#include <glm/glm.hpp>

spark::TissueMesh
::TissueMesh( const RenderableName& name,
              TextureManagerPtr tm,
              float totalLengthMeters,
              size_t heatDim )
: m_tempTextureName( name + "_TISSUE_TEMPERATURE_TEXTURE" ),
  m_conditionTextureName( name + "_TISSUE_CONDITION_TEXTURE" ),
  m_vaporizationDepthMapTextureName( name + "_TISSUE_VAPORIZATION_DEPTH_TEXTURE" ),
  m_textureManager( tm ),
  m_N( heatDim + 2 ),
  m_voxelDimMeters( totalLengthMeters / (float)heatDim ),
  m_diffusionIters( 100 ),
  //m_SORovershoot( 1.00001 ),
  m_dessicationThresholdTemp( 273.15 + 37.0 + 20.0 ), //63.0 ),
  m_charThresholdTemp( 273.15 + 37.0 + 250.0 )
{
    m_heatMap.resize( m_N * m_N, 0.0 );
    // body temp is 37C
    m_tempMapA.resize( m_N * m_N, 273.15 + 37.0 );
    m_tempMapB.resize( m_N * m_N, 273.15 + 37.0 );
    m_tissueCondition.resize( m_N * m_N, normalTissue );
    m_vaporizationDepthMap.resize( m_N * m_N );

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
::update( double dt )
{
    if( !m_textureManager ) return;
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

    ///////////////////////////////////////////////////////////////////////
    // Update tissue condition based on temperature
    const float vaporizationDepth = 0.002;
    size_t ind = 0;
    for( size_t y = 1; y < m_N-1; ++y )
    {
        for( size_t x = 1; x < m_N-1; ++x )
        {
            ind = index(x,y);
            float tempKelvin = (*m_currTempMap)[ind];

            // as the temp rises above 100, can start vaporizing
            // the total energy (J) needed to complete the vaporization
            // can be represented as a change in temp
            float vapeHeatAsKelvin 
                =   273.15f + 100.0f 
                  + latentHeatOfVaporization(x,y) / specificHeat(x,y);
            //  =   J/g / (J/(gK)) --> temp in kelvin
            if(    (m_tissueCondition[ind] == normalTissue)
                && (tempKelvin > vapeHeatAsKelvin) )
            {
                // Vaporized tissue, from normalTissue
                m_tissueCondition[ind] = vaporizingTissue;
                m_tissueConditionUpdateBounds.addPoint( x, y );
            }
            else if( m_tissueCondition[ind] == vaporizingTissue )
            {
                // was vaporizing previously, now remove and set as gone
                // TODO -- how long to vaporize?  Just one timestep seems
                // arbitrary, especially if visualized
                // still good to keep it discrete though for 
                // efficiency moving to the graphics card.
                // Possibly, introduce intermediate vaporizing states?
                m_tissueCondition[ind] = vaporizedTissue;
                m_tissueConditionUpdateBounds.addPoint( x, y );

                /////////////////////////////////////////////////////////
                //// TODO -------  
                //// Depth must be limited by current tool depth!
                //// HACK -------
                //// depth limited by arbitrary constant!
                //// see SimulationState.lua local passDepth in update() method
                const float maxVaporizationDepth = 0.03;

                m_vaporizationDepthMap[ind] = std::min( m_vaporizationDepthMap[ind] + vaporizationDepth,
                                                        maxVaporizationDepth );
                
                // For diffusion purposes, vaporized tissue
                // is at a much reduced temperature-- it's not there anymore
                // to diffuse heat from.
                (*m_currTempMap)[ind] -= vapeHeatAsKelvin;
                (*m_nextTempMap)[ind] -= vapeHeatAsKelvin;
            }
            else if( m_tissueCondition[ind] == vaporizedTissue )
            {
                // Depth has been altered, underlying tissue is normal
               // m_tissueCondition[ind] = normalTissue;
               // (*m_currTempMap)[ind] = 273.15f + 37.0f;
               // (*m_nextTempMap)[ind] = 273.15f + 37.0f;
            }
            
            // Dessication 
            if(    (m_tissueCondition[ind] == normalTissue)
                && (tempKelvin > m_dessicationThresholdTemp) )
            {
                m_tissueCondition[ind] = dessicatedTissue;
                m_tissueConditionUpdateBounds.addPoint( x, y );
            }
            // Charring
            if(    (m_tissueCondition[ind] == dessicatedTissue)
                && (tempKelvin > m_charThresholdTemp) )
            {
                m_tissueCondition[ind] = charredTissue;
                m_tissueConditionUpdateBounds.addPoint( x, y );
            }
        }
    } // end condition update

    // Request for condition to be pushed to graphics card
    const bool useSubsetLoading = false;
    if( useSubsetLoading )
    {
        m_textureManager->queueSubsetLoad2DByteTextureFromData( m_conditionTextureName,
            m_tissueCondition,
            m_N,
            m_tissueConditionUpdateBounds.minX,
            m_tissueConditionUpdateBounds.minY,
            m_tissueConditionUpdateBounds.maxX,
            m_tissueConditionUpdateBounds.maxY
            );
    }
    else
    {
        m_textureManager->queueLoad2DByteTextureFromData( m_conditionTextureName, 
                                                          m_tissueCondition, 
                                                          m_N );
    }
    m_textureManager->queueLoad2DFloatTextureFromData( m_vaporizationDepthMapTextureName,
        m_vaporizationDepthMap, 
        m_N );
    ///////////////////////////////////////////////////////////////////////
    // Diffuse temperature by Fourier's law of thermal conduction
    // q = -k \nabla T
    const double a = 1e-8;//1e-5;//1e-7; // diffusion rate
    const double decayRate = 1e-5f;//0.33f; // 1e-5f
    double k = dt * a / (m_diffusionIters * m_voxelDimMeters * m_voxelDimMeters );
    if( k > 0.25 ) 
    {
        LOG_WARN(g_log) << "Diffusion likely unstable";
        k = 0.25;
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
                //double delta =
                //    k * ( 4.0f* ( (*m_currTempMap)[index(x+1,y)]
                //                + (*m_currTempMap)[index(x-1,y)]
                //                + (*m_currTempMap)[index(x,y+1)]
                //                + (*m_currTempMap)[index(x,y-1)] 
                //                )
                //                // Diagonals
                //                + (*m_currTempMap)[index(x+1,y+1)]
                //                + (*m_currTempMap)[index(x+1,y-1)]
                //                + (*m_currTempMap)[index(x-1,y+1)]
                //                + (*m_currTempMap)[index(x-1,y-1)]
                //                - (20.0f* (*m_currTempMap)[ind])
                //    );
                //(*m_nextTempMap)[ind] = (*m_currTempMap)[ind]
                //    + delta 
                //        - ( decayRate * dt * (*m_currTempMap)[ind] )
                //        ;

                // Same, but replacing index calls
                double delta =
                    k * ( 4.0f* ( (*m_currTempMap)[ind+1]
                + (*m_currTempMap)[ind-1]
                + (*m_currTempMap)[ind+m_N]
                + (*m_currTempMap)[ind-m_N] 
                )
                    // Diagonals
                    + (*m_currTempMap)[ind+1+m_N]
                + (*m_currTempMap)[ind+1-m_N]
                + (*m_currTempMap)[ind-1+m_N]
                + (*m_currTempMap)[ind-1-m_N]
                - (20.0f* (*m_currTempMap)[ind])
                    );
                (*m_nextTempMap)[ind] = (*m_currTempMap)[ind]
                + delta 
                    - ( decayRate * dt * (*m_currTempMap)[ind] )
                    ;
            }
        }
    } // End diffusion

 

    // push temp data to graphics card

    //m_textureManager->queueLoad2DFloatTextureFromData( m_tempTextureName, *m_nextTempMap, m_N );
    swapTempMaps();
}

void
spark::TissueMesh
::accumulateHeat( float x, float y, float heatInJoules )
{
    size_t centerIndex = indexFromXY( x, y );
    m_heatMap[centerIndex] += heatInJoules;
    if( heatInJoules > 1e6 )
    {
        assert(false);
    }
}
void 
spark::TissueMesh
::accumulateElectricalEnergy( float posx, float posy,
                              float voltage,
                              float current,
                              float dutyCycle,
                              float radiusOfContact,
                              float dt )
{
    // potentially wasteful, but touch every voxel is an easy way to 
    // capture the full heating effect
    int ind = 0;
    glm::vec2 center( posx, posy );
    glm::vec2 cellPos;
    float cellX = 0;
    float cellY = 0;
    float distCellToContactCenter2;
    float distCellToContactCenter;
    float radiusOfContact2 = radiusOfContact*radiusOfContact;
    float areaOfContact = M_PI*radiusOfContact2;
    float currentDensity2 = current*current / areaOfContact;
    float unitsFactor = 1e-8;
    for( size_t y = 1; y < m_N-1; ++y )
    {
        for( size_t x = 1; x < m_N-1; ++x )
        {
            ind = index(x,y);
            indexToPosition( ind, &cellX, &cellY );
            cellPos.x = cellX - center.x;
            cellPos.y = cellY - center.y;
            distCellToContactCenter2 = glm::dot( cellPos, cellPos );
            distCellToContactCenter = std::sqrt( distCellToContactCenter2 );
            // are we inside the direct contact radius?
            if( distCellToContactCenter2 < radiusOfContact2 )
            {
                m_heatMap[ind] += unitsFactor * currentDensity2 * dutyCycle * dt;
            }
            else
            {
                // surface effect is dissipating with the cube of distance
                float d = distCellToContactCenter - radiusOfContact;
                float invDist3 = 1.0f / (d*d*d);
                {
                    float localCurrDensity2 = (current*current);// / (m_voxelDimMeters*m_voxelDimMeters);
                    m_heatMap[ind] += unitsFactor * localCurrDensity2 * dutyCycle * dt * invDist3;
                }
            }
        }
    }
}

const spark::TextureName&
spark::TissueMesh
::getVaporizationDepthMapTextureName( void ) const
{
    return m_vaporizationDepthMapTextureName;
}

const spark::TextureName&
spark::TissueMesh
::getTempMapTextureName( void ) const
{
    return m_tempTextureName;
}

const spark::TextureName&
spark::TissueMesh
::getConditionMapTextureName( void ) const
{
    return m_conditionTextureName;
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
    // assume 80% water
    return 0.8 * 2.257e6; // J/kg
}

void 
spark::TissueMesh
::acquireVaporizingLocations( std::vector<glm::vec2>& vaping )
{
    float x = 0; float y = 0;
    for( size_t i = 0; 
         (i < m_tissueCondition.size()) && vaping.size() < 32; // max # of cells vaping 
         ++i )
    {
        if( m_tissueCondition[i] == vaporizingTissue )
        {
            indexToPosition( i, &x, &y );
            vaping.push_back( glm::vec2( x, y ) );
        }
    }
}

size_t 
spark::TissueMesh
::indexFromUV( float u, float v ) const
{
    if( u < 0 || v < 0 )
    {
        assert( false );
        return 0;
    }
    if( u > 1 || v > 1 )
    {
        assert( false );
        return 0;
    }
    // Use negative rounding:
    // -.09 * 10 - 0.5 = -1.4 -> -1 
    // -.04 * 10 - 0.5 = -.9 -> 0
    // .04 * 10 + 0.5 = .9 -> 0
    // .09 * 10 + 0.5 = 1.4 -> 1
    size_t x = size_t( u * m_N + (u>0 ? 0.5 : -0.5) );
    size_t y = size_t( v * m_N + (v>0 ? 0.5 : -0.5) );
    x = ( x == m_N ? m_N-1 : x );
    y = ( y == m_N ? m_N-1 : y );
    return index( x, y );
}


size_t 
spark::TissueMesh
::indexFromXY( float x, float y ) const
{
    int rx = (int)((x / m_voxelDimMeters) + ((float)m_N/2.0f) - 0.5f);
    int ry = (int)((y / m_voxelDimMeters) + ((float)m_N/2.0f) - 0.5f);
    size_t ind = m_N * ry + rx;
    if( ind >= m_tissueCondition.size() )
    {
        LOG_ERROR(g_log) << "ERROR -- indexFromXY( " << x 
            << ", " << y << " ) = " << ind 
            << " which is > m_tissueCondition.size()=" 
            << m_tissueCondition.size();
        ind = 0;
    }
    if( ind >= m_heatMap.size() )
    {
        LOG_ERROR(g_log) << "ERROR -- indexFromXY( " << x 
            << ", " << y << " ) = " << ind 
            << " which is > m_heatMap.size() =" 
            << m_heatMap.size();
        ind = 0;
    }
    return ind;
}

void 
spark::TissueMesh
::indexToPosition( size_t ind, float* pX, float* pY ) const
{
    assert( ind < m_tissueCondition.size() );
    int y = ind / m_N; 
    int x = ind % m_N;
    *pX = m_voxelDimMeters * ( x - ((float)m_N/2.0f) + 0.5f );
    *pY = m_voxelDimMeters * ( y - ((float)m_N/2.0f) + 0.5f );
}
