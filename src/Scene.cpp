
#include "Scene.hpp"
#include "Mesh.hpp"

#include <functional>
#include <algorithm>

spark::Scene
::Scene( void )
{ }

spark::Scene
::~Scene( )
{ }

void
spark::Scene
::render( void )
{
    LOG_TRACE(g_log) << "==== Scene::render with "
                     << m_commands.size() << " commands, "
                     << m_passes.size() << " passes and " 
                     << m_renderables.size() << " renderables.";
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
        LOG_TRACE(g_log) << "\tCOMMAND: Renderable=\""
            << rc.m_renderable->name() << "\", Pass=\""
            << rc.m_pass->name() << "\"[\"" 
            << rc.m_pass->targetName() << "\"]"
            << " Material=\""
            << rc.m_material->name() << "\"";
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
                prevRenderPass->postRender( );
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
        prevRenderPass->postRender();
    }
}

void
spark::Scene
::update( float dt )
{
    for( auto i = m_renderables.begin(); i != m_renderables.end(); ++i )
    {
        (*i)->update( dt );
    }
}

void 
spark::Scene
::add( RenderPassPtr rp ) 
{ 
    m_passes.push_back( rp ); 
}

void 
spark::Scene
::add( RenderablePtr r ) 
{ 
    m_renderables.push_back( r ); 
}

spark::RenderPassPtr 
spark::Scene
::getPass( const RenderPassName& name ) const
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
    LOG_TRACE(g_log) << "Scene::prepareRenderCommands with " 
        << m_passes.size() << " passes and " 
        << m_renderables.size() << " renderables.";
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
::reset( void )
{
    LOG_INFO(g_log) << "Resetting Scene";
    m_passes.clear();
    m_renderables.clear();
}
