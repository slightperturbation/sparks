#include "SoftTestDeclarations.hpp"
#include "GuiEventSubscriber.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace spark
{
    /// Updates a PerspectiveProjection based on mouse motions.
    /// See http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball
    class ArcBall : public GuiEventSubscriber
    {
    public:
        ArcBall( void );
        virtual ~ArcBall() {}
        /// Update the ArcBall's regions of interest
        virtual void resizeViewport( int left, int bottom,
                                     int right, int top ) override;
        /// x and y are in raw screen coordinates
        void onMouseMove( int x, int y );
        void onMouseRotationButton( int x, int y, bool isPressed );
        void onMouseDollyButton( int x, int y, bool isPressed );
        void onMouseTrackingButton( int x, int y, bool isPressed );
        /// Modify the given perspective to reflect recent input.
        void updatePerspective( PerspectiveProjectionPtr persp );
    private:
        /// Returns the rotation for the current mouse positions.
        glm::mat4 rotation( void );
        /// Convert screen point (x,y) to a direction on the unit sphere
        /// centered at the center of the screen (according to extents).
        glm::vec3 getDirToSurfacePoint( int x, int y );

        /// Extents
        int m_left;
        int m_right;
        int m_bottom;
        int m_top;
        
        bool m_isRotating;
        bool m_isDollying;
        bool m_isTracking;
        float m_dollySpeed;
        glm::ivec2 m_dollyStart;
        glm::ivec2 m_dollyCurr;
        glm::ivec2 m_trackingStart;
        glm::ivec2 m_trackingCurr;
        glm::ivec2 m_rotatingStart;
        glm::ivec2 m_rotatingCurr;
    };
    typedef std::shared_ptr< ArcBall > ArcBallPtr;
}