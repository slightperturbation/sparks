
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

    ConstRenderPassPtr prevRenderPass( nullptr );
    RenderCommand prevRenderCommand, rc;
    // Render each render command in order.
    int counter = 0;
    
    // Allow passes and their targets to clear and setup buffers
    for( auto pass = m_passes.begin(); pass != m_passes.end(); ++pass )
    {
        (*pass)->startFrame( prevRenderPass );
        prevRenderPass = *pass;
    }
    // Render all accumulated passes
    prevRenderPass = ConstRenderPassPtr( nullptr );
    while( !m_commands.empty() )
    {
        RenderCommand rc = m_commands.top();
        LOG_TRACE(g_log) << "----Executing RenderCommand " << counter++ << ": " << rc;
        ConstRenderPassPtr currRenderPass = rc.m_pass;
        if( currRenderPass != prevRenderPass )
        {
            // New render pass, allow both old and new to change GL state.
            if( prevRenderPass ) 
            {
                prevRenderPass->postRender( prevRenderPass );
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

