
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
    LOG_TRACE(g_log) << "Scene::render with "
                     << m_commands.size() << " commands, "
                     << m_passes.size() << " passes and " 
                     << m_renderables.size() << " renderables.";
    const RenderPass* lastRenderPass = NULL;
    RenderCommand lastRenderCommand, rc;
    // Render each render command in order.
    int counter = 0;
    while( !m_commands.empty() )
    {
        RenderCommand rc = m_commands.top();
        LOG_TRACE(g_log) << "----Executing RenderCommand " << ++counter << ": " << rc;
        const RenderPass* currRenderPass = rc.m_pass.get();
        if( currRenderPass != lastRenderPass )
        {
            // New render pass, allow both old and new to change GL state.
            if( lastRenderPass ) lastRenderPass->postRender();
            if( currRenderPass ) currRenderPass->preRender();
        }
        rc( lastRenderCommand );
        lastRenderCommand = rc;
        m_commands.pop();
    }
    if( lastRenderPass )
    {
        // Finally, allow the last-set render pass to cleanup state
        lastRenderPass->postRender();
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

