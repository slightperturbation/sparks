#ifndef SPARK_SCRIPTSTATE_HPP
#define SPARK_SCRIPTSTATE_HPP

#include "Spark.hpp"
#include "SceneState.hpp"

#include <boost/optional.hpp>

namespace spark
{
    /// State that delegates state responsibilites to a Lua script
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
    /// function ExampleState:activate()
    /// end
    ///
    /// function ExampleState:update( dt )
    /// end
    ///
    /// function ExampleState:fixedUpdate( dt )
    /// end
    ///
    /// function ExampleState:deactivate()
    /// end
    ///
    /// function ExampleState:nextState()
    ///   	if currTime > 20 then return "nextState" end
	///     return "" -- Keep current state
    /// end
    ///
    /// theState = ExampleState:new()
    ///
    class ScriptState : public SceneState
    {
    public:
        ScriptState( const StateName& name, SparkFacadePtr facade );
        ScriptState( const StateName& name,
                    ScenePtr scene,
                    FileAssetFinderPtr finder,
                    TextureManagerPtr tm,
                    ShaderManagerPtr sm,
                    PerspectiveProjectionPtr camera,
                    FrameBufferRenderTargetPtr frameBufferTarget,
                    GuiEventPublisherPtr guiEventPublisher );
        virtual ~ScriptState();
        void runScriptFromFile( const std::string& filename );
        virtual void activate( void )         override;
        virtual void update( double dt )      override;
        virtual void fixedUpdate( double dt ) override;
        virtual void deactivate( void )       override;
        boost::optional<spark::StateName> nextState( double currTime ) override;
    private:
        SparkFacadePtr m_facade;
        LuaInterpreterPtr m_lua;
    };
}

#endif
