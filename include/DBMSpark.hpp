//
//  Spark.h
//  sparks
//
//  Created by Brian Allen on 3/18/13.
//
//

#ifndef spark_DBMSpark_h
#define spark_DBMSpark_h

#include "Spark.hpp"

#include <vector>
#include <memory>

#include <Eigen/Dense>

namespace spark
{
    struct PointCharge
    {
        PointCharge() : q( 1.0f )
        {
            pos << 0, 0, 0;
            phi << 0, 0, 0;
        }
        PointCharge( float x, float y, float z ) : q( 1.0f )
        {
            pos << x, y, z;
            phi << 0, 0, 0;
        }
    
        Eigen::Vector3f pos;      //< Location of point charge
        Eigen::Vector3f phi;      //< Electric field vector at location pos
        float q;                  //< Sigened electric charge
    };
    typedef std::vector<PointCharge> PointCharges;

    float dist( const PointCharge& a, const PointCharge& b );


    /// Dielectric breakdown model
    class DBMSpark
    {
    public:
        DBMSpark();
        void setAggregate( const PointCharges& a_ );
        void initializeBoundary( const PointCharges& a_boundary );
    
        void clear( void );
        void update( double dt );
        PointCharges& aggregate( void ) { return m_aggregate; }
        const PointCharges& aggregate( void ) const { return m_aggregate; }
        PointCharges& candidate( void ) { return m_candidate; }
        const PointCharges& candidate( void ) const { return m_candidate; }
    private:
        /// Compute new phi (electric field) values at each m_candidate PointCharge
        /// by adding a_additionalCharge.
        /// Post: m_minPhi and m_maxPhi updated.
        void updateElectricFields( const PointCharge& a_additionalCharge );
    
        /// Compute the electric field of a_point by adding field from every
        /// current memeber of the aggregate and the boundary.
        void recomputeElectricFieldAtPoint( PointCharge& a_point );
    
        /// Compute the electric field at the location of "to" due to the charge of "from"
        Eigen::Vector3f field( const PointCharge& to, const PointCharge& from );
    
        /// Use m_candidate electric field strengths (phi) to compute the probability
        /// that a candidate will be selected for inclusion in the aggregate
        /// Returns the index in m_candidate of the selected PointCharge.
        /// Precondition: m_candidate must have at least one element.
        size_t selectNextCandidate( void ) const;
    
        /// Choose a new point in the neighborhoood (with m_h units)
        /// of the a_center position.  Returned PointCharge has the
        /// same electric field as a_center.
        PointCharge sampleNeighborhood( const PointCharge& a_center, int dir, int totalDirs );
    
        /// Typical distance between a point charge and its neighbor
        float m_h;
    
        /// Number of additional samples to add when a candidate is
        /// brought into the aggregate.
        size_t m_degree;

        float m_eta; //< "fractal" power, good between 1 and 10

        /// Point charges composing the actual spark, note phi values not updated after adding
        PointCharges m_aggregate;
        /// All candidate point charges that might be included in the aggregate in next step
        PointCharges m_candidate;
        /// All point charges making up the "boundary"
        PointCharges m_boundary;
    
        float m_minPhi; //< min electric field of candidate point charges
        float m_maxPhi; //< max electric field of candidate point charges
    };
    typedef spark::shared_ptr< DBMSpark > DBMSparkPtr;

    /// Setup the DBM-style scenario with a central negative point aggregate at 0,0,0,
    /// surrounded by a ball of positive charges at radius.
    DBMSparkPtr buildSpark_pointInBall( float radius );
} // end namespace spark
#endif
