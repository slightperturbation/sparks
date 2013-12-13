#include "StateManager.hpp"

spark::StateManager
::StateManager( void )
{
    // Noop
}

spark::StateManager
::~StateManager()
{
    // Noop
}

void
spark::StateManager
::addState( StatePtr state )
{
    if( state->name().empty() )
    {
        LOG_ERROR(g_log) << "Attempt to add a state with an empty name to the StateManager.";
        return;
    }
    if( m_states.end() != m_states.find( state->name() ) )
    {
        LOG_ERROR(g_log) << "Attempt to add a state with a name (\""
        << state->name() << "\") already held by the StateManager.";
        return;
    }
    LOG_DEBUG(g_log) << "Adding state \"" << state->name()
    << "\" to StateManager.";
    m_states[ state->name() ] = state;
}

void
spark::StateManager
::removeStateByName( const StateName& name )
{
    auto iter = m_states.find( name );
    if( iter == m_states.end() )
    {
        LOG_ERROR(g_log) << "Attempt to removeStateByName( \""
        << name << "\" ) but no such state is known.";
        return;
    }
    if( name == m_currState->name() )
    {
        m_pendingRemoval =  boost::optional<StateName>( name );
    }
    else
    {
        m_states.erase( iter );
    }
}

bool
spark::StateManager
::hasStateByName( const StateName& name ) const
{
    return( m_states.find( name ) != m_states.end() );
}

void
spark::StateManager
::setCurrState( const StateName& nextState )
{
    if( !hasStateByName(nextState) )
    {
        // Use a static flag to avoid flooding with error output.
        static StateName lastErrorStateName = "";
        if( lastErrorStateName != nextState )
        {
            lastErrorStateName = nextState;
            std::stringstream msg;
            msg << "Attempt to setCurrState(\"" << nextState
                << "\") but no such state is managed by this StateManager. "
                << "(Remember to remove the \"State.lua\" suffix from file names, "
                << "e.g, use \"Xxx\" for the state defined in XxxState.lua)";
            // Note -- serious enough error that warrants dumping to stderr
            std::cerr << msg.str() << std::endl;
            LOG_ERROR(g_log)  << msg.str();
        }
        return;
    }
    if( m_currState )
    {
        m_currState->deactivate();
    }
    if( m_pendingRemoval && m_pendingRemoval.get() == m_currState->name() )
    {
        removeStateByName( m_pendingRemoval.get() );
    }
    m_currState = m_states[nextState];
    
    // Load if needed
    if( ! isLoaded(m_currState->name()) )
    {
        m_currState->load();
        m_isLoaded[m_currState->name()] = true;
    }
    
    m_currState->activate();
    LOG_INFO(g_log) << "Switched to State \"" << m_currState->name() << "\".";
}

spark::StatePtr
spark::StateManager
::currState( void )
{
    return m_currState;
}

spark::StateName
spark::StateManager
::currStateName( void ) const
{
    if( m_currState )
    {
        return m_currState->name();
    }
    else
    {
        LOG_ERROR(g_log) << "currStateName() requested, but null current state";
        return "";
    }
}

spark::StatePtr
spark::StateManager
::operator[]( const StateName& name )
{
    return m_states[name];
}

void
spark::StateManager
::update( double dt )
{
    if( m_currState )
    {
        m_currState->update( dt );
    }
    else
    {
        LOG_ERROR(g_log) << "StateManager::update() requested, but null current state";
    }
}

void
spark::StateManager
::render( void )
{
    if( m_currState )
    {
        m_currState->render( );
    }
    else
    {
        LOG_ERROR(g_log) << "StateManager::render() requested, but null current state";
    }
}

void
spark::StateManager
::updateState( double currTime )
{
    if( m_currState )
    {
        boost::optional<StateName> nextState = m_currState->nextState( currTime );
        if( nextState )
        {
            setCurrState( nextState.get() );
        }
    }
}

void
spark::StateManager
::shutdown( void )
{
    if( m_currState )
    {
        m_currState->deactivate();
        m_currState->shutdown();
    }
    if( m_pendingRemoval && m_pendingRemoval.get() == m_currState->name() )
    {
        removeStateByName( m_pendingRemoval.get() );
    }
    m_currState.reset(); // No current state after shutdown
}

void 
    spark::StateManager
    ::reset( void )
{
    if( m_currState )
    {
        m_currState->reset();
    }
}


bool
spark::StateManager
::isLoaded( const StateName& name )
{
    auto iter = m_isLoaded.find( name );
    return ( iter == m_isLoaded.end() ) ? false : (*iter).second;
}


