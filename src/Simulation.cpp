#include "Simulation.hpp"

#include <vector>


void
spark::Simulation
::add( TaskGeneratorPtr arg )
{
    m_taskGenerators.push_back( arg );
}

void
spark::Simulation
::executeTasks( float t, float dt )
{
    std::vector< TaskPtr > tasks;
    for( auto g = m_taskGenerators.begin(); g != m_taskGenerators.end(); ++g )
    {
        tasks.push_back( (*g)->createTask( t, dt ) );
    }
    // TODO Send tasks to thread pools
    for( auto i = tasks.begin(); i != tasks.end(); ++i )
    {
        (*i)->execute();
    }
}
