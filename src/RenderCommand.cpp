
#include "RenderCommand.hpp"
#include "RenderPass.hpp"
#include "Projection.hpp"
#include "Material.hpp"
#include "Renderable.hpp"


#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h> // for time
#include <glm/glm.hpp>

void
spark::RenderCommand
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
    
    // setup transformation uniforms on shader
    // OK to cast-away const because we are in the execution of
    // the render command, thus guaranteed to be in a single-thread
    // due to OpenGL calls being limited to a single thread.
    MaterialPtr mutableMaterial = spark::const_pointer_cast<Material>( m_material );
    
    // Define Common uniforms
    ///////////////////////////////////////////////////////////////////
    //// Common Uniforms (see RenderCommand)
    //// GLSL:
    //uniform mat4 u_projViewModelMat;     // projection * view * model
    //uniform mat4 u_viewModelMat;         // transforms object into camera(eye) space
    //uniform mat4 u_inverseViewModelMat;  // inverse of the model-view matrix, can give camera position
    //uniform mat4 u_projMat;              // projects camera(eye) space to clip(screen) space
    //uniform mat3 u_normalMat;            // transpose(inverse(viewModelMat))
    //uniform float u_time;                // current time (in seconds)
    //uniform vec2 u_targetSizeInPixels;   // Size of the render target in pixels
    ///////////////////////////////////////////////////////////////////
    const glm::mat4 model = m_renderable->getTransform();
    const glm::mat4 view = m_perspective->viewMatrix();
    const glm::mat4 proj = m_perspective->projectionMatrix();
    const glm::mat4 viewModel = view * model;
    const glm::mat4 invViewModel = glm::inverse( viewModel );
    const glm::mat4 projViewModel = proj * viewModel;
    const glm::mat3 normal = glm::transpose( glm::inverse( glm::mat3(viewModel) ) );
    const float time = glfwGetTime();

    mutableMaterial->setShaderUniform<glm::mat4>( "u_projMat", 
                                                  proj );
    mutableMaterial->setShaderUniform<glm::mat4>( "u_viewModelMat", 
                                                  viewModel );
    mutableMaterial->setShaderUniform<glm::mat4>( "u_inverseViewModelMat", 
                                                  invViewModel );
    mutableMaterial->setShaderUniform<glm::mat4>( "u_projViewModelMat", 
                                                  projViewModel );
    mutableMaterial->setShaderUniform<glm::mat3>( "u_normalMat", 
                                                  normal );
    mutableMaterial->setShaderUniform<float>( "u_time", time );
    mutableMaterial->setShaderUniform<glm::vec2>( "u_targetSizeInPixels",
                                                 m_pass->targetSize() );
    
    // Lights
    //m_illumination->use( mutableMaterial );
    //mutableMaterial->setShaderUniform<glm::vec4>( "u_")

    const bool isVerboseDebug = false;
    if( isVerboseDebug )
    {
        m_material->dumpShaderUniforms();
    }
    m_material->use();
    m_renderable->render( *this );
}

bool
spark::RenderCommandCompare
::operator()( const RenderCommand& a, const RenderCommand& b ) const
{
    // TODO Render commands should be ordered by:
    //  - pass
    //  - depth (TODO defer to RenderPass sort order!)
    //  - shader
    //  - textures
    
    // Sort by render pass
    bool isALessThanB = renderPassCompareByPriority( a.m_pass, b.m_pass );

    const bool traceSortOrder = false;
    if( traceSortOrder )
    {
        LOG_TRACE(g_log) << "****BEGIN Compare RenderCommands:";
        LOG_TRACE(g_log) << "\tA: " << a;
        LOG_TRACE(g_log) << "\tB: " << b;
        LOG_TRACE(g_log) << "\tfirst rendered is "
        << ( isALessThanB ? "B" : "A" )
        << " because renderPassCompareByPriority(a,b) is "
        << (isALessThanB ? "true" : "false" )
        << " and highest priority goes first.";
        LOG_TRACE(g_log) << "****END   Compare RenderCommands";
    }
    return isALessThanB;
}


std::ostream& spark::operator<<( std::ostream& out, const RenderCommand& rc )
{
    out << "Pass[" << rc.m_pass 
        << "]\tPersp[" << rc.m_perspective->name()
        << "]\tRenderable[" << rc.m_renderable->name() 
        << "]\tMaterial[" << rc.m_material->name() << "]";
    if( rc.m_illumination )
    {
        out << "\tIllumination[" << rc.m_illumination->name() << "]";
    }
    return out;
}
