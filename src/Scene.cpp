
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
    m_passes.sort( renderPassCompareByPriority );

    for( auto rp = m_passes.begin(); rp != m_passes.end(); ++rp )
    {
        for( auto r = m_renderables.begin(); r != m_renderables.end(); ++r )
        {
            createRenderCommand( *rp, *r );
        }
    }
}

void
Scene
::render( void )
{
    const RenderPass* lastRenderPass = NULL;
    // Render each render command in order.
    while( !m_commands.empty() )
    {
        const RenderCommand& rc = m_commands.top();
        const RenderPass* currRenderPass = rc.m_pass.get();
        if( currRenderPass != lastRenderPass && currRenderPass != NULL )
        {
            // New render pass, allow both old and new to change GL state.
            lastRenderPass->postRender();
            currRenderPass->preRender();
        }
        rc();
        m_commands.pop();
    }
    if( lastRenderPass )
    {
        // Finally, allow the last-set render pass to cleanup state
        lastRenderPass->postRender();
    }
}