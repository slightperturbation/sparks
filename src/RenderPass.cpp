
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

bool createRenderCommand( RenderCommand& outRC, 
                          ConstRenderPassPtr aRenderPass, 
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

    outRC.m_pass = aRenderPass;
    outRC.m_renderable = aRenderable;
    outRC.m_material = aRenderPass->getMaterialForRenderable( aRenderable );
    outRC.m_perspective = aRenderPass->m_perspective;

    if( !outRC.m_material ) 
    {
        return false;
    }
    if( !outRC.m_perspective )
    {
        LOG_ERROR(g_log) << "RenderCommand has no perspective.";
        assert( false );
    }
    if( !outRC.m_renderable )
    {
        LOG_ERROR(g_log) << "RenderCommand has no renderable.";
        assert( false );
    }
    return true;
}
