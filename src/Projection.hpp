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
      m_cameraPos( 0.0f, 0.0f, -1.0f ),
      m_cameraTarget( 0, 0, 0 ),
      m_cameraUp( 0.0f, 1.0f, 0.0f ),
      m_fov( 80.0f ),
      m_aspectRatio( 800.0f/600.0f ),
      m_nearPlaneDist( 0.01f ),
      m_farPlaneDist( 20.0f )
    {
    }

    Projection( const Projection& that ) : 
      m_modelMatrix( that.m_modelMatrix ),
      m_cameraPos( that.m_cameraPos ),
      m_cameraTarget( that.m_cameraTarget ),
      m_cameraUp( that.m_cameraUp ),
      m_fov( that.m_fov ),
      m_aspectRatio( that.m_aspectRatio ),
      m_nearPlaneDist( that.m_nearPlaneDist ),
      m_farPlaneDist( that.m_farPlaneDist )
    {

    }

          glm::vec3& cameraUp( void )        { return m_cameraUp; }
    const glm::vec3& cameraUp( void ) const  { return m_cameraUp; }
          glm::vec3& cameraPos( void )       { return m_cameraPos; }
    const glm::vec3& cameraPos( void ) const { return m_cameraPos; }
          glm::vec3& cameraTarget( void )       { return m_cameraTarget; }
    const glm::vec3& cameraTarget( void ) const { return m_cameraTarget; }

    void setCameraPos( const glm::vec3& pos ) { m_cameraPos = pos; }
    void setCameraTarget( const glm::vec3& target ) { m_cameraTarget = target; }
    void setModelMatrix( const glm::mat4& modelMatrix ) { m_modelMatrix = modelMatrix; }
    void setAspectRatio( float ratio ) { m_aspectRatio = ratio; }
    
    const glm::mat4& modelMatrix( void ) const { return m_modelMatrix; }
    glm::mat4&       modelMatrix( void )       { return m_modelMatrix; }

    glm::mat4 getModelViewProjMatrix( void ) const 
    {
        return getViewProjMatrix() * m_modelMatrix;
    }

    glm::mat4 getViewProjMatrix( void ) const
    {
        return projectionMatrix() * viewMatrix();
    }
    
    glm::mat4 viewMatrix( void ) const
    {
        return glm::lookAt(m_cameraPos,
                           m_cameraTarget,
                           m_cameraUp );
    }
    
    glm::mat4 modelViewMatrix( void ) const 
    {
        return viewMatrix() * modelMatrix();
    }

    glm::mat4 projectionMatrix( void ) const
    {
        return glm::perspective( m_fov, m_aspectRatio, m_nearPlaneDist, m_farPlaneDist );
    }

private:
    glm::mat4 m_modelMatrix;
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraTarget;
    glm::vec3 m_cameraUp;
    float m_fov;
    float m_aspectRatio;
    float m_nearPlaneDist;
    float m_farPlaneDist;
};
typedef std::shared_ptr< Projection > PerspectivePtr;
typedef std::shared_ptr< const Projection > ConstPerspectivePtr;

#endif