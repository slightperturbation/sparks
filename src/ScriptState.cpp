
#include "ScriptState.hpp"
#include "Scene.hpp"
#include "SceneFacade.hpp"
#include "LuaInterpreter.hpp"

#include <sstream>

spark::ScriptState
::ScriptState( const spark::StateName& name,
               spark::SceneFacadePtr facade )
: SceneState( name, facade->getScene() ),
  m_facade( facade ),
  m_lua( new LuaInterpreter( facade->getFinder() ) )
{
    m_lua->setFacade( facade );
    runScriptFromFile( fileNameFromStateName( name ) );
}

spark::ScriptState
::ScriptState( const StateName& name,
               ScenePtr scene,
               OpenGLWindow* window,
               FileAssetFinderPtr finder,
               TextureManagerPtr tm,
               ShaderManagerPtr sm,
               PerspectiveProjectionPtr camera,
               FrameBufferRenderTargetPtr frameBufferTarget,
               InputPtr inputManager,
               GuiEventPublisherPtr guiEventPublisher )
: SceneState( name, scene ),
  m_lua( new LuaInterpreter( finder ) )
{
    m_facade = SceneFacadePtr( new SceneFacade( scene,
                                                window,
                                                finder,
                                                tm, sm,
                                                camera,
                                                frameBufferTarget,
                                                inputManager,
                                                guiEventPublisher )
                              );
    m_lua->setFacade( m_facade );
    runScriptFromFile( fileNameFromStateName( name ) );
}

spark::ScriptState
::~ScriptState()
{ }

void
spark::ScriptState
::runScriptFromFile( const std::string& filename )
{
    m_lua->runScriptFromFile( filename.c_str() );
}

void
spark::ScriptState
::load( void )
{
    m_lua->runScriptFromString( "theState:load()" );
    SceneState::load();
}

void
spark::ScriptState
::activate( void )
{
    m_lua->runScriptFromString( "theState:activate()" );
    SceneState::activate();
}

void
spark::ScriptState
::deactivate( void )
{
    m_lua->runScriptFromString( "theState:deactivate()" );
    SceneState::deactivate();
}

void
spark::ScriptState
::reset( void )
{
    m_lua->runScriptFromString( "theState:reset()" );
    SceneState::reset();
}

void
spark::ScriptState
::update( double dt )
{
    // poll for loading thread to complete
    // when complete set flag to advance to next state
    std::stringstream cmd;
    cmd << "theState:update( " << dt << " )";
    m_lua->runScriptFromString( cmd.str().c_str() );
    SceneState::update( dt );
}

boost::optional<spark::StateName>
spark::ScriptState
::nextState( double currTime )
{
    // advance to menu when loading complete
    std::stringstream cmd;
    cmd << "theState:nextState( " << currTime << " )";
    m_lua->runScriptFromString( cmd.str().c_str() );

    lua_State* lua = m_lua->m_lua;
    std::string next;
    try
    {
        luabind::object obj = luabind::globals( lua )["theNextState"];
        if( obj )
        {
            next = luabind::object_cast<std::string>( obj );
        }
        
        // TODO -- instead of relying on the global variable "theNextState"
        // we should use the return value of the call to theState:nextState
        // however, the following tries aren't working:
//
//        luabind::object theState = luabind::globals( lua )["theState"];
//        if( theState )
//        {
//            std::string obj = luabind::call_function<std::string>( lua,
//                                                                  "theState.nextState",
//                                                                  theState,
//                                                                  currTime );
//            std::string obj = luabind::call_function<std::string>( lua,
//                                                                  "theState:nextState",
//                                                                  currTime );
//
//        }
//        else
//        {
//            LOG_ERROR(g_log) << "ScriptState should build a global object called theState";
//        }
        
    } catch( luabind::error& err )
    {
        LOG_ERROR(g_log) << "ScriptState::nextState error";
        throw err;
    }
    if( next.empty() )
    {
        return boost::optional<spark::StateName>();
    }
    else
    {
        return boost::optional<spark::StateName>( next );
    }
}

std::string
spark::ScriptState
::fileNameFromStateName( const StateName& name )
{
    std::stringstream filename;
    filename << name << "State.lua";
    return filename.str();
}


