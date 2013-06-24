#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "SoftTestDeclarations.hpp"
#include "ShaderInstance.hpp"
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
#include <sstream>

namespace spark
{
    /// Projection manages state for the Model, View and Projection transforms
    /// and provides utility functions for setting camera and camera target.
    class Projection : public ShaderUniformHolder
    {
    public:
        Projection( void );
        virtual std::string name( void ) const = 0;
        /// Returns the direction from this "camera" to the given targetPoint.
        virtual glm::vec3 lookAtDirection( const glm::vec3& targetPoint ) const = 0;
        virtual glm::vec3 upDirection( void ) const = 0;

        float aspectRatio( void ) const { return m_aspectRatio; }
        void aspectRatio( float ratio ) { m_aspectRatio = ratio; }

        float nearPlaneDistance( void ) const { return m_nearPlaneDist; }
        void nearPlaneDistance( float nearDist ) { m_nearPlaneDist = nearDist; }

        float farPlaneDistance( void ) const { return m_farPlaneDist; }
        void farPlaneDistance( float farDist ) { m_farPlaneDist = farDist; }

        glm::mat4 getViewProjMatrix( void ) const
        {
            return projectionMatrix() * viewMatrix();
        }

        virtual glm::mat4 viewMatrix( void ) const = 0;
        virtual glm::mat4 projectionMatrix( void ) const = 0;
    protected:
        glm::mat4 m_modelMatrix;
        float m_aspectRatio;
        float m_nearPlaneDist;
        float m_farPlaneDist;
    };
    typedef spark::shared_ptr< Projection > ProjectionPtr;
    typedef spark::shared_ptr< const Projection > ConstProjectionPtr;

    ///////////////////////////////////////////////////////////////////////////
    /// Concrete projection for camera-style projection
    class PerspectiveProjection : public Projection
    {
    public:
        PerspectiveProjection( void );
        virtual std::string name( void ) const;
        virtual glm::vec3 lookAtDirection( const glm::vec3& targetPoint ) const override;
        virtual glm::vec3 upDirection( void ) const override;
        
        glm::vec3 cameraUp( void ) const               { return m_cameraUp; }
        void cameraUp( const glm::vec3& up )           { m_cameraUp = up; }
        void cameraUp( float x, float y, float z )     { m_cameraUp = glm::vec3(x,y,z); }

        glm::vec3 cameraPos( void ) const              { return m_cameraPos; }
        void cameraPos( const glm::vec3& pos )         { m_cameraPos = pos; }
        void cameraPos( float x, float y, float z )    { m_cameraPos = glm::vec3(x,y,z); }

        glm::vec3 cameraTarget( void ) const           { return m_cameraTarget; }
        void cameraTarget( const glm::vec3& target )   { m_cameraTarget = target; }
        void cameraTarget( float x, float y, float z ) { m_cameraTarget = glm::vec3(x,y,z); }

        float fov( void ) const                        { return m_fov; }
        void  fov( float arg )                         { m_fov = arg; }

        virtual glm::mat4 viewMatrix( void ) const;
        virtual glm::mat4 projectionMatrix( void ) const;
    protected:
        glm::vec3 m_cameraPos;
        glm::vec3 m_cameraTarget;
        glm::vec3 m_cameraUp;
        float m_fov;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Concrete orthogonal projection
    class OrthogonalProjection : public Projection
    {
    public:
        OrthogonalProjection();
        virtual std::string name( void ) const;
        virtual glm::vec3 lookAtDirection( const glm::vec3& targetPoint ) const override;
        virtual glm::vec3 upDirection( void ) const override;

        float left( void ) const { return m_left; }
        void left( float arg ) { m_left = arg; }
        float right( void ) const { return m_right; }
        void right( float arg ) { m_right = arg; }
        float bottom( void ) const { return m_bottom; }
        void bottom( float arg ) { m_bottom = arg; }
        float top( void ) const { return m_top; }
        void top( float arg ) { m_top = arg; }

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
} // end namespace spark
#endif