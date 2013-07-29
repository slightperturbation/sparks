//
//  Spark.cpp
//  sparks
//
//  Created by Brian Allen on 3/18/13.
//
//

#include "DBMSpark.hpp"

#include <iterator>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>

using namespace Eigen;

spark::DBMSpark
::DBMSpark()
: m_h( 0.025f ),
  m_degree( 5 ),
  m_eta( 10 )
{
    
}

void
spark::DBMSpark
::setAggregate( const PointCharges& a_ )
{
    m_aggregate.clear();
    m_aggregate.reserve( a_.size() );
    copy( a_.begin(), a_.end(), std::back_inserter(m_aggregate) );
}

void
spark::DBMSpark
::initializeBoundary( const PointCharges& a_boundary )
{
    // Compute the electric field for each candidate
    // based on the given boundary
    for( size_t i=0; i<m_candidate.size(); ++i )
    {
        recomputeElectricFieldAtPoint( m_candidate[i] );
    }
}

void
spark::DBMSpark
::clear( void )
{
    m_aggregate.clear();
    m_candidate.clear();
}

void
spark::DBMSpark
::update( void )
{
    LOG_DEBUG(g_log) << "Begin Spark::update(), agg: "
    << m_aggregate.size() << ", can: " << m_candidate.size() << "\n";
    if( m_aggregate.empty() && m_candidate.empty() )
    {
        return;
    }
    
    if( !m_candidate.empty() )
    {
        // Select a new pointcharge from the candidates
        size_t index = selectNextCandidate();
    
        // Add this PointCharge to the Aggregate
        m_aggregate.push_back( m_candidate[index] );

        // Remove from the list of candidates, and replace with a sample from the neighborhood
        m_candidate[index] = sampleNeighborhood( m_aggregate.back(), 0, m_degree );
    }
    // Add additional samples from the neighborhood of the newly selected candidate
    for( size_t i=0; i<m_degree-1; ++i )
    {
        m_candidate.push_back( sampleNeighborhood(m_aggregate.back(), i, m_degree ) );
    }
    
    // Incorporate the new aggregate member into every candidate's
    // electric field.
    updateElectricFields( m_aggregate.back() );
    LOG_DEBUG(g_log) << "End  Spark::update(), agg: "
    << m_aggregate.size() << ", can: " << m_candidate.size() << "\n";
}

void
spark::DBMSpark
::updateElectricFields( const PointCharge& a_additionalCharge )
{
    for( size_t i=0; i<m_candidate.size(); ++i )
    {
        m_candidate[i].phi += field( m_candidate[i], a_additionalCharge );
    }
}

void
spark::DBMSpark
::recomputeElectricFieldAtPoint( PointCharge& a_point )
{
    a_point.phi << 0, 0, 0;
    for( size_t a=0; a<m_aggregate.size(); ++a )
    {
        Vector3f fieldAtA = field( a_point, m_aggregate[a] );
        a_point.phi += fieldAtA;
        float phi = fieldAtA.norm();
        m_minPhi = std::min<float>( m_minPhi, phi );
        m_maxPhi = std::max<float>( m_maxPhi, phi );
    }
    for( size_t b=0; b<m_boundary.size(); ++b )
    {
        Vector3f fieldAtA = field( a_point, m_boundary[b] );
        a_point.phi += fieldAtA;
        float phi = fieldAtA.norm();
        m_minPhi = std::min<float>( m_minPhi, phi );
        m_maxPhi = std::max<float>( m_maxPhi, phi );
    }
}

Eigen::Vector3f
spark::DBMSpark
::field( const PointCharge& to, const PointCharge& from )
{
    Vector3f r = (to.pos-from.pos);
    float d = r.norm();
    // Green's func soln to spherical shell boundary-value problem
    return ( 1.0f - ( m_h / d ) ) * r.normalized() * from.q;
}

size_t
spark::DBMSpark
::selectNextCandidate( void ) const
{
    if( m_candidate.empty() ) throw "Cannot call selectNextCandidate on empty candidate list.";

    float invRange = 1.0f / (m_maxPhi - m_minPhi);
    
    // compute total probability
    float totProb = 0.0f;
    for( size_t i=0; i<m_candidate.size(); ++i )
    {
        totProb += std::pow( (m_candidate[i].phi.norm() - m_minPhi)/invRange, m_eta );
    }
    // randomly select a number between 1.0f and 0.0f.
    float p = std::rand()*1.0f/RAND_MAX;
    
    // gobble-up the p until you reach the choosen one.
    for( size_t i=0; i<m_candidate.size(); ++i )
    {
        float prob = std::pow( (m_candidate[i].phi.norm() - m_minPhi)/invRange, m_eta ) / totProb;
        if( prob < p ) return i;
        p -= prob;
    }
    return m_candidate.size() - 1;
}

spark::PointCharge
spark::DBMSpark
::sampleNeighborhood( const PointCharge& a_sample, int a_dir, int a_totDirs )
{
    PointCharge p( a_sample );
    // add some small random displacement
    for( size_t dim=0; dim<2; ++dim )
    {
        p.pos[dim] += m_h - (2.0f*std::rand()*m_h/RAND_MAX);
    }
    recomputeElectricFieldAtPoint( p );
    return p;
}
