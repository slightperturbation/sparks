#ifndef SPARK_TISSUEMESH_HPP
#define SPARK_TISSUEMESH_HPP

#include "Spark.hpp"

#include "Renderable.hpp"
#include "Mesh.hpp"

namespace spark
{
    /// 2D model of tissue temperature
    class TissueMesh : public Renderable
    {
        // Possible states/conditions the tissue can be in 
        // (not a enum class due to VS2010)
        enum TissueCondition { normalTissue, 
                               dessicatedTissue, 
                               vaporizingTissue, 
                               charredTissue };
    public:
        TissueMesh( const RenderableName& name,
                    TextureManagerPtr tm,
                    float totalLengthMeters,
                    size_t heatDim );
        virtual ~TissueMesh();
        
        /// Renderable
        virtual void render( const RenderCommand& rc ) const override;
        
        /// Update the VBO based on changes to the data.
        virtual void update( float dt );
        
        /// Add a source of heat for this time-step
        /// Heat sources are cleared/zero'd each update
        /// x,y are in object coordinates, [0,1]^2
        void accumulateHeat( float x, float y, float heatInJoules );

        /// Returns the handle/name of the texture holding the temperature 
        /// map.  Note that 37.0C is typical body temperature and should
        /// be considered the baseline temperature.
        const TextureName& getTempMapTextureName( void ) const;

        /// Returns the real-world units (meters) length of one side of
        /// the tissue sample, not including the boundary elements.
        float totalLengthPerSide( void ) const;
    protected:
        virtual void attachShaderAttributes( GLuint shaderIndex ) { }

        /// Returns the specific heat of tissue at location (may be temp dependent)
        /// specific_heat -- energy(J) required to change one unit
        ///                  of mass (kg) by one degree
        /// See: "In vitro measurements of temperature-dependent specific
        /// heat of liver tissue," Haemmerich 2006
        /// 3411 J kg−1 K−1 at 25 C
        /// 3550 J kg−1 K−1 at 65 C
        /// 4187 J kg−1 K−1 at 83.5 C
        /// Units: J/(kg K)
        float specificHeat( size_t x, size_t y ) const;

        /// Density of tissue at location
        /// Units: kg/m^3
        float density( size_t x, size_t y ) const;

        /// Mass of tissue at location
        /// Mass loss begins at 70C
        /// Units: kg
        float mass( size_t x, size_t y ) const;

        /// Volume of one voxel element in cubic meters
        /// Units: m^3
        float volumePerElement( size_t x, size_t y ) const;

        /// Joules/gram required to vaporize the tissue at x,y
        /// Unites: J/kg
        float latentHeatOfVaporization( size_t x, size_t y ) const;
    private:
        void swapTempMaps( void )
        {
            std::vector<float>* tmp = m_currTempMap;
            m_currTempMap = m_nextTempMap;
            m_nextTempMap = tmp;
        }
        /// Provides the index into m_heatMap for x,y
        size_t index( size_t x, size_t y ) const
        {
            return x + m_N*(y);
        }
        
        /// Transform from uv space (0,1) to element index
        /// note the "edges" are boundaries and not part of the map
        /// E.g.: for TissueMesh( dim=10 ), one dimension would be laid out as:
        /// 
        /// -1/N 0 1/N 2/N 3/N ... N-1/N 1 N+1/N
        ///   0  1  2   3   4        10 11   12   INDEX <-- returned index
        ///  -.1 0  .1  .2  .3       .9  1  1.1   U parameter <-- input u
        ///  
        size_t indexFromUV( float u, float v ) const
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
        
        TextureName m_textureName;
        TextureManagerPtr m_textureManager;
        /// number of elements per dim, including two boundary elements
        /// at 0 and m_N-1
        size_t m_N;
        /// Length of the edge of a single voxel
        float m_voxelDimMeters;
        /// 2D map of surface temperatures
        std::vector< float > m_tempMapA;
        std::vector< float > m_tempMapB;
        std::vector< float >* m_currTempMap;
        std::vector< float >* m_nextTempMap; 
        /// 2D map of energy added to surface at current timestep
        /// zero'd after update()
        std::vector< double > m_heatMap;
        /// Discrete tissue state map, used by shader
        /// integer component gives the condition of the tissue (see TissueCondition)
        /// fractional component gives how long 
        std::vector< unsigned char > m_tissueCondition;

        /// Iteration count for SOR diffusion calcs
        size_t m_diffusionIters;
        /// Overshoot factor for SOR, must be 1.0 or more and less than 2.0
        /// 1.0 equivalent to Guass-Seidel; 2.0 is unstable!
        double m_SORovershoot;
    };
    typedef spark::shared_ptr< TissueMesh > TissueMeshPtr;
}
#endif
