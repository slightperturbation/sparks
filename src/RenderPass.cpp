
#include "RenderPass.hpp"

#include "RenderTarget.hpp"
#include "RenderCommand.hpp"
#include "Perspective.hpp"
#include "Renderable.hpp"

RenderPass
::RenderPass( void )
    : m_priority( 1.0f )
{

}

void 
RenderPass
::preRender( void ) const
{
    if( m_target ) { m_target->preRender(); }
}

void 
RenderPass
::postRender( void ) const
{
    if( m_target ) { m_target->postRender(); }
}

bool renderPassCompareByPriority( ConstRenderPassPtr a, 
                                  ConstRenderPassPtr b )
{
    return a->priority() < b->priority();
}

RenderCommand createRenderCommand( ConstRenderPassPtr aRenderPass, 
                                   ConstRenderablePtr aRenderable )
{
    RenderCommand rc;
    rc.m_pass = aRenderPass;
    rc.m_renderable = aRenderable;
    rc.m_perspective = aRenderPass->m_perspective;

    return rc;
}
