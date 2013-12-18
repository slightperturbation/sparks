#include "Display.hpp"

#include "Projection.hpp"
#include "Renderable.hpp"
#include "StateManager.hpp"
#include "RenderTarget.hpp"

spark::Display
::Display( void )
{
    // Noop
}

void
spark::Display
::resizeViewport( int left, int bottom,
                  int width, int height )
{
    m_windowLeft = left;
    m_windowBottom = bottom;
    m_windowWidth = width;
    m_windowHeight = height;
}

void
spark::Display
::setPerspective( PerspectiveProjectionPtr camera )
{
    m_camera = camera;
}

void
spark::Display
::setEyeTracker( EyeTrackerPtr eyeTracker )
{
    m_eyeTracker = eyeTracker;
}

void
spark::Display
::setFrameBufferRenderTarget( FrameBufferRenderTargetPtr target )
{
    m_frameBuffer = target;
}

//////////////////////////////////////////////////////////////////////////

spark::MonoDisplay
::MonoDisplay( void )
{
}


void
spark::MonoDisplay
::render( StateManager& stateManager )
{
    m_camera->aspectRatio( float(m_windowWidth)/float(m_windowHeight) );
    m_frameBuffer->resizeViewport( m_windowLeft, m_windowBottom,
                                   m_windowWidth, m_windowHeight );

    if( m_eyeTracker )
    {
        m_eyeTracker->updatePerspective( m_camera );
    }
    stateManager.render();
}

//////////////////////////////////////////////////////////////////////////

spark::SideBySideDisplay
::SideBySideDisplay(  )
    : m_eyeSeparationDistance( 0.1 )
{
}

void
spark::SideBySideDisplay
::render( StateManager& stateManager )
{
    m_camera->aspectRatio( float(m_windowWidth/2)/float(m_windowHeight) );

    glm::vec3 rightVec = glm::cross( ( m_camera->cameraTarget() - m_camera->cameraPos() ), m_camera->cameraUp() );
    rightVec = glm::normalize( rightVec );
    
    
    // Right eye
    m_frameBuffer->resizeViewport( m_windowLeft, m_windowBottom,
                                   m_windowWidth/2, m_windowHeight );
    m_camera->cameraPos() += rightVec * m_eyeSeparationDistance;
    m_eyeTracker->updatePerspective( m_camera, EyeTracker::rightEye );
    stateManager.render();

    // Left eye
    m_frameBuffer->resizeViewport( m_windowLeft + m_windowWidth/2, m_windowBottom,
                                   m_windowWidth/2, m_windowHeight );
    m_camera->cameraPos() -= rightVec * m_eyeSeparationDistance;
    m_eyeTracker->updatePerspective( m_camera, EyeTracker::leftEye );
    stateManager.render();
}

void
spark::SideBySideDisplay
::enableOculusDistortion( void )
{
    // set shader on framebuffer render target
    
}

void
spark::SideBySideDisplay
::disableOculusDistoration( void )
{
    
}

//////////////////////////////////////////////////////////////////////////

spark::QuadBufferStereoDisplay
::QuadBufferStereoDisplay()
{
    // Noop
}

void
spark::QuadBufferStereoDisplay
::render( StateManager& stateManager )
{
    m_camera->aspectRatio( float(m_windowWidth)/float(m_windowHeight) );
    m_frameBuffer->resizeViewport( m_windowLeft, m_windowBottom,
                                   m_windowWidth, m_windowHeight );

    glDrawBuffer( GL_BACK_RIGHT );
    m_eyeTracker->updatePerspective( m_camera, EyeTracker::rightEye );
    stateManager.render(); // Extra scene render in stereo mode
    
    glDrawBuffer( GL_BACK_LEFT );
    m_eyeTracker->updatePerspective( m_camera, EyeTracker::leftEye );
    stateManager.render();
}
