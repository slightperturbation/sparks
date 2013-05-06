
#include "RenderCommand.hpp"
#include "RenderPass.hpp"
#include "Projection.hpp"
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
    
    bool isNewMaterial = false;
    if( precedingCommand.m_material != this->m_material )
    {
        isNewMaterial = true;
        // setup transformation uniforms on shader
        // OK to cast-away const because we are in the execution of
        // the render command, thus guaranteed to be in a single-thread
        // due to OpenGL calls being limited to a single thread.
        MaterialPtr mutableMaterial = std::const_pointer_cast<Material>( m_material );
        
        // Perspective
        const glm::mat4 model = m_renderable->getTransform();
        const glm::mat4 view = m_perspective->viewMatrix();
        const glm::mat4 proj = m_perspective->projectionMatrix();
        const glm::mat4 viewModel = view * model;
        const glm::mat4 projViewModel = proj * viewModel;
        const glm::mat3 normal = glm::transpose( glm::inverse( glm::mat3(viewModel) ) );
        
        mutableMaterial->setShaderUniform<glm::mat4>( "u_projMat", proj );
        mutableMaterial->setShaderUniform<glm::mat4>( "u_viewModelMat", viewModel );
        mutableMaterial->setShaderUniform<glm::mat4>( "u_projViewModelMat", projViewModel );
        mutableMaterial->setShaderUniform<glm::mat3>( "u_normalMat", normal );
        
    }
    if( isNewMaterial || (precedingCommand.m_illumination != m_illumination) )
    {
        // Lights
        MaterialPtr mutableMaterial = std::const_pointer_cast<Material>( m_material );

        //m_illumination->use( mutableMaterial );
        //mutableMaterial->setShaderUniform<glm::vec4>( "u_")
    }
    
    // Show all the uniforms that have been set above for debugging
    m_material->dumpShaderUniforms();    
    m_material->use();
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


std::ostream& operator<<( std::ostream& out, const RenderCommand& rc )
{
    out << "Pass[" << rc.m_pass->name() 
        << "]\tPersp[" << rc.m_perspective->name()
        << "]\tRenderable[" << rc.m_renderable->name() 
        << "]\tMaterial[" << rc.m_material->name() << "]";
    if( rc.m_illumination )
    {
        out << "\tIllumination[" << rc.m_illumination->name() << "]";
    }
    return out;
}
