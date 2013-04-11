
#include "RenderCommand.hpp"
#include "RenderPass.hpp"

void
RenderCommand
::operator()( void )
{

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
