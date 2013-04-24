
#include "Scene.hpp"

#include <functional>

Scene
::Scene( void )
{

}

Scene
::~Scene( )
{

}

void
Scene
::prepareRenderCommands( void )
{
    LOG_TRACE(g_log) << "Scene::prepareRenderCommands with " 
        << m_passes.size() << " passes and " 
        << m_renderables.size() << " renderables.";
    m_passes.sort( renderPassCompareByPriority );

    for( auto rp = m_passes.begin(); rp != m_passes.end(); ++rp )
    {
        for( auto r = m_renderables.begin(); r != m_renderables.end(); ++r )
        {
            m_commands.push( createRenderCommand( *rp, *r ) );
        }
    }
}

void
Scene
::render( void )
{
    LOG_TRACE(g_log) << "Scene::render with "
        << m_commands.size() << " commands, "
        << m_passes.size() << " passes and " 
        << m_renderables.size() << " renderables.";

    const RenderPass* lastRenderPass = NULL;
    RenderCommand lastRenderCommand, rc;
    // Render each render command in order.
    while( !m_commands.empty() )
    {
        RenderCommand rc = m_commands.top();
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