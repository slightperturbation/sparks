//
//  Projection.cpp
//  sparks
//
//  Created by Brian Allen on 3/26/13.
//
//

#include "Projection.hpp"
#include "Utilities.hpp"

spark::Projection
::Projection( void )
: m_aspectRatio( 800.0f/600.0f ),
  m_nearPlaneDist( 0.001f ),
  m_farPlaneDist( 100.0f )
{
}

void 
spark::Projection
::resizeViewport( int left, int bottom,
    int width, int height )
{
    aspectRatio( (float)width/(float)height );
}

///////////////////////////////////////////////////////////////////////////////


spark::PerspectiveProjection
::PerspectiveProjection( void )
: m_cameraPos( 0.0f, 0.0f, -1.0f ),
  m_cameraTarget( 0, 0, 0 ),
  m_cameraUp( 0.0f, 1.0f, 0.0f ),
  m_fov( 80.0f )
{
    unsetProjectionMatrix();
}

std::string
spark::PerspectiveProjection
::name( void ) const
{
    std::stringstream out;
    out << "Perspective(from={" << m_cameraPos
    << "}, to={"<<m_cameraTarget<<"})";
    return out.str();
}

glm::vec3
spark::PerspectiveProjection
::lookAtDirection( const glm::vec3& targetPoint ) const
{
    return m_cameraPos - targetPoint;
}

glm::vec3
spark::PerspectiveProjection
::upDirection( void ) const
{
    return m_cameraUp;
}

glm::mat4
spark::PerspectiveProjection
::cameraViewMatrix( void ) const
{
    return glm::lookAt( m_cameraPos,
        m_cameraTarget,
        m_cameraUp );
}

glm::mat4
spark::PerspectiveProjection
::projectionMatrix( void ) const
{
    if( m_projMat )
    {
        return *m_projMat;
    }
    else
    {
        return glm::perspective( m_fov,
            m_aspectRatio,
            m_nearPlaneDist,
            m_farPlaneDist );
    }
}

void 
spark::PerspectiveProjection
::setProjectionMatrix( const glm::mat4& mat )
{
    m_projMat.reset( mat );
}

void 
spark::PerspectiveProjection
::unsetProjectionMatrix( void )
{
    m_projMat.reset();
}

glm::mat4
spark::PerspectiveProjection
::viewMatrix( void ) const
{
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "ViewMatrix_EYE    = \n" << m_currEyeView << "\n";
        LOG_TRACE(g_log) << "ViewMatrix_CAMERA = \n" << cameraViewMatrix() << "\n";
        LOG_TRACE(g_log) << "ViewMatrix_TOTAL  = \n" << m_currEyeView * cameraViewMatrix() << "\n";
    }
    return m_currEyeView * cameraViewMatrix();
}

void 
spark::PerspectiveProjection
::setEyeViewMatrix( const glm::mat4& eyeMat ) 
{
    m_currEyeView = eyeMat;
}

void 
spark::PerspectiveProjection
::unsetEyeViewMatrix( void )
{
    m_currEyeView = glm::mat4();
}

///////////////////////////////////////////////////////////////////////////////

spark::OrthogonalProjection
::OrthogonalProjection()
: m_left( 0 ),
  m_right( 1 ),
  m_bottom( 0 ),
  m_top( 1 ),
  m_direction( 0, 0, 0 )
{
    m_nearPlaneDist = -5.1f;
    m_farPlaneDist = 5.1f;
}

std::string
spark::OrthogonalProjection
::name( void ) const
{
    std::stringstream out;
    out << "Ortho(L="
    << m_left << ", R=" << m_right << ", B="
    << m_bottom << ", T=" << m_top << ", N="
    << m_nearPlaneDist << ", F="
    << m_farPlaneDist << ")";
    return out.str();
}

glm::vec3
spark::OrthogonalProjection
::lookAtDirection( const glm::vec3& targetPoint ) const
{
    return m_direction;
}

glm::vec3
spark::OrthogonalProjection
::upDirection( void ) const
{
    return glm::vec3( 0, 1, 0 );
}

void 
spark::OrthogonalProjection
::setLookAtDirection( const glm::vec3& direction )
{ 
    m_direction = direction; 
}

glm::mat4 
spark::OrthogonalProjection
::viewMatrix( void ) const
{
    if( m_direction == glm::vec3( 0, 0, 0 ) )
    {
        return glm::mat4();
    }
    glm::vec3 eye( 0, 0, 0 );
    return glm::lookAt( lookAtDirection(eye), // target position
        eye,
        upDirection() );
}

glm::mat4 
spark::OrthogonalProjection
::projectionMatrix( void ) const
{
    return glm::ortho( m_left, m_right, m_bottom, m_top, 
                       m_nearPlaneDist, m_farPlaneDist );
}

///////////////////////////////////////////////////////////////////////////////





