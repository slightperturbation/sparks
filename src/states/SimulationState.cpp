
#include "states/SimulationState.hpp"

#include "Scene.hpp"
#include "ScriptState.hpp"
#include "SceneFacade.hpp"
#include "LuaInterpreter.hpp"
#include "TissueMesh.hpp"

#include <cstdlib>  // tmp for rand

spark::SimulationState
::SimulationState( const RenderableName& name, SceneFacadePtr facade )
: ScriptState( name, facade )
{
}

spark::SimulationState
::~SimulationState()
{
    
}

void
spark::SimulationState
::load( void )
{

    m_tissueMesh = TissueMeshPtr( new TissueMesh( 
        name() + "_TISSUE_SIMULATION", 
        m_facade->getTextureManager(),
        0.2, 
        126 ) );
    m_scene->add( m_tissueMesh );  // register for updates
    // Register tissue mesh with lua 
    m_lua->registerObject( "theTissueSim", m_tissueMesh );

    // Delegate to lua script
    ScriptState::load();
}

void
spark::SimulationState
::activate( void )
{
    ScriptState::activate();
}

void
spark::SimulationState
::deactivate( void )
{
    ScriptState::deactivate();
}

void
spark::SimulationState
::update( double dt )
{
    ScriptState::update( dt );
}

void
spark::SimulationState
::fixedUpdate( double dt )
{
    float x, y, joules;

    // Current wattage from UI/Lua
    float wattage = 3000.0;
    int numContactElems = 10;

    // Direct Heat
    for( int i = 0; i < numContactElems; ++i )
    {
        x = rand()/(float)RAND_MAX;
        y = rand()/(float)RAND_MAX;
        joules = dt * wattage / (float)numContactElems;
        m_tissueMesh->accumulateHeat( x, y, joules );
    }

    // Arcing heat
    // TODO

    ScriptState::fixedUpdate( dt );
}

void
spark::SimulationState
::render( void )
{
    ScriptState::render();    
}

boost::optional<spark::StateName>
spark::SimulationState
::nextState( double currTime )
{
    return ScriptState::nextState( currTime );
}

