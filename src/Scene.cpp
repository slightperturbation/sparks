
#include "Scene.hpp"
#include "Updateable.hpp"
#include "Mesh.hpp"

#include <boost/chrono.hpp>

#include <functional>
#include <algorithm>

spark::Scene
::Scene( void )
{ }

spark::Scene
::~Scene( )
{
    //    // wait for the threads to complete
    //    for( auto iter = m_updateThreads.begin();
    //         iter != m_updateThreads.end();
    //         ++iter )
    //    {
    //        (*iter)->join();
    //    }
    for( auto iter = m_updateTasks.begin(); iter != m_updateTasks.end(); ++iter )
    {
        (*iter)->stop();
    }
}

void
spark::Scene
::render( void )
{
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "==== Scene::render with "
                         << m_commands.size() << " commands, "
                         << m_passes.size() << " passes and " 
                         << m_renderables.size() << " renderables.";
    }
    for( auto p = m_passes.begin(); p != m_passes.end(); ++p )
    {
        LOG_TRACE(g_log) << "\tPASS: " << *p;
    }
    for( auto r = m_renderables.begin(); r != m_renderables.end(); ++r )
    {
        LOG_TRACE(g_log) << "\tRENDERABLE: " << *r;
    }
    RenderCommandQueue commandsCopy = m_commands;
    while( !commandsCopy.empty() )
    {
        const RenderCommand& rc = commandsCopy.top();
        if( g_log->isTrace() )
        {
            LOG_TRACE(g_log) << "\tCOMMAND: Renderable=\""
                             << rc.m_renderable->name() << "\", Pass=\""
                             << rc.m_pass->name() << "\"[\"" 
                             << rc.m_pass->targetName() << "\"]"
                             << " Material=\""
                             << rc.m_material->name() << "\"";
        }
        commandsCopy.pop();
    }

    RenderCommand prevRenderCommand, rc;
    // Render each render command in order.
    int counter = 0;
    
    // Allow passes and their targets to clear and setup buffers
    ConstRenderPassPtr prevRenderPass;
    for( auto pass = m_passes.begin(); pass != m_passes.end(); ++pass )
    {
        ConstRenderPassPtr cp = *pass;
        cp->startFrame( prevRenderPass );
        prevRenderPass = cp;
    }
    // Render all accumulated passes
    prevRenderPass.reset();
    while( !m_commands.empty() )
    {
        RenderCommand rc = m_commands.top();
        LOG_TRACE(g_log) << "----Executing RenderCommand "
                         << counter++ << ": " << rc;
        ConstRenderPassPtr currRenderPass = rc.m_pass;
        if( currRenderPass != prevRenderPass )
        {
            // New render pass, allow both old and new to change GL state.
            if( prevRenderPass ) 
            {
                prevRenderPass->postRender( currRenderPass );
            }
            if( currRenderPass ) 
            {
                currRenderPass->preRender( prevRenderPass );
            }
        }
        // Pass previous to avoid re-setting current state when possible
        rc( prevRenderCommand );
        prevRenderPass = currRenderPass;
        prevRenderCommand = rc;
        m_commands.pop();
    }
    if( prevRenderPass )
    {
        prevRenderPass->postRender( ConstRenderPassPtr(nullptr) );
    }
}

void
spark::Scene
::update( double dt )
{
    for( auto i = m_updateables.begin(); i != m_updateables.end(); ++i )
    {
        (*i)->update( dt );
    }
}

void
spark::Scene
::activate( void )
{
    // resume updates
    for( auto iter = m_updateTasks.begin(); iter != m_updateTasks.end(); ++iter )
    {
        (*iter)->resume();
    }
}

void
spark::Scene
::deactivate( void )
{
    // stop updates
    for( auto iter = m_updateTasks.begin(); iter != m_updateTasks.end(); ++iter )
    {
        (*iter)->pause();
    }
}

void
spark::Scene
::addUpdateable( UpdateablePtr up )
{
    if( up )
    {
        if( std::find( m_updateables.begin(), m_updateables.end(), up ) != m_updateables.end() )
        {
            LOG_WARN(g_log) << "Attempt to add Updateable to Scene multiple times.";
        }
        m_updateables.push_back( up );
    }
    else
    {
        LOG_ERROR(g_log) << "Scene::add(UpdateablePtr) called with a nullptr.";
    }
}

void
spark::Scene
::addAsyncUpdateable( UpdateablePtr up )
{
    float dt = 1.0f/30.0f;
    if( up )
    {
        if( std::find( m_updateables.begin(),
                       m_updateables.end(), up )
            != m_updateables.end() )
        {
            LOG_ERROR(g_log) << "Probably don't want an updateable to also be async'ly updated.";
            assert( false );
        }
        m_updateTasks.push_back( FixedUpdateTaskPtr( new FixedUpdateTask( up, dt ) ) );
        m_updateTasks.back()->start();
    }
}

