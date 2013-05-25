#include "Display.hpp"

#include "Viewport.hpp"
#include "Projection.hpp"
#include "Renderable.hpp"


spark::Display
::Display( PerspectivePtr context )
    : m_context( context )
{

}

spark::SimpleDisplay
::SimpleDisplay( PerspectivePtr context )
    : Display( context )
{
}

void
spark::SimpleDisplay
::resizeWindow( int width, int height ) 
{
    m_viewport.setExtents( 0, 0, width, height );
}

void 
spark::SimpleDisplay
::render( const Renderables& scene )
{
    m_context->aspectRatio( float(m_viewport.m_width)/float(m_viewport.m_height) );
    m_viewport.render( scene, m_context );
}

//////////////////////////////////////////////////////////////////////////

spark::SideBySideDisplay
::SideBySideDisplay( PerspectivePtr context )
    : Display( context ), m_eyeSeparationDistance( 0.0 )
{
}

void
spark::SideBySideDisplay
::resizeWindow( int width, int height )
{
    // update viewports
    m_rightViewport.setExtents( 0,0, width/2, height );
    m_leftViewport.setExtents( width/2, 0, width/2, height );
}

void
spark::SideBySideDisplay
::render( const Renderables& scene )
{
    m_context->aspectRatio( float(m_rightViewport.m_width)/float(m_rightViewport.m_height) );

    //glm::vec3 rightVec = glm::cross( ( m_context->cameraTarget() - m_context->cameraPos() ), m_context->cameraUp() );
    //rightVec = glm::normalize( rightVec );

    // TODO -- need to wrap and hold the rendercontext?
    // change viewpoint, make a duplicate context, shift it
    //{
    //    RenderContext right( m_context );
    //    right.cameraPos() += rightVec * m_eyeSeparationDistance;
    //    m_rightViewport.render( scene, right );
    //}
    //{
    //    RenderContext left( m_context );
    //    left.cameraPos() -= rightVec * m_eyeSeparationDistance;
    //    m_leftViewport.render( scene, left );
    //}
}
