#ifndef SPARK_TISSUEMESH_HPP
#define SPARK_TISSUEMESH_HPP

#include "Spark.hpp"

#include "Updateable.hpp"
#include "Renderable.hpp"
#include "Mesh.hpp"

#include <limits>

namespace spark
{
    /// 2D model of tissue temperature that provides methods for adding
    /// heat (accumulateHeat()) and a texture to communicate the results
    /// for rendering.
    /// TODO - Rename, it's not a mesh.  TissueTexture?  TissueTempModel?
    class TissueMesh
    : public Updateable
    {
        // Possible states/conditions the tissue can be in 
        // (not a enum class due to VS2010)
        enum TissueCondition { normalTissue, 
                               dessicatedTissue, 
                               vaporizingTissue, 
                               vaporizedTissue,
                               charredTissue };
    public:
        TissueMesh( const RenderableName& name,
                    TextureManagerPtr tm,
                    float totalLengthMeters,
                    size_t heatDim );
        virtual ~TissueMesh();
                
        /// Update the condition map based on accumulated heat.
        virtual void update( double dt ) override;
        
        /// Add a source of heat for this time-step
        /// Heat sources are cleared/zero'd each update
        /// x,y are in "world" scale, with center of tissue at 0,0
        /// This is the heat energy imparted to the voxel over the 
        /// update() timestep.
        void accumulateHeat( float x, float y, float heatInJoules );

        /// Calculate the joule heating effect for electrical energy
        /// applied at the location x,y.
        /// radiusOfContact assumes a circle of contact centered at x,y
        /// Elements inside the radius are given energy directly,
        /// elements outside receive dissipating energy.
        void accumulateElectricalEnergy( float x, float y,
                                         float voltage,
                                         float current,
                                         float dutyCycle,
                                         float radiusOfContact,
                                         float dt );

        /// Returns the handle/name of the texture holding the temperature 
        /// map.  Note that 310.15 Kelvin is typical body temperature and should
        /// be considered the baseline temperature.
        const TextureName& getTempMapTextureName( void ) const;
        
        /// Returns the handle/name of the map holding the height of the
        /// tissue at the given position.
        const TextureName& getVaporizationDepthMapTextureName( void ) const;
        
        /// Returns the handle/name of the texture holding the discrete
        /// condition of the tissue at each location.  Possible values are:
        ///
        /// normalTissue=0, dessicatedTissue=1, vaporizingTissue=2,
        /// and charredTissue=3.
        ///
        /// Vaporizing is set when the tissue is actively undergoing vaporization.
        /// Once vaporization is complete (sufficient energy to vaporize an
        /// equal mass of water has been injected) the tissue becomes charred.
        const TextureName& getConditionMapTextureName( void ) const;

        /// Adds positions of each cell currently vaporizing.
        /// Positions in "world" scale with center of tissue at 0,0
        void acquireVaporizingLocations( std::vector<glm::vec2>& vaping );

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
        struct BoundingBox
        {
            int minX;
            int minY;
            int maxX;
            int maxY;
            BoundingBox() { reset(); }
            void reset() 
            { 
                minX = minY = std::numeric_limits<int>::max();
                maxX = maxY = 0; }
            /// Extend the region to encompass the given point.
            void addPoint( int x, int y )
            {
                minX = std::min( x, minX ); 
                minY = std::min( y, minY ); 
                maxX = std::max( x, maxX ); 
                maxY = std::max( y, maxY ); 
            }
        };

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
        size_t indexFromUV( float u, float v ) const;

        /// Provides index for cell at "world" coordinates x,y
        /// Assumes tissue is centered at 0,0
        /// Inverse operation of indexToPosition()
        size_t indexFromXY( float x, float y ) const;

        /// Provides "world" x,y coordinates for cell at index ind.
        /// Assumes center (m_N/2)+0.5 is located at the origin
        void indexToPosition( size_t ind, float* pX, float* pY ) const;
        
        TextureName m_tempTextureName;
        TextureName m_conditionTextureName;
        TextureName m_vaporizationDepthMapTextureName;
        TextureManagerPtr m_textureManager;
        /// number of elements per dim, including two boundary elements
        /// at 0 and m_N-1
        size_t m_N;
        /// Length of the edge of a single voxel
        float m_voxelDimMeters;
        /// 2D map of surface temperatures
        /// All temperatures in Kelvin
        /// Temperatures are effectively just measures of heat content,
        /// phase transitions are managed in update() method.
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
        BoundingBox m_tissueConditionUpdateBounds;
        /// Holds the depth of tissue removed due to vaporization effects.
        std::vector< float > m_vaporizationDepthMap;
        
        /// Iteration count for SOR diffusion calcs
        size_t m_diffusionIters;
        /// Overshoot factor for SOR, must be 1.0 or more and less than 2.0
        /// 1.0 equivalent to Guass-Seidel; 2.0 is unstable!
        //double m_SORovershoot;

        double m_dessicationThresholdTemp;
        double m_charThresholdTemp;
    };
    typedef spark::shared_ptr< TissueMesh > TissueMeshPtr;
}
#endif
