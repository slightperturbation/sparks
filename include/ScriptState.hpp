#ifndef SPARK_SCRIPTSTATE_HPP
#define SPARK_SCRIPTSTATE_HPP

#include "Spark.hpp"
#include "SceneState.hpp"
#include "Utilities.hpp"

#include <boost/optional.hpp>

namespace spark
{
    /// State that delegates state responsibilities to a Lua script
    /// Call runScriptFromFile() to build a "theState" object
    /// instance.  Example:
    ///
    /// local ExampleState = {}
    ///
    /// function ExampleState:new()
    ///	  newObj = { angle = 45 }
    ///	  self.__index = self
    ///	  return setmetatable(newObj, self)
    /// end
    ///
    /// function ExampleState:load()
    /// end
    ///
    /// function ExampleState:activate()
    /// end
    ///
    /// function ExampleState:update( dt )
    /// end
    ///
    /// function ExampleState:deactivate()
    /// end
    /// 
    /// function ExampleState:reset()
    /// end
    ///
    /// function ExampleState:nextState( currTime )
    ///   	if currTime > 20 then
    ///       theNextState = "nextStateName"
    ///     else
    ///       theNextState = "" -- Keep current state
    ///     end
    /// end
    ///
    /// theState = ExampleState:new()
    /// theNextState = ""
    //////////
    /// \todo should hold the luabind::object and call methods on that
    /// instead of relying on the name of the state object ("theState")
    /// but luabind method calls not working yet.
    class ScriptState : public SceneState
    {
    public:
        ScriptState( const StateName& name, SceneFacadePtr facade );
        ScriptState( const StateName& name,
                    ScenePtr scene,
                    OpenGLWindow* window,
                    FileAssetFinderPtr finder,
                    TextureManagerPtr tm,
                    ShaderManagerPtr sm,
                    PerspectiveProjectionPtr camera,
                    FrameBufferRenderTargetPtr frameBufferTarget,
                    InputPtr inputManager,
                    FontManagerPtr fontManager,
                    AudioManagerPtr audioManager,
                    GuiEventPublisherPtr guiEventPublisher );
        virtual ~ScriptState();
        virtual void load( void )             override;
        virtual void activate( void )         override;
        virtual void update( double dt )      override;
        virtual void deactivate( void )       override;
        virtual void reset( void )            override;
        virtual void shutdown( void )         override;
        
        boost::optional<spark::StateName> nextState( double currTime ) override;

        /// \todo -- Implement reloading of scripts (handling of recreating objects)
        void reloadScript( void ); 

    protected:
        void runScriptFromFile( const std::string& filename );
        std::string fileNameFromStateName( const StateName& name );

        SceneFacadePtr m_facade;
        LuaInterpreterPtr m_lua;
        
        // Disallow copy ctor and assignment by private & undefined
        ScriptState( const ScriptState& );
        void operator=( const ScriptState& );
    };
    typedef spark::shared_ptr< ScriptState > ScriptStatePtr;
}

#endif
