
#include "RenderCommand.hpp"
#include "RenderPass.hpp"
#include "Perspective.hpp"
#include "Material.hpp"
#include "Renderable.hpp"

#include <glm/glm.hpp>

void
RenderCommand
::operator()( const RenderCommand& precedingCommand )
{
    // Check preconditions
    if( !m_material ) 
    {
        LOG_ERROR(g_log) << "RenderCommand called with no material.";
        assert( false );
    }
    if( !m_perspective )
    {
        LOG_ERROR(g_log) << "RenderCommand called with no perspective.";
        assert( false );
    }
    if( !m_renderable )
    {
        LOG_ERROR(g_log) << "RenderCommand called with no renderable.";
        assert( false );
    }
    if( precedingCommand.m_perspective != this->m_perspective )
    {
        // setup uniforms on shader
        // OK to cast-away const because we are in the execution of
        // the render command, thus guaranteed to be in a single-thread
        // due to OpenGL calls being limited to a single thread.
        MaterialPtr mutableMaterial = std::const_pointer_cast<Material>( m_material );
        mutableMaterial->usePerspective( m_perspective );
    }
    if( precedingCommand.m_material != this->m_material )
    {
        m_material->use();
    }
    m_renderable->render();
}

bool
RenderCommandCompare
::operator()( const RenderCommand& a, const RenderCommand& b ) const
{
    // Render commands are order by:
    //  - pass
    //  - transparency
    //  - depth ( front to back for opaque, back to front for transparent)
    //  - shader
    //  - textures
    //  
    if( a.m_pass->priority() != a.m_pass->priority() )
    {
        return a.m_pass->priority() < a.m_pass->priority();
    }
    return false;
}
