//
//  LSpark.cpp
//  sparks
//
//  Created by Brian Allen on 3/20/13.
//
//

#include "LSpark.hpp"
#include "Projection.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>
#include <random>

using namespace Eigen;
using namespace std;

spark::Segment
::Segment()
: m_pos( 0,0,0 ),
  m_intensity( 1.0 ),
  m_parentIndex( -1 )
{
    // Noop
}

spark::Segment
::Segment( const Vector3f& a_pos, float a_intensity )
: m_pos(a_pos),
  m_intensity(a_intensity), 
  m_parentIndex( -1 )
{
    // Noop
}

spark::LSpark
::LSpark()
: m_camera()
{
    // Noop
}

float
spark::LSpark
::unitRandom( void )
{
    return 1.0f - ( (rand()*2.0f) / RAND_MAX );
}

void
spark::LSpark
::create( const Eigen::Vector3f& a_begin,
          const Eigen::Vector3f& a_end,
          float a_intensity,
          float a_scale,
          int   a_depth,
          float a_forkProb )
{
    m_segments.clear();
    m_segments.reserve( 2 << (a_depth+1) );  // +1 for forks, actual size random
    m_segments.push_back( Segment(a_begin, a_intensity) );
    m_segments.push_back( Segment(a_end, a_intensity) );
    m_segments.back().m_parentIndex = 0;
    splitSegment( 1, a_scale, a_depth, a_forkProb );
}

void
spark::LSpark
::setViewProjection( ConstProjectionPtr aCamera )
{
    m_camera = aCamera;
}

spark::ConstProjectionPtr
spark::LSpark
::viewProjection( void ) const
{
    return m_camera;
}

void
spark::LSpark
::update( double dt )
{
    for( size_t i=0; i<m_segments.size(); ++i )
    {
        // Jitter positions around a bit
        float jitterScale = 0.0005f;
        //m_segments[i].m_pos += Vector3f( jitterScale*unitRandom(), jitterScale*unitRandom(), jitterScale*unitRandom() );
        // decrease intensity
        m_segments[i].m_intensity *= 0.95f; // TODO -- parameterize?
    }
}

void
spark::LSpark
::advect( VelocityFieldInterfacePtr velocityField )
{
    // TODO
}

float
spark::LSpark
::length( void ) const
{
    if( m_segments.size() < 2 )
    {
        throw "LSpark error-- length called with no segments";
    }
    return (m_segments[1].m_pos - m_segments[0].m_pos).norm();
}

void
spark::LSpark
::splitSegment( size_t a_index, float a_scale, int a_depth, float a_forkProb )
{
    if( a_depth <= 0 ) return;
    if( m_segments[a_index].m_parentIndex == -1 ) return;

    // Sometimes add an additional fork branch to either of the new segments
    // Don't split an end-point!
    if( (m_segments[a_index].m_parentIndex != -1)
       && ((rand()*1.0f/RAND_MAX) < a_forkProb)
       && !isTerminalSegment( a_index )
       )
    {
        forkSegment( a_index, a_scale, a_depth, a_forkProb );
    }
    
    // Parent <------------------ a_index
    //
    // Parent <---- newSegment <--- a_index
    //
    const float intensityFalloff = 0.9f;
    const Segment& s = m_segments[a_index];
    const Vector3f parentPos = s.parentPos(m_segments);
    const Vector3f dir = parentPos - s.m_pos;  // long dir of segment
    const Vector3f unitDir = dir.normalized();
    const float scale = a_scale * dir.norm();
    const Vector3f midpoint = dir*0.5 + s.m_pos;  // TODO -- add a little randomness to choosing the endpoint

    Vector3f camDir;
    if( m_camera )
    {
        glm::vec3 dir = m_camera->lookAtDirection( glm::vec3(s.m_pos[0],
                                                             s.m_pos[1],
                                                             s.m_pos[2]) );
        camDir = Vector3f( dir[0], dir[1], dir[2] );
    }
    else
    {
        LOG_ERROR(g_log) << "LSpark::splitSegment called without camera set.";
        camDir[0] = 0; camDir[1] = 0; camDir[2] = -1;
    }
    const Vector3f perp = (unitDir.cross( camDir )).normalized();
    const float rand1 = unitRandom(); const float rand2 = unitRandom();
    //const float rand1 = 1; const float rand2 = 1;
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

    //LOG_DEBUG(g_log) << "SPLIT[" << a_index << "] -> " << a_index << ":" << newSegmentIdx << "\n"
    //    << "\t [" << a_index << "].parent = " << thisSegmentRef.m_parentIndex << "\n"
    //    << "\t [" << newSegmentIdx << "].parent = " << newSegmentRef.m_parentIndex << "\n";

    // Recurse on each of the branches
    splitSegment( a_index, a_scale, a_depth - 1, a_forkProb );
    splitSegment( newSegmentIdx, a_scale, a_depth - 1, a_forkProb );
}

void
spark::LSpark
::forkSegment( size_t a_index, float a_scale, int a_depth, float a_forkProb )
{
    const float intensityFalloff = 1.0f;
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
    
    Vector3f camDir;
    if( m_camera )
    {
        glm::vec3 dir = m_camera->lookAtDirection( glm::vec3(parentPos[0],
                                                             parentPos[1],
                                                             parentPos[2]) );
        camDir = Vector3f( dir[0], dir[1], dir[2] );
    }
    else
    {
        LOG_ERROR(g_log) << "LSpark::forkSegment called without camera set.";
        camDir[0] = 0; camDir[1] = 0; camDir[2] = -1;
    }
    const Vector3f perp = (unitDir.cross( camDir )).normalized();

    Segment fork;
    fork.m_intensity = s.m_intensity * intensityFalloff;
    Vector3f slightOffset = perp * forkOffsetScale * unitRandom()
        + perp.cross( unitDir ) * forkOffsetScale * unitRandom();

    fork.m_pos = s.m_pos - forkLengthScale * dir + slightOffset;
    fork.m_parentIndex = a_index;
    m_segments.push_back( fork );
    size_t forkSegmentIdx = m_segments.size() - 1;
    splitSegment( forkSegmentIdx, a_scale, a_depth - 1, a_forkProb );
}

bool
spark::LSpark
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
