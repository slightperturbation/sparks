
//
//  LSpark.hpp
//  sparks
//
//  Created by Brian Allen on 3/20/13.
//
//

#ifndef sparks_LSpark_hpp
#define sparks_LSpark_hpp

#include "SoftTestDeclarations.hpp"
#include "VelocityFieldInterface.hpp"

#include <Eigen/Dense>
#include <vector>
#include <memory>

namespace spark
{
    class Segment;
    typedef std::vector< Segment > Segments;
    class Segment
    {
    public:
        Segment();
        Segment( const Eigen::Vector3f& a_pos, float a_intensity );
        const Eigen::Vector3f& parentPos( const Segments& segments ) const 
        { 
            return segments[m_parentIndex].m_pos; 
        }

        Eigen::Vector3f m_pos;
        float m_intensity;
        size_t m_parentIndex;

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };


    /// Lindenmayer-system (L-System) spark, recursively built by random 
    /// splitting and forking.
    class LSpark
    {
    public:
        LSpark();

        /// Creates a spark connecting begin to end.  
        /// a_depth determines the recursive depth of creation.
        /// a_intensity is the visual intensity (brightness & color).
        /// Not thread-safe.
        void create( const Eigen::Vector3f& a_begin,
                     const Eigen::Vector3f& a_end,
                     float a_intensity,
                     int a_depth );

        // From Spark
        const Segments& segments( void ) const { return m_segments; }
        Segments& segments( void ) { return m_segments; }
        void update( float dt );
        /// Move mid-points according to velocity field
        void advect( VelocityFieldInterfacePtr velocityField );
    private:
        /// Returns a float in [-1, 1]
        static float unitRandom( void );
        /// Split the segment at a_index, replacing m_segmetn[a_index]
        /// with a new leg, and adding one or more additional segments to
        /// m_segments.
        void splitSegment( size_t a_index, float a_scale, int a_depth );
        // Forking -- based on a continuation of the earlier segment 
        // (note how this polarizes the spark-- begin is now different for end)
        void forkSegment( size_t a_index, float a_scale, int a_depth );
    
        /// Returns true iff a_index is a segment that is not the
        /// parent of any other segments.
        bool isTerminalSegment( size_t a_index ) const;

        Segments m_segments;
    };
    typedef std::shared_ptr< LSpark > LSparkPtr;
}
#endif
