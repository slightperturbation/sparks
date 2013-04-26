#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "SoftTestDeclarations.hpp"
#include "Shader.hpp"
#include "Utilities.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <memory>
#include <iostream>
#include <string>


/// Simple Point lights only for now
class Light
{
public:
    Light()
        : m_intensity( 1.0 ),
          m_color(  1.0f, 0.2f, 0.2f ),
          m_position( 0.5f, 0.5f, 1.0f )
    {
        m_position = glm::normalize( m_position );
    }
    float m_intensity;
    glm::vec3 m_color;
    glm::vec3 m_position;
};

/// Projection manages state for the Model, View and Projection transforms
/// and provides utility functions for setting camera and camera target.
class Projection : public ShaderUniformHolder
{
public:
    Projection() : 
      m_aspectRatio( 800.0f/600.0f ),
      m_nearPlaneDist( 0.01f ),
      m_farPlaneDist( 20.0f )
    {
    }

    float aspectRatio( void ) const { return m_aspectRatio; }
    void aspectRatio( float ratio ) { m_aspectRatio = ratio; }

    float nearPlaneDistance( void ) const { return m_nearPlaneDist; }
    void nearPlaneDistance( float nearDist ) { m_nearPlaneDist = nearDist; }

    float farPlaneDistance( void ) const { return m_farPlaneDist; }
    void farPlaneDistance( float farDist ) { m_farPlaneDist = farDist; }

    const glm::mat4& modelMatrix( void ) const { return m_modelMatrix; }
    void modelMatrix( const glm::mat4& modelMatrix ) { m_modelMatrix = modelMatrix; }

    glm::mat4 getModelViewProjMatrix( void ) const 
    {
        return getViewProjMatrix() * m_modelMatrix;
    }
    glm::mat4 getViewProjMatrix( void ) const
    {
        return projectionMatrix() * viewMatrix();
    }
    glm::mat4 modelViewMatrix( void ) const 
    {
        return viewMatrix() * modelMatrix();
    }

    virtual glm::mat4 viewMatrix( void ) const = 0;
    virtual glm::mat4 projectionMatrix( void ) const = 0;
protected:
    glm::mat4 m_modelMatrix;
    float m_aspectRatio;
    float m_nearPlaneDist;
    float m_farPlaneDist;
};
typedef std::shared_ptr< Projection > PerspectivePtr;
typedef std::shared_ptr< const Projection > ConstPerspectivePtr;

/// Concrete projection for camera-style projection
class PerspectiveProjection : public Projection
{
public:
    PerspectiveProjection( void )
    :   m_cameraPos( 0.0f, 0.0f, -1.0f ),
        m_cameraTarget( 0, 0, 0 ),
        m_cameraUp( 0.0f, 1.0f, 0.0f ),
        m_fov( 80.0f )
    { }
    glm::vec3 cameraUp( void ) const             { return m_cameraUp; }
    void cameraUp( const glm::vec3& up )         { m_cameraUp = up; }

    glm::vec3 cameraPos( void ) const            { return m_cameraPos; }
    void cameraPos( const glm::vec3& pos )       { m_cameraPos = pos; }

    glm::vec3 cameraTarget( void ) const         { return m_cameraTarget; }
    void cameraTarget( const glm::vec3& target ) { m_cameraTarget = target; }

    float fov( void ) const                      { return m_fov; }
    void  fov( float arg )                       { m_fov = arg; }

    virtual glm::mat4 viewMatrix( void ) const
    {
        return glm::lookAt(m_cameraPos,
            m_cameraTarget,
            m_cameraUp );
    }

    virtual glm::mat4 projectionMatrix( void ) const
    {
        return glm::perspective( m_fov, m_aspectRatio, m_nearPlaneDist, m_farPlaneDist );
    }
protected:
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraTarget;
    glm::vec3 m_cameraUp;
    float m_fov;
};


class OrthogonalProjection : public Projection
{
public:
    OrthogonalProjection()
    : m_left( 0 ),
      m_right( 1 ),
      m_bottom( 0 ),
      m_top( 1 )
    {
        m_nearPlaneDist = -10.1;
        m_farPlaneDist = 10.1;
    }
    float left( void ) const { return m_left; }
    void left( float arg ) { m_left = arg; }
    float right( void ) const { return m_right; }
    void right( float arg ) { m_right = arg; }
    float bottom( void ) const { return m_bottom; }
    void bottom( float arg ) { m_bottom = arg; }
    float top( void ) const { return m_top; }
    void top( float arg ) { m_top = arg; }
    float nearPlane( void ) const { return m_nearPlaneDist; }
    void nearPlane( float arg ) { m_nearPlaneDist = arg; }
    float farPlane( void ) const { return m_farPlaneDist; }
    void farPlane( float arg ) { m_farPlaneDist = arg; }

    virtual glm::mat4 viewMatrix( void ) const
    {
        return glm::mat4(); // default ctor provides identity matrix
    }
    virtual glm::mat4 projectionMatrix( void ) const
    {
        return glm::ortho( m_left, m_right, m_bottom, m_top, m_nearPlaneDist, m_farPlaneDist );
    }
protected:
    float m_left;
    float m_right;
    float m_bottom;
    float m_top;
};
#endif