
#include "states/SimulationState.hpp"

#include "Scene.hpp"
#include "ScriptState.hpp"
#include "SceneFacade.hpp"
#include "LuaInterpreter.hpp"
#include "TissueMesh.hpp"

#include "SlicedVolume.hpp"
//#include "RayCastVolume.hpp"
#include "Fluid.hpp"

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

    {
        //m_fluidData.reset( new Fluid(22) );
        int n = 48;
        m_fluidData.reset( new Fluid(n) );
        m_fluidData->setDiffusion( 0.01 );
        m_fluidData->setVorticity( 1e2 ); //1e4 );
        m_fluidData->setGravityFactor( 0, 1000, -3000 );
        //m_fluidData->setGravityFactor( 0, -4000, 0.0 );
        
        spark::shared_ptr< spark::SlicedVolume > slices( new
            SlicedVolume( m_facade->getTextureManager(),
            m_facade->getShaderManager(),
            "TransparentPass",
            1024, m_fluidData ) );
        //    RayCastVolumePtr rayCastFluid( new RayCastVolume( "fluid_raycastvolume",
        //                                               textureManager,
        //                                               shaderManager,
        //                                               m_fluidData ) );
        float sideLength = 1.0;
        glm::mat4 xform_scale = glm::scale( glm::mat4(), 
            glm::vec3( sideLength, sideLength, sideLength ) );
        glm::mat4 xform_move = glm::translate( glm::mat4(), 
            glm::vec3( 0, (2.0/n)*sideLength - sideLength/2, 0 ) );
        glm::mat4 xform_rot = glm::rotate( glm::mat4(), 90.0f, glm::vec3( 1,0,0 ) ); // z-up to y-up
        glm::mat4 xform =  xform_move * xform_rot * xform_scale;// 

        //rayCastFluid->setTransform( xform );
        slices->setTransform( xform );
        m_scene->add( slices );
        //m_lua->registerObject( "theSmoke", m_fluidData );
    }


    m_tissueMesh = TissueMeshPtr( new TissueMesh( 
        name() + "_TISSUE_SIMULATION", 
        m_facade->getTextureManager(),
        0.5, 
        126 //254 // level of detail 510-- good, 126 for debugging
        ) );
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
    // random starting position
    static float x = 0;//rand()/(float)RAND_MAX;
    static float y = 0;//rand()/(float)RAND_MAX;
    float joules;

    // Current wattage from UI/Lua
    float wattage = 100000.0;
    int numContactElems = 3;
    
    // Direct Heat
    float extent = m_tissueMesh->totalLengthPerSide()/2.0f;
    for( int i = 0; i < numContactElems; ++i )
    {
        // random walk
        x += 0.02 * rand()/(float)RAND_MAX - 0.01;
        y += 0.02 * rand()/(float)RAND_MAX - 0.01;
        x = std::min( x, extent );
        x = std::max( x, -extent );
        y = std::min( y, extent );
        y = std::max( y, -extent );
        joules = dt * wattage / (float)numContactElems;
        m_tissueMesh->accumulateHeat( x, y, joules );
    }

    // Smoke

    std::vector<glm::vec2> vapingLocations;
    m_tissueMesh->acquireVaporizingLocations( vapingLocations );
    for( auto iter = vapingLocations.begin(); iter != vapingLocations.end(); ++iter )
    {
        glm::vec2& pos = *iter;
        m_fluidData->addSourceAtLocation( pos.x, pos.y );
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

