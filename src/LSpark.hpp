
//
//  LSpark.hpp
//  sparks
//
//  Created by Brian Allen on 3/20/13.
//
//

#ifndef sparks_LSpark_hpp
#define sparks_LSpark_hpp


#include <Eigen/Dense>
#include <vector>
#include <memory>

class Segment
{
public:
    Segment();
    Segment( const Eigen::Vector3f& a_begin, const Eigen::Vector3f& a_end, float a_intensity )
        : m_begin(a_begin), m_end(a_end), m_intensity(a_intensity)
    { }
    Eigen::Vector3f m_begin;
    Eigen::Vector3f m_end;
    float m_intensity;
};
typedef std::vector< Segment > Segments;

class LSpark
{
public:
    LSpark();

    void create( const Eigen::Vector3f& a_begin,
                 const Eigen::Vector3f& a_end,
                 float a_intensity,
                 int a_depth );

    // From Spark
    const Segments& segments( void ) const { return m_segments; }
    void update( float dt );
private:
    /// Returns a float in [-1, 1]
    static float unitRandom( void );
    /// Split the segment at a_index, replacing m_segmetn[a_index]
    /// with a new leg, and adding one or more additional segments to
    /// m_segments.
    void splitSegment( size_t a_index, float a_scale, int a_depth );

    Segments m_segments;
};
typedef std::shared_ptr< LSpark > LSparkPtr;

#endif
