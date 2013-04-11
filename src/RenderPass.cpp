
#include "RenderPass.hpp"

#include "RenderCommand.hpp"
#include "Perspective.hpp"
#include "Renderable.hpp"

bool renderPassCompareByPriority( ConstRenderPassPtr a, 
                                  ConstRenderPassPtr b )
{
    return a->priority() < b->priority();
}

RenderCommand createRenderCommand( RenderPassPtr aRenderPass, 
                                   ConstRenderablePtr aRenderable )
{
    RenderCommand rc;
    rc.m_pass = aRenderPass;
    rc.m_renderable = aRenderable;
    rc.m_perspective = aRenderPass->m_perspective;

    return rc;
}
