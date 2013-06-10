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
  m_nearPlaneDist( 0.25f ),
  m_farPlaneDist( 20.0f )
{
}

///////////////////////////////////////////////////////////////////////////////


spark::PerspectiveProjection
::PerspectiveProjection( void )
: m_cameraPos( 0.0f, 0.0f, -1.0f ),
  m_cameraTarget( 0, 0, 0 ),
  m_cameraUp( 0.0f, 1.0f, 0.0f ),
  m_fov( 80.0f )
{ }

std::string
spark::PerspectiveProjection
::name( void ) const
{
    std::stringstream out;
    out << "Perspective(from={" << m_cameraPos
    << "}, to={"<<m_cameraTarget<<"})";
    return out.str();
}


glm::mat4
spark::PerspectiveProjection
::viewMatrix( void ) const
{
    glm::mat4 view = glm::lookAt(m_cameraPos,
                                 m_cameraTarget,
                                 m_cameraUp );
    return view;
}


glm::mat4
spark::PerspectiveProjection
::projectionMatrix( void ) const
{
    return glm::perspective( m_fov,
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
    m_nearPlaneDist = -10.1f;
    m_farPlaneDist = 10.1f;
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
