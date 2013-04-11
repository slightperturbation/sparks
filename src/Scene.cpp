
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
    RenderPass* lastRenderPass = NULL;
    // Render each render command in order.
    while( !m_commands.empty() )
    {
        RenderCommand& rc = m_commands.top();
        RenderPass* currRenderPass = rc.m_pass.get();
        if( currRenderPass != lastRenderPass && currRenderPass != NULL )
        {
            rc.m_pass->setTarget();
        }
        rc();
        m_commands.pop();
    }
}