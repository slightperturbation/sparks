#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Spark.hpp"
#include "ShaderInstance.hpp"
#include "Utilities.hpp"
#include "GuiEventSubscriber.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/optional.hpp>

#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>

namespace spark
{
    /// Projection manages state for the Model, View and Projection transforms
    /// and provides utility functions for setting camera and camera target.
    class Projection : public ShaderUniformHolder, public GuiEventSubscriber
    {
    public:
        Projection( void );
        virtual ~Projection() {}

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

        // Handle GUI Events
        virtual void resizeViewport( int left, int bottom,
            int width, int height ) override;

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
        void  fov( float degrees )                     { m_fov = degrees; }

        /// View from the current camera, possibly modified by the
        /// EyeView matrix.  See setEyeViewMatrix() and unsetEyeViewMatrix()
        virtual glm::mat4 viewMatrix( void ) const override;

        /// Can be explicitly set, see setProjectionMatrix()
        virtual glm::mat4 projectionMatrix( void ) const override;

        /// Explicitly set projection (e.g. perspective) matrix, overriding
        /// the current camera settings.  Can be reverted to fov-based 
        /// projection matrix using unsetProjectionMatrix()
        void setProjectionMatrix( const glm::mat4& mat );

        /// Undoes previously called setProjectionMatrix() to revert
        /// to a fov-defined projection matrix.
        void unsetProjectionMatrix( void );

        /// Set the currently used eye-view matrix which, if set, will 
        /// be multiplied by the standard view matrix to get the 
        /// final viewMatrix()
        /// Effectively, this transforms from camera/head-space to eye-space
        void setEyeViewMatrix( const glm::mat4& eyeMat );

        /// Undo the effect of setEyeViewMatrix(), returning to "monoscopic"
        /// view mode.
        void unsetEyeViewMatrix( void );
    protected:
        /// return the computed view matrix based on the camera pos/target/up
        virtual glm::mat4 cameraViewMatrix( void ) const;
    protected:
        glm::vec3 m_cameraPos;
        glm::vec3 m_cameraTarget;
        glm::vec3 m_cameraUp;
        float m_fov;
        glm::mat4 m_currEyeView;
        boost::optional< glm::mat4 > m_projMat;
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