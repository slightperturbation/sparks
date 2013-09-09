
#include "SceneState.hpp"
#include "Scene.hpp"
#include "SceneFacade.hpp"
#include "LuaInterpreter.hpp"

spark::SceneState
::SceneState( const StateName& name, ScenePtr scene )
: State( name ),
  m_scene( scene )
{
    // Noop
}

spark::SceneState
::~SceneState()
{
    // Noop
}

void
spark::SceneState
::load( void )
{
    // Noop
}

void
spark::SceneState
::activate( void )
{
    m_scene->activate();
}

void
spark::SceneState
::deactivate( void )
{
    m_scene->deactivate();
}

void
spark::SceneState
::update( double dt )
{
    m_scene->update( dt );
}

void
spark::SceneState
::fixedUpdate( double dt )
{
    m_scene->fixedUpdate( dt );
}

void
spark::SceneState
::render()
{
    m_scene->prepareRenderCommands();
    m_scene->render();
}

boost::optional<spark::StateName>
spark::SceneState
::nextState( double currTime )
{
    boost::optional<StateName> SceneState( boost::none );
    return SceneState;
}

void
spark::SceneState
::add( RenderablePtr renderable )
{
    m_scene->add( renderable );
}

void
spark::SceneState
::add( RenderPassPtr renderPass )
{
    m_scene->add( renderPass );
}

