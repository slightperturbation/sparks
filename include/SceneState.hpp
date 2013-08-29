#ifndef SPARK_SCENESTATE_HPP
#define SPARK_SCENESTATE_HPP

#include "Spark.hpp"
#include "State.hpp"

namespace spark
{
    /// Concrete State that delegates to a single Scene object.
    /// Use only for scenes that are nothing more than an unchanging Scene.
    /// Concrete implementations written in C++ should inherit from this,
    /// and concrete implementations in Lua should instance ScriptState.
    class SceneState : public State
    {
    public:
        SceneState( const StateName& name, ScenePtr scene );
        virtual ~SceneState();
        virtual void load( void )             override;
        virtual void activate( void )         override;
        virtual void deactivate( void )       override;
        virtual void update( double dt )      override;
        virtual void fixedUpdate( double dt ) override;
        virtual void render( void )           override;
        boost::optional<spark::StateName> nextState( double currTime ) override;
        
        /// Add the given Renderable to the Scene held by this SceneState.
        void add( RenderablePtr renderable );
        /// Add the given RenderPass to the Scene held by this SceneState.
        void add( RenderPassPtr renderPass );
    protected:
        ScenePtr m_scene;
    };
}
#endif
