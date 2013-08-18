#ifndef SPARK_SCENESTATE_HPP
#define SPARK_SCENESTATE_HPP

#include "Spark.hpp"
#include "State.hpp"

namespace spark
{
    /// Concrete State that delegates to a single Scene object.
    /// Not practically useful
    class SceneState : public State
    {
    public:
        SceneState( const StateName& name, ScenePtr scene );
        virtual ~SceneState();
        virtual void activate( void )         override;
        virtual void deactivate( void )       override;
        virtual void update( double dt )      override;
        virtual void fixedUpdate( double dt ) override;
        virtual void render()      override;
        boost::optional<spark::StateName> nextState( double currTime ) override;
    protected:
        ScenePtr m_scene;
    };
    

}

#endif

