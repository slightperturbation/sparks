
#include "RenderPass.hpp"

#include "RenderTarget.hpp"
#include "RenderCommand.hpp"
#include "Projection.hpp"
#include "Renderable.hpp"

spark::RenderPass
::RenderPass( const RenderPassName& aName )
: m_name( aName ),
  m_priority( 1.0f ),
  m_depthTest( true ),
  m_depthMask( true ),
  m_colorMask( true ),
  m_backfaceCulling( false ),
  m_cullFace( GL_BACK )
{
    useInterpolatedBlending();
}

void 
spark::RenderPass
::initialize( RenderTargetPtr aTarget, 
              PerspectivePtr aPerspective )
{ 
    m_target = aTarget; 
    m_perspective = aPerspective; 
}

void 
spark::RenderPass
::initialize( RenderTargetPtr aTarget, 
              PerspectivePtr aPerspective,
              float aPriority )
{ 
    m_target = aTarget; 
    m_perspective = aPerspective; 
    m_priority = aPriority; 
}

spark::RenderPassName 
spark::RenderPass
::name( void ) const 
{ 
    return m_name; 
}

std::string 
spark::RenderPass
::targetName( void ) const 
{ 
    return m_target->name();
}

void 
spark::RenderPass
::setName( const RenderPassName& aName ) 
{
    m_name = aName; 
}

void 
spark::RenderPass
::preRender( ConstRenderPassPtr prevPass ) const
{
    if( m_target ) 
    {
        // Blending
        if( !prevPass
            || (prevPass->m_isBlendingEnabled != m_isBlendingEnabled)
            || (prevPass->m_blendSourceFactor != m_blendSourceFactor)
            || (prevPass->m_blendDestinationFactor != m_blendDestinationFactor)
            || (prevPass->m_blendEquation != m_blendEquation)
          )
        {
            LOG_TRACE(g_log) << "RenderPass::preRender - setting blend mode";
            if( m_isBlendingEnabled )
            {
                GL_CHECK( glEnable( GL_BLEND ) );
                GL_CHECK( glBlendFunc( m_blendSourceFactor, m_blendDestinationFactor ) );
                GL_CHECK( glBlendEquation( m_blendEquation ) );
            }
            else
            {
                GL_CHECK( glDisable( GL_BLEND ) );
            }
        }
        // Depth Mask
        if( !prevPass || (prevPass->m_depthMask != m_depthMask) )
        {
            LOG_TRACE(g_log) << "RenderPass::preRender - glDepthMask( "
                << ( m_depthMask ? "true" : "false") << " )";
            GL_CHECK( glDepthMask( m_depthMask ) );
        }
        // Color Mask
        if( !prevPass || (prevPass->m_colorMask != m_colorMask) )
        {
            LOG_TRACE(g_log) << "RenderPass::preRender - glColorMask( "
            << ( m_colorMask ? "true" : "false") << " )";
            GL_CHECK( glColorMask( m_colorMask,
                                   m_colorMask,
                                   m_colorMask,
                                   m_colorMask ) );
        }        
        // Depth Test
        if( !prevPass || prevPass->m_depthTest != m_depthTest )
        {
            LOG_TRACE(g_log) << "RenderPass::preRender - glDepthTest( "
                             << ( m_depthTest ? "true" : "false" ) << " )";
            if( m_depthTest )
            {
                GL_CHECK( glEnable( GL_DEPTH_TEST ) );
            }
            else
            {
                GL_CHECK( glDisable( GL_DEPTH_TEST ) );
            }
        }
        // Backface culling
        if( !prevPass || (prevPass->m_backfaceCulling != m_backfaceCulling) )
        {
            if( m_backfaceCulling )
            {
                glEnable( GL_CULL_FACE );
                if( prevPass->m_cullFace != m_cullFace )
                {
                    if( (m_cullFace == GL_FRONT)
                       || ( m_cullFace == GL_BACK)
                       || ( m_cullFace == GL_FRONT_AND_BACK )
                       )
                    {
                        glCullFace( m_cullFace );
                    }
                    else
                    {
                        LOG_ERROR(g_log) << "Pass " << name() << " has invalid"
                                         << " constant for cull face"
                                         << " (setCullFace())";
                    }
                }
            }
            else
            {
                glDisable( GL_CULL_FACE );
            }
        }
    }
}

void 
spark::RenderPass
::postRender( ConstRenderPassPtr prevPass ) const
{
    if(   m_target && 
        ( !prevPass || (prevPass->m_target != m_target) ) ) 
    { 
        m_target->postRender(); 
    }
}

void
spark::RenderPass
::startFrame( ConstRenderPassPtr prevPass ) const
{
    if(   m_target && 
        ( !prevPass || (prevPass->m_target != m_target) ) ) 
    { 
        m_target->startFrame(); 
    }
}

spark::ConstMaterialPtr
spark::RenderPass
::getMaterialForRenderable( ConstRenderablePtr aRenderable ) const
{
    ConstMaterialPtr mat = aRenderable->getMaterialForPassName( m_name );
    // Assign a default material if:
    if(   !mat  // it doesn't already have one
       &&  m_defaultMaterial // this pass has a default
       && (!aRenderable->requiresExplicitMaterial()) // the renderable allows it
    )
    {
        mat = m_defaultMaterial;
    }
    return mat;
}

