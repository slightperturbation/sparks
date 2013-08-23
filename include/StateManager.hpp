#ifndef SPARKS_STATEMANAGER_HPP
#define SPARKS_STATEMANAGER_HPP

#include "Spark.hpp"
#include "State.hpp"
#include "Time.hpp"

#include <boost/optional.hpp>
#include <map>

namespace spark
{
    ///
    class StateManager
    {
    public:
        StateManager( void );
        virtual ~StateManager();
        
        /// Allows state to accessed by this StateManager.  Must be called
        /// before setting State to current with setCurrState().
        void addState( StatePtr state );
        
        /// Removes the state with name. If name is the current State,
        /// the state will be removed after currState.deactivate() finishes.
        void removeStateByName( const StateName& name );
        
        /// Returns true if a state with this name is currently managed.
        bool hasStateByName( const StateName& name ) const;

        /// Force the current State to switch to nextState.
        /// If nextState is different from the current state,
        /// deactivate() will be called on the current state,
        /// then activate() on nextState.
        void setCurrState( const StateName& nextState );
        
        /// Returns the current state.
        StatePtr currState( void );
        
        /// Returns the name of the current State.
        StateName currStateName( void ) const;
        
        /// Returns the state associated with the given name.
        StatePtr operator[]( const StateName& name );
        
        /// Allow the state to update its internal state.
        /// Called once per graphics frame.
        virtual void update( double dt );
        
        /// Called periodically with a fixed dt (fixed time-step update).
        virtual void fixedUpdate( double dt );
        
        /// Allow the state to render itself to the current OpenGL
        /// context.  Must be called only by the main GUI thread.
        virtual void render( void );
        
        /// Ask the current state if we should change states.
        void updateState( double currTime );
    private:
        /// Returns true iff the state of name has had load() called already.
        bool isLoaded( const StateName& name );
        
        StatePtr m_currState;
        std::map< StateName, StatePtr > m_states;
        std::map< StateName, bool > m_isLoaded;
        /// If true, holds a StateName of a state to removed when possible.
        /// Used when removeStateByName() is called on the current state,
        /// so the state is removed after deactivate().
        boost::optional<StateName> m_pendingRemoval;
    };
}


#endif
