#include "ArcBall.hpp"

#include "Projection.hpp"

spark::ArcBall
::ArcBall( void )
: m_left(0), m_right(0), m_bottom(0), m_top(0),
  m_isRotating(false), m_isDollying(false), m_isTracking(false),
  m_dollySpeed(1.0f),
  m_dollyStart(0, 0), m_dollyCurr(0, 0),
  m_trackingStart(0, 0), m_trackingCurr(0, 0),
  m_rotatingStart(0, 0), m_rotatingCurr(0, 0)
{ }

void
spark::ArcBall
::setExtents( int left, int bottom, int right, int top )
{
    m_left = left; m_bottom = bottom;  m_right = right;  m_top = top;
}

void
spark::ArcBall
::onMouseMove( int x, int y )
{
    if( x < m_left || x > m_right || y < m_bottom || y > m_top )
    {
        // ignore click outside of extents
        return;
    }
    if( m_isRotating )
    {
        m_rotatingCurr = glm::ivec2(x, y);
    }
    if( m_isDollying )
    {
        m_dollyCurr = glm::ivec2( x, y );
    }
    if( m_isTracking )
    {
        m_trackingCurr = glm::ivec2( x, y );
    }
}

void
spark::ArcBall
::onMouseRotationButton( int x, int y, bool isPressed )
{
    m_isRotating = isPressed;
    if( isPressed )
    {
        m_rotatingStart = m_rotatingCurr = glm::ivec2( x, y );
    }
}

void
spark::ArcBall
::onMouseDollyButton( int x, int y, bool isPressed )
{
    m_isDollying = isPressed;
    if( isPressed )
    {
        m_dollyCurr = m_dollyStart = glm::ivec2( x, y );
    }
}

void
spark::ArcBall
::onMouseTrackingButton( int x, int y, bool isPressed )
{
    m_isTracking = isPressed;
    if( isPressed )
    {
        m_trackingCurr = m_trackingStart = glm::ivec2( x, y );
    }
}

glm::mat4
spark::ArcBall
::rotation( void )
{
    if( (m_rotatingStart == m_rotatingCurr) )
    {
        return glm::mat4(1.0f); // no rotation
    }
    glm::vec3 va = getDirToSurfacePoint( m_rotatingStart.x, m_rotatingStart.y );
    glm::vec3 vb = getDirToSurfacePoint( m_rotatingCurr.x, m_rotatingCurr.y );
    if( va.z == 0 || vb.z == 0 ) return glm::mat4(1.0f);
    m_rotatingStart = m_rotatingCurr;
    
    float angle = std::acos( std::min(1.0f, glm::dot( va, vb ) ) );
    glm::vec3 rotationAxis = glm::normalize(glm::cross( va, vb ));
    glm::mat4 rot = glm::rotate( glm::mat4(1.0f), glm::degrees(angle), rotationAxis );
    return rot;
}

void
spark::ArcBall
::updatePerspective( PerspectiveProjectionPtr persp )
{
    // move camera according to dolly
    if( m_dollyStart != m_dollyCurr )
    {
        glm::vec3 p_world = persp->cameraPos();
        glm::vec3 t_world = persp->cameraTarget();
        
        // in direction of view
        float height = m_top - m_bottom;
        float view_dist = (m_dollyStart.y - m_dollyCurr.y) / height;
        glm::vec3 offset_world = view_dist * m_dollySpeed * (p_world - t_world);
        
        persp->cameraPos( offset_world + p_world );
        persp->cameraTarget( offset_world + t_world );
        m_dollyStart = m_dollyCurr;
    }
    
    // Move camera according to tracking
    if( m_trackingStart != m_trackingCurr )
    {
        glm::vec3 p_world = persp->cameraPos();
        glm::vec3 t_world = persp->cameraTarget();
        glm::vec3 u_world = persp->cameraUp();
        glm::vec3 forward_world = t_world - p_world;
        glm::vec3 right_world = glm::cross( forward_world, u_world );
        float width = m_right - m_left;
        float height = m_top - m_bottom;
        glm::vec3 offset_world = ((m_trackingStart.y - m_trackingCurr.y)/height)
                                 * forward_world;
        offset_world += ( (m_trackingStart.x - m_trackingCurr.x)/width )
                        * right_world;
        persp->cameraPos( p_world + offset_world );
        persp->cameraTarget( t_world + offset_world );
        m_trackingStart = m_trackingCurr;
    }
    
    // rotate the camera around the target position
    if( (m_rotatingStart != m_rotatingCurr) )
    {
        glm::vec4 p_world( persp->cameraPos(), 1.0f );
        glm::vec4 t_world( persp->cameraTarget(), 1.0f );
        
        glm::mat4 rot_view = rotation();
        
        glm::mat4 view_from_world = persp->viewMatrix();
        glm::mat4 world_from_view = glm::inverse( view_from_world );
        
        glm::vec4 p_view = view_from_world * p_world;
        glm::vec4 t_view = view_from_world * t_world;
        
        glm::vec4 rotatedCamPos_view = rot_view * ( p_view - t_view ) + t_view;
        glm::vec4 rotatedCamPos_world = world_from_view * rotatedCamPos_view;
        persp->cameraPos( rotatedCamPos_world.x,
                          rotatedCamPos_world.y,
                          rotatedCamPos_world.z );
    }
    return;
}

glm::vec3
spark::ArcBall
::getDirToSurfacePoint( int x, int y )
{
    float width = m_right - m_left;
    float height = m_top - m_bottom;
    // p is point on ball's surface in normalized screen coordinates [-0.5,0.5].
    glm::vec3 p( 0.5f - x/width, 0.5f - y/height, 0.0f );
    // y is reversed between screen and "natural" coords
    p.y *= -1.0f;
    float len2 = p.x*p.x + p.y*p.y;
    p.x *= 2.0f;
    p.y *= 2.0f;
    if( len2 <= 0.5f*0.5f )
    {
        // on the sphere
        p.z = std::sqrt( 1.0f - 4.0f*len2 );
    } else {
        // outside the sphere
        p.z = 0.0f;
    }
    return p;
}