void
spark::RenderPass
::setBlending( GLenum sourceFactor, 
               GLenum destinationFactor,
               GLenum equation )
{
    m_isBlendingEnabled = true;
    m_blendSourceFactor = sourceFactor;
    m_blendDestinationFactor = destinationFactor;
    m_blendEquation = equation;
}

void
spark::RenderPass
::useAdditiveBlending( void )
{
    setBlending( GL_ONE, GL_ONE, GL_FUNC_ADD );
}

void
spark::RenderPass
::useInterpolatedBlending( void )
{
    setBlending( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD );
}

void
spark::RenderPass
::useModulatedBlending( void )
{
    setBlending( GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD );
}

void
spark::RenderPass
::disableBlending( void )
{
    m_isBlendingEnabled = false;
}

void
spark::RenderPass
::enableBlending( void )
{
    m_isBlendingEnabled = true;
}

void
spark::RenderPass
::useDefaultMaterial( ConstMaterialPtr defaultMaterial )
{
    m_defaultMaterial = defaultMaterial;
}

spark::ConstMaterialPtr
spark::RenderPass
::defaultMaterial( void ) const
{
    return m_defaultMaterial;
}

void
spark::RenderPass
::setDepthTest( bool isDepthTestEnabled )
{
    m_depthTest = isDepthTestEnabled;
}

bool
spark::RenderPass
::depthTest( void ) const
{
    return m_depthTest;
}

void 
spark::RenderPass
::setDepthWrite( bool isWritingDepth )
{
    m_depthMask = isWritingDepth;
}
bool 
spark::RenderPass
::depthWrite( void ) const
{
    return m_depthMask;
}

void
spark::RenderPass
::setColorWrite( bool isWritingColor )
{
    m_colorMask = isWritingColor;
}
bool
spark::RenderPass
::colorWrite( void ) const
{
    return m_colorMask;
}

bool 
spark
::renderPassCompareByPriority( ConstRenderPassPtr a,
                               ConstRenderPassPtr b )
{
    return a->priority() > b->priority();
}

bool 
spark
::createRenderCommand( RenderCommand& outRC, 
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

const char* nameForBlendConstant( GLenum blendConstant )
{
    switch( blendConstant )
    {
    case GL_ZERO: return "GL_ZERO";
    case GL_ONE:  return "GL_ONE";
    case GL_SRC_COLOR: return "GL_SRC_COLOR";
    case GL_ONE_MINUS_SRC_COLOR: return "GL_ONE_MINUS_SRC_COLOR";
    case GL_DST_COLOR: return "GL_DST_COLOR";
    case GL_ONE_MINUS_DST_COLOR: return "GL_ONE_MINUS_DST_COLOR";
    case GL_SRC_ALPHA: return "GL_SRC_ALPHA";
    case GL_ONE_MINUS_SRC_ALPHA: return "GL_ONE_MINUS_SRC_ALPHA";
    case GL_DST_ALPHA: return "GL_DST_ALPHA";
    case GL_ONE_MINUS_DST_ALPHA: return "GL_ONE_MINUS_DST_ALPHA";
    case GL_CONSTANT_COLOR: return "GL_CONSTANT_COLOR";
    case GL_ONE_MINUS_CONSTANT_COLOR: return "GL_ONE_MINUS_CONSTANT_COLOR";
    case GL_CONSTANT_ALPHA: return "GL_CONSTANT_ALPHA";
    case GL_ONE_MINUS_CONSTANT_ALPHA: return "GL_ONE_MINUS_CONSTANT_ALPHA";
    case GL_SRC1_COLOR: return "GL_SRC1_COLOR";
    case GL_ONE_MINUS_SRC1_COLOR: return "GL_ONE_MINUS_SRC1_COLOR";
    case GL_SRC1_ALPHA: return "GL_SRC1_ALPHA";
    case GL_ONE_MINUS_SRC1_ALPHA: return "GL_ONE_MINUS_SRC1_ALPHA";
        // Equations
    case GL_FUNC_ADD: return "GL_FUNC_ADD";
    case GL_FUNC_SUBTRACT: return "GL_FUNC_SUBTRACT";
    case GL_FUNC_REVERSE_SUBTRACT: return "GL_FUNC_REVERSE_SUBTRACT";
    case GL_MIN: return "GL_MIN";
    case GL_MAX: return "GL_MAX";
    default:
        return "UNKNOWN";
    }
}

std::ostream& spark::operator<<( std::ostream& out,
                                 ConstRenderPassPtr pass )
{
    out << "\"" << pass->name() << "\"(pri=" << pass->m_priority
        << ")(Target=" << pass->m_target << ")";
    if( pass->m_blendEquation == -1 )
    {
        out << "(BlendingDisabled)";
    }
    else
    {
        out << "(Blend:src=" 
            << nameForBlendConstant( pass->m_blendSourceFactor )
            << ",dst="
            << nameForBlendConstant( pass->m_blendDestinationFactor )
            << ",eqn="
            << nameForBlendConstant( pass->m_blendEquation )
            << ")";
    }
    return out;
}

std::ostream& spark::operator<<( std::ostream& out,
                                 RenderPassPtr pass )
{
    ConstRenderPassPtr cp = pass;
    out << cp;
    return out;
}







