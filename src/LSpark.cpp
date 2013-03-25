//
//  LSpark.cpp
//  sparks
//
//  Created by Brian Allen on 3/20/13.
//
//

#include "LSpark.hpp"

#include <cstdlib>
#include <random>

using namespace Eigen;
using namespace std;

Segment
::Segment()
: m_begin( 0,0,0 ),
    m_end( 0,0,0 ),
    m_intensity( 1.0 )
{
    // Noop
}

LSpark
::LSpark()
{

}

float
LSpark
::unitRandom( void )
{
    return 1.0f - ( rand()*2.0f / RAND_MAX );
}

void
LSpark
::create( const Eigen::Vector3f& a_begin,
    const Eigen::Vector3f& a_end,
    float a_intensity,
    int a_depth )
{
    float scale = 1.0;
    m_segments.clear();
    m_segments.reserve( 2 << (a_depth+1) );
    m_segments.push_back( Segment(a_begin, a_end, a_intensity) );
    splitSegment( 0, scale, a_depth );
}

void
LSpark
::update( float dt )
{
    for( size_t i=0; i<m_segments.size(); ++i )
    {
        // Jitter positions around a bit
        float jitterScale = 0.1f;
        m_segments[i].m_begin += Vector3f( jitterScale*unitRandom(), jitterScale*unitRandom(), jitterScale*unitRandom() );
        // decrease intensity
        m_segments[i].m_intensity *= 0.9f; // TODO -- parameterize? 
    }
}

void
LSpark
::splitSegment( size_t a_index, float a_scale, int a_depth )
{
    if( a_depth <= 0 )
    {
        return;
    }
    Segment& s = m_segments[a_index];
    Vector3f dir = s.m_end - s.m_begin;
    Vector3f midpoint = dir*0.5 + s.m_begin;

    // TODO -- pass the camera position to here
    Vector3f anyNonParallelDir = dir; 
    anyNonParallelDir[0] += 1; anyNonParallelDir[1] += 2; anyNonParallelDir[2] += 3;
    Vector3f perp = (dir.cross( anyNonParallelDir )).normalized();
    Vector3f offset =   perp            * (a_scale/a_depth) * unitRandom()
                      + perp.cross(dir) * (a_scale/a_depth) * unitRandom();
    Vector3f oldEnd = s.m_end;
    s.m_end = midpoint + offset;

    // add a new segment from s.m_end to oldEnd
    Segment newSegment;
    newSegment.m_begin = s.m_end;
    newSegment.m_end = oldEnd;
    newSegment.m_intensity = s.m_intensity;
    m_segments.push_back( newSegment );

    // TODO -- FORKING!!!
}