void 
spark::Scene
::add( RenderPassPtr rp ) 
{ 
    if( rp )
    {
        m_passes.push_back( rp ); 
    }
    else
    {
        LOG_ERROR(g_log) << "Scene::add(RenderPassPtr) called with a nullptr.";
    }
}

void 
spark::Scene
::add( RenderablePtr r ) 
{
    if( r )
    {
        if( std::find(m_renderables.begin(), m_renderables.end(), r) != m_renderables.end() )
        {
            LOG_ERROR(g_log) << "Attempt to add renderable \"" <<
                r->name() << "\" to Scene multiple times.";
        }
        m_renderables.push_back( r );
    }
    else
    {
        LOG_ERROR(g_log) << "Scene::add(RenderablePtr) called with a nullptr.";
    }
}

spark::RenderPassPtr 
spark::Scene
::getRenderPass( const RenderPassName& name ) const
{
    for( auto iter = m_passes.begin(); iter != m_passes.end(); ++iter )
    {
        RenderPassPtr p = *iter;
        if( p->name() == name )
        {
            return p;
        }
    }
    return RenderPassPtr();
}


void
spark::Scene
::prepareRenderCommands( void )
{
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "Scene::prepareRenderCommands with " 
            << m_passes.size() << " passes and " 
            << m_renderables.size() << " renderables.";
    }
    m_passes.sort( renderPassCompareByPriority );

    RenderCommand rc;
    for( auto rp = m_passes.begin(); rp != m_passes.end(); ++rp )
    {
        for( auto r = m_renderables.begin(); r != m_renderables.end(); ++r )
        {
            if( createRenderCommand( rc, *rp, *r ) )
            {
                m_commands.push( rc );
            }
        }
    }
}

void 
spark::Scene
::logPasses( void ) const
{
    LOG_INFO(g_log) << "Passes:";
    for( auto piter = m_passes.begin(); piter != m_passes.end(); ++piter )
    {
        ConstRenderPassPtr p = *piter;
        LOG_INFO(g_log) << "\t" << p->priority() << ": " << p;
    }
}

void
spark::Scene
::logRenderables( void ) const 
{
    LOG_INFO(g_log) << "Renderables:";
    for( auto r = m_renderables.begin(); r != m_renderables.end(); ++r )
    {
        LOG_TRACE(g_log) << "\t" << *r;
    }
}

void 
spark::Scene
::reset( void )
{
    LOG_INFO(g_log) << "Resetting Scene";
    deactivate();
    m_updateTasks.clear();
    m_updateables.clear();
    m_passes.clear();
    m_renderables.clear();
}

spark::Scene::FixedUpdateTask
::FixedUpdateTask( UpdateablePtr udp, float dt )
  : m_updateable( udp ),
    m_hasStarted( false ),
    m_isPaused( false ),
    m_isStopped( false ),
    m_dt( dt )
{
    m_prevUpdateTime = getTime();
}

void
spark::Scene::FixedUpdateTask
::executeTask( void )
{
    while( true )
    {
        double currTime = getTime();
        double elapsedTimeSoFar = currTime - m_prevUpdateTime;
        if( !m_isPaused && (elapsedTimeSoFar > m_dt) )
        {
            m_updateable->update( m_dt );
            m_prevUpdateTime = currTime;
        }
        else
        {
            // Wait for roughly how long until the next update is needed.
            const int waitTimeMilliseconds = int( 1000.0 * (m_dt - (getTime() - m_prevUpdateTime)) );
            if( waitTimeMilliseconds > 3 )
            {
                boost::this_thread::sleep_for( boost::chrono::milliseconds( waitTimeMilliseconds ) );
            }
        }
        if( m_isStopped )
        {
            break;
        }
    }
    // Never returns, expect on break for m_isStopped
}

void
spark::Scene::FixedUpdateTask
::pause( void )
{
    m_isPaused = true;
}

void
spark::Scene::FixedUpdateTask
::resume( void )
{
    m_isPaused = false;
}

void
spark::Scene::FixedUpdateTask
::start( void )
{
    if( !m_hasStarted )
    {
        m_hasStarted = true;
        m_thread = boost::thread( &FixedUpdateTask::executeTask, this );
    }
}

void
spark::Scene::FixedUpdateTask
::stop( void )
{
    m_isStopped = true;
    m_thread.timed_join( boost::posix_time::seconds( 1 ) );
}
