#ifndef SPARK_STATE_HPP
#define SPARK_STATE_HPP

#include "Spark.hpp"
#include <boost/optional.hpp>

namespace spark
{
    /// Represents a State that the simulator may be in.  
    /// The current state directs the flow of updates events and rendering.
    /// The StateManager calls methods as appropriate to signal changes
    /// in the simulator's state.
    /// See spark::StateManager
    class State
    {
    public:
        State( const StateName& name );
        virtual ~State();

        /// Returns the name of this state, which should be a unique
        /// identifier among all instances held by a StateManager.
        virtual StateName name( void ) const;
        
        /// Called once when state is first entered/activated.
        virtual void load( void ) = 0;

        /// Called each time the state is entered/activated.
        virtual void activate( void ) = 0;
        
        /// Called once when leaving this state to enter another state.
        virtual void deactivate( void ) = 0;
        
        /// Allow the state to update its internal state.
        /// Called once per graphics frame.  
        virtual void update( double dt ) = 0;

        /// Called periodically with a fixed dt (fixed time-step update).
        virtual void fixedUpdate( double dt ) = 0;
        
        /// Allow the state to render itself to the current OpenGL
        /// context.  Must be called only by the main GUI thread.
        virtual void render( void ) = 0;

        /// Restore the initial conditions of this state.
        virtual void reset( void ) = 0;

        /// Returns a StateName to request switching to a different
        /// State.  The StateManager (generally) honors this as a request
        /// for the next State.
        /// If returns a StateName, deactivate() will be called soon.
        /// Called by StateManager after update() and fixedUpdate()
        virtual boost::optional<StateName> nextState( double currTime ) = 0;
    private:
        StateName m_name;
    };
}

#endif
