//
//  Projection.cpp
//  sparks
//
//  Created by Brian Allen on 3/26/13.
//
//

#include "Projection.hpp"

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
::viewMatrix( void ) const
{
    return m_view;
}

glm::mat4
spark::PerspectiveProjection
::projectionMatrix( void ) const
{
    return m_projection;
}

void 
spark::PerspectiveProjection
::setViewMatrix( const glm::mat4& mat )
{
    m_view = mat;
}

void 
spark::PerspectiveProjection
::setProjectionMatrix( const glm::mat4& mat )
{
    m_projection = mat;
}

void 
spark::PerspectiveProjection
::setMatricesFromCamera( void )
{
    m_view = glm::lookAt( m_cameraPos,
        m_cameraTarget,
        m_cameraUp );
    m_projection = glm::perspective( m_fov,
        m_aspectRatio,
        m_nearPlaneDist,
        m_farPlaneDist );
}


///////////////////////////////////////////////////////////////////////////////

spark::OrthogonalProjection
::OrthogonalProjection()
: m_left( 0 ),
  m_right( 1 ),
  m_bottom( 0 ),
  m_top( 1 )
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
    return glm::vec3( 0, 0, -1 );
}

glm::vec3
spark::OrthogonalProjection
::upDirection( void ) const
{
    return glm::vec3( 0, 1, 0 );
}
