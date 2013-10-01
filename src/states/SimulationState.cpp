
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
    enum PerformanceType { faster, balanced, highQuality, veryHighQuality };
    PerformanceType perf = balanced;
    {
        int n = 0;
        int slices = 32;
        switch( perf )
        {
        case faster:
            slices = 32;
            n = 8;
            m_fluidData.reset( new Fluid(n) );
            m_fluidData->setDiffusion( 5e-3 );//1e-2 );
            m_fluidData->setVorticity( 1e4 );//1e4 ); //1e2 );
            m_fluidData->setGravityFactor( 0, 500, 1100 ); // +x left, +y is away from viewer, 
            break;
            
        case balanced:
            slices = 128;
            n = 32; //24; 
            m_fluidData.reset( new Fluid(n) );
            m_fluidData->setDiffusion( 5e-6 ); // 5e-3 );//1e-2 );
            m_fluidData->setVorticity( 5e5 );//1e4 ); //1e2 );
            m_fluidData->setGravityFactor( 10, 800, 5000 ); //( 0, 500, 1100 ); // +x left, +y is away from viewer, 
            break;

        case highQuality:
            slices = 256;
            n = 48; // looks good, still too slow 
            m_fluidData.reset( new Fluid(n) );
            m_fluidData->setDiffusion( 5e-3 );//1e-2 );
            m_fluidData->setVorticity( 1e4 );//1e4 ); //1e2 );
            m_fluidData->setGravityFactor( 0, 500, 1100 ); // +x left, +y is away from viewer, 
            break;

        case veryHighQuality:
            slices = 512;
            n = 48;// looks great, but slow
            m_fluidData.reset( new Fluid(n) );
            m_fluidData->setDiffusion( 5e-3 );//1e-2 );
            m_fluidData->setVorticity( 1e4 );//1e4 ); //1e2 );
            m_fluidData->setGravityFactor( 0, 500, 1100 ); // +x left, +y is away from viewer, 
            break;
        }

        spark::SlicedVolumePtr sliceVolume( new
            SlicedVolume( m_facade->getTextureManager(),
            m_facade->getShaderManager(),
            "TransparentPass",
            slices, m_fluidData ) );
        //    RayCastVolumePtr rayCastFluid( new RayCastVolume( "fluid_raycastvolume",
        //                                               textureManager,
        //                                               shaderManager,
        //                                               m_fluidData ) );
        float slicesSideLength = 0.5;
        glm::mat4 xform_scale = glm::scale( glm::mat4(), 
            glm::vec3( slicesSideLength, slicesSideLength, slicesSideLength ) );

        // fixed offset hard-coded in SimulationState.lua
        glm::vec3 worldOffset_fromLua( 0, -0.1, -0.1 );

        glm::mat4 xform_move = glm::translate( glm::mat4(), 
            glm::vec3( 0, slicesSideLength/2.0f - (0.5f/n)*slicesSideLength, 0 )
            + worldOffset_fromLua
        ); // 2.5, 1 for boundary cell, 0.5 to center on grid

        glm::mat4 xform_rot = glm::rotate( glm::mat4(), 90.0f, glm::vec3( 1,0,0 ) ); // z-up to y-up

        glm::mat4 xform =  xform_move * xform_rot * xform_scale;

        //rayCastFluid->setTransform( xform );
        sliceVolume->setTransform( xform );
        m_scene->add( sliceVolume );
        m_scene->addAsyncUpdateable( sliceVolume );
        m_lua->registerObject( "theSmokeVolume", sliceVolume );
    }

    int cellCountPerSide = 510;
    // change tissue shader blur radius to match!
    switch( perf )
    {
    case faster:
        cellCountPerSide = 126;
        break;
    case balanced:
        cellCountPerSide = 254;
        break;
    case highQuality:
        cellCountPerSide = 510;
        break;
    case veryHighQuality:
        cellCountPerSide = 1022;
        break;
    }
    m_tissueMesh = TissueMeshPtr( 
        new TissueMesh( name() + "_TISSUE_SIMULATION",
                        m_facade->getTextureManager(),
                        0.5, // scale
                        cellCountPerSide //254 // level of detail 510-- good, 126 for debugging
                        )
    );
    m_scene->addAsyncUpdateable( m_tissueMesh );  // register for updates
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
    // random starting position
    //static float x = 0;//rand()/(float)RAND_MAX;
    //static float y = 0;//rand()/(float)RAND_MAX;
    //float joules;
    
    //// get current wattage from network
    //float wattage = 30000.0;
    //int numContactElems = 3;
    //
    //// Direct Heat
    //float extent = m_tissueMesh->totalLengthPerSide()/2.0f;
    //for( int i = 0; i < numContactElems; ++i )
    //{
    //    // random walk
    //    x += 0.005 * rand()/(float)RAND_MAX - 0.0025;
    //    y += 0.005 * rand()/(float)RAND_MAX - 0.0025;
    //    x = std::min( x, extent );
    //    x = std::max( x, -extent );
    //    y = std::min( y, extent );
    //    y = std::max( y, -extent );
    //    joules = dt * wattage / (float)numContactElems;
    //    m_tissueMesh->accumulateHeat( x, y, joules );
    //}
    
    // Tie tissue vaporization to the Smoke
    float deltaDensity = 10000.5;
    float maxDensity = 1;
    std::vector<glm::vec2> vapingLocations;
    m_tissueMesh->acquireVaporizingLocations( vapingLocations );
    for( auto iter = vapingLocations.begin(); iter != vapingLocations.end(); ++iter )
    {
        glm::vec2& pos = *iter;
        m_fluidData->addSourceAtLocation( pos.x, pos.y, deltaDensity, maxDensity  );
    }

    ScriptState::update( dt );
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

