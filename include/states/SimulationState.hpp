#ifndef SPARK_SIMULATIONSTATE_HPP
#define SPARK_SIMULATIONSTATE_HPP

#include "Spark.hpp"

#include "ScriptState.hpp"

namespace spark
{
    /// Concrete state handling the simulation.
    /// Augments the lua SimulationState script.
    class SimulationState : public ScriptState
    {
    public:
        SimulationState( const StateName& name,
                         SceneFacadePtr facade );
        virtual ~SimulationState();
        
        virtual void load( void )             override;
        virtual void activate( void )         override;
        virtual void deactivate( void )       override;
        virtual void update( double dt )      override;
        virtual void fixedUpdate( double dt ) override;
        virtual void render( void )           override;
        
        boost::optional<spark::StateName> nextState( double currTime ) override;
    private:
        // Owned Tissue simulation mesh
        TissueMeshPtr m_tissueMesh;

        // Disallow copy ctor and assignment
        SimulationState( const SimulationState& ); // undefined to disable
        void operator=( const SimulationState& ); // undefined to disable
    };
}

#endif