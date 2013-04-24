
#include "RenderPass.hpp"

#include "RenderTarget.hpp"
#include "RenderCommand.hpp"
#include "Projection.hpp"
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

ConstMaterialPtr
RenderPass
::getMaterialForRenderable( ConstRenderablePtr aRenderable ) const
{
    return aRenderable->getMaterialForPassName( m_name );
}

bool renderPassCompareByPriority( ConstRenderPassPtr a,
                                  ConstRenderPassPtr b )
{
    return a->priority() < b->priority();
}

RenderCommand createRenderCommand( ConstRenderPassPtr aRenderPass, 
                                   ConstRenderablePtr aRenderable )
{
    // Assert preconditions
    if( !aRenderPass ) 
    {
        LOG_ERROR(g_log) << "RenderPass is null in createRenderCommand";
        assert( false );
    }
    if( !aRenderable) 
    {
        LOG_ERROR(g_log) << "Renderable is null in createRenderCommand";
        assert( false );
    }

    RenderCommand rc;
    rc.m_pass = aRenderPass;
    rc.m_renderable = aRenderable;
    rc.m_material = aRenderPass->getMaterialForRenderable( aRenderable );
    rc.m_perspective = aRenderPass->m_perspective;

    if( !rc.m_material ) 
    {
        LOG_ERROR(g_log) << "RenderCommand has no material.";
        assert( false );
    }
    if( !rc.m_perspective )
    {
        LOG_ERROR(g_log) << "RenderCommand has no perspective.";
        assert( false );
    }
    if( !rc.m_renderable )
    {
        LOG_ERROR(g_log) << "RenderCommand has no renderable.";
        assert( false );
    }
    return rc;
}
