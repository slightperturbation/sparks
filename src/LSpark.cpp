//
//  LSpark.cpp
//  sparks
//
//  Created by Brian Allen on 3/20/13.
//
//

#include "LSpark.hpp"

#include <iostream>
#include <cstdlib>
#include <random>

using namespace Eigen;
using namespace std;

Segment
::Segment()
: m_pos( 0,0,0 ),
  m_intensity( 1.0 ),
  m_parentIndex( -1 )
{
    // Noop
}

Segment
::Segment( const Vector3f& a_pos, float a_intensity )
: m_pos(a_pos),
  m_intensity(a_intensity), 
  m_parentIndex( -1 )
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
    return 1.0f - ( (rand()*2.0f) / RAND_MAX );
}

void
LSpark
::create( const Eigen::Vector3f& a_begin,
    const Eigen::Vector3f& a_end,
    float a_intensity,
    int a_depth )
{
    float scale = 0.1; // 10% of total length of spark
    m_segments.clear();
    m_segments.reserve( 2 << (a_depth+1) );  // +1 for forks, actual size random
    m_segments.push_back( Segment(a_begin, a_intensity) );
    m_segments.push_back( Segment(a_end, a_intensity) );
    m_segments.back().m_parentIndex = 0;
    splitSegment( 1, scale, a_depth );
}

void
LSpark
::update( float dt )
{
    for( size_t i=0; i<m_segments.size(); ++i )
    {
        // Jitter positions around a bit
        float jitterScale = 0.1f;
        m_segments[i].m_pos += Vector3f( jitterScale*unitRandom(), jitterScale*unitRandom(), jitterScale*unitRandom() );
        // decrease intensity
        m_segments[i].m_intensity *= 0.9f; // TODO -- parameterize? 
    }
}

void
LSpark
::advect( VelocityFieldInterfacePtr velocityField )
{
    
}

void
LSpark
::splitSegment( size_t a_index, float a_scale, int a_depth )
{
    if( a_depth <= 0 ) return;
    if( m_segments[a_index].m_parentIndex == -1 ) return;

    // Sometimes add an additional fork branch to either of the new segments
    // Don't split an end-point!
    const float probFork = 1.0;
    if( (m_segments[a_index].m_parentIndex != -1)
       && ((rand()*1.0f/RAND_MAX) < probFork)
       && !isTerminalSegment( a_index )
       )
    {
        forkSegment( a_index, a_scale, a_depth );
    }
    
    // Parent <------------------ a_index
    //
    // Parent <---- newSegment <--- a_index
    //
    const float intensityFalloff = 0.9f;
    const Segment& s = m_segments[a_index];
    const Vector3f parentPos = s.parentPos(m_segments);
    const Vector3f dir = parentPos - s.m_pos;
    const Vector3f unitDir = dir.normalized();
    const float scale = a_scale * dir.norm();
    const Vector3f midpoint = dir*0.5 + s.m_pos;  // TODO -- add a little randomness to choosing the endpoint

    //$$$$$$$$$$$$$$$$$$$$$$$$$$$$TODO -- pass the camera position to here
    Vector3f anyNonParallelDir = dir; 
    anyNonParallelDir[0] += 1; anyNonParallelDir[1] += 2; anyNonParallelDir[2] += 3;
    const Vector3f perp = (unitDir.cross( anyNonParallelDir )).normalized();
    const float rand1 = unitRandom(); const float rand2 = unitRandom();
    const Vector3f offset =   perp                * scale * rand1
                            + perp.cross(unitDir) * scale * rand2;

    // add a new segment from oldBegin to midpoint
    Segment newSegment;
    newSegment.m_pos = midpoint + offset;
    newSegment.m_intensity = s.m_intensity * intensityFalloff;
    newSegment.m_parentIndex = s.m_parentIndex;

    m_segments.push_back( newSegment );
    const size_t newSegmentIdx = m_segments.size() - 1;
    m_segments[a_index].m_parentIndex = newSegmentIdx;

    //const Segment& thisSegmentRef = m_segments[a_index];
    //const Segment& newSegmentRef = m_segments[newSegmentIdx];

    //LOG_INFO(g_log) << "SPLIT[" << a_index << "] -> " << a_index << ":" << newSegmentIdx << "\n"
    //    << "\t [" << a_index << "].parent = " << thisSegmentRef.m_parentIndex << "\n"
    //    << "\t [" << newSegmentIdx << "].parent = " << newSegmentRef.m_parentIndex << "\n";

    // Recurse on each of the branches
    splitSegment( a_index, a_scale, a_depth - 1 );
    splitSegment( newSegmentIdx, a_scale, a_depth - 1 );
}

void
LSpark
::forkSegment( size_t a_index, float a_scale, int a_depth )
{
    const float intensityFalloff = 0.666f;
    if( a_depth <= 0 )
    {
        return;
    }
    
    // Parent <------------------ a_index <--------- next
    //
    // Parent <------------------ a_index <--------- next
    //                                    \
    //                                     --------- newSegment
    //                                     
    assert( m_segments.size() > a_index );
    const Segment& s = m_segments[a_index];
    const Vector3f parentPos = s.parentPos(m_segments);
    const Vector3f dir = parentPos - s.m_pos;
    const Vector3f unitDir = dir.normalized();
    const float scale = a_scale * dir.norm();

    const float forkOffsetScale = 0.5f * scale;
    const float forkLengthScale = 1.0f;// * (rand()*1.0f/RAND_MAX);
    //$$$$$$$$$$$$$$$$$$$$$$$$$$$$TODO -- pass the camera position to here
    Vector3f anyNonParallelDir = dir; 
    anyNonParallelDir[0] += 1; anyNonParallelDir[1] += 2; anyNonParallelDir[2] += 3;
    const Vector3f perp = (unitDir.cross( anyNonParallelDir )).normalized();

    Segment fork;
    fork.m_intensity = s.m_intensity * intensityFalloff;
    Vector3f slightOffset = perp * forkOffsetScale * unitRandom()
        + perp.cross( unitDir ) * forkOffsetScale * unitRandom();

    fork.m_pos = s.m_pos - forkLengthScale * dir + slightOffset;
    fork.m_parentIndex = a_index;
    m_segments.push_back( fork );
    size_t forkSegmentIdx = m_segments.size() - 1;
    splitSegment( forkSegmentIdx, a_scale, a_depth - 1 );
}

bool
LSpark
::isTerminalSegment( size_t a_index ) const
{
    for( size_t i = 0; i < m_segments.size(); ++i )
    {
        if( m_segments[i].m_parentIndex == a_index )
        {
            return false;
        }
    }
    return true;
}


