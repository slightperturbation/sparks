#ifndef SPARK_SIMULATION_HPP
#define SPARK_SIMULATION_HPP

#include "SoftTestDeclarations.hpp"
#include "Task.hpp"

namespace spark
{
    /// Interface for classes that generate tasks for threading.
    class TaskGenerator
    {
    public:
        virtual ~TaskGenerator() {}
        /// Create a new task to calculate the update at time t,
        /// which is dt since last update.
        virtual TaskPtr createTask( float t, float dt ) = 0;
    };
    typedef spark::shared_ptr< TaskGenerator > TaskGeneratorPtr;
    
    /// Responsible for updating simulation objects using parallel-dispatched tasks.
    class Simulation
    {
        typedef std::vector< Task > SimulationTasks;
        typedef std::vector< TaskGeneratorPtr > TaskGenerators;
    public:
        /// Add an object that creates tasks for execution.
        void add( TaskGeneratorPtr arg );
        void executeTasks( float t, float dt );
    private:
        TaskGenerators m_taskGenerators;
    };
}

#endif
