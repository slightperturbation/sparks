#include "Display.hpp"

#include "Viewport.hpp"
#include "RenderContext.hpp"
#include "Renderable.hpp"


Display::Display( RenderContext& context )
    : m_context( context )
{

}

SimpleDisplay::SimpleDisplay( RenderContext& context )
    : Display( context )
{
}

void SimpleDisplay::resizeWindow( int width, int height ) 
{
    m_viewport.setExtents( 0, 0, width, height );
}

void SimpleDisplay::render( const Renderables& scene )
{
    m_context.setAspectRatio( float(m_viewport.m_width)/float(m_viewport.m_height) );
    m_viewport.render( scene, m_context );
}

//////////////////////////////////////////////////////////////////////////

SideBySideDisplay::SideBySideDisplay( RenderContext& context )
    : Display( context ), m_eyeSeparationDistance( 0.0 )
{
}

void SideBySideDisplay::resizeWindow( int width, int height )
{
    // update viewports
    m_rightViewport.setExtents( 0,0, width/2, height );
    m_leftViewport.setExtents( width/2, 0, width/2, height );
}

void SideBySideDisplay::render( const Renderables& scene )
{
    glClearColor( 0.3, 0.3, 0.5, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    m_context.setAspectRatio( float(m_rightViewport.m_width)/float(m_rightViewport.m_height) );


    glm::vec3 rightVec = glm::cross( ( m_context.cameraTarget() - m_context.cameraPos() ), m_context.cameraUp() );
    rightVec = glm::normalize( rightVec );

    // change viewpoint, make a duplicate context, shift it
    {
        RenderContext right( m_context );
        right.cameraPos() += rightVec * m_eyeSeparationDistance;
        m_rightViewport.render( scene, right );
    }
    {
        RenderContext left( m_context );
        left.cameraPos() -= rightVec * m_eyeSeparationDistance;
        m_leftViewport.render( scene, left );
    }
}
