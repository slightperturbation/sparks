#include "ZSpaceSystem.hpp"

#include "ZSpace/Common/Math/MathConverterGl.h"
#include "ZSpace/Common/Math/Matrix4.h"
#include "ZSpace/Common/Math/Vector3.h"
#include "ZSpace/Common/System/DisplayInfo.h"

#include "ZSpace/Stereo/StereoFrustum.h"
#include "ZSpace/Stereo/StereoLeftRightDetect.h"
#include "ZSpace/Stereo/StereoViewport.h"
#include "ZSpace/Stereo/StereoWindow.h"

#include "ZSpace/Tracker/TrackerSystem.h"
#include "ZSpace/Tracker/TrackerTarget.h"
#include "ZSpace/Tracker/ITrackerVibrateCapability.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

spark::ZSpaceSystem
    ::ZSpaceSystem( void )
    : m_stereoFrustum( nullptr )
{
    // NOTE -- not managed by this program, seems 
    // zSpace objects are managed?
    // TODO - clarify the ownership of zSpace objects (and put them in a unique_ptr?)
    m_stereoWindow = new zspace::stereo::StereoWindow();
    m_stereoViewport = new zspace::stereo::StereoViewport();
    m_stereoWindow->addStereoViewport( m_stereoViewport );
    m_stereoViewport->setUsingWindowSize( true ); // request to handle resize
    m_stereoFrustum = m_stereoViewport->getStereoFrustum();

    zspace::stereo::StereoLeftRightDetect::initialize( m_stereoWindow, 
        zspace::stereo::StereoLeftRightDetect::WINDOW_TYPE_GL );

    m_trackerSystem = new zspace::tracker::TrackerSystem();

    // get the "primary" (aka only) stylus
    zspace::tracker::TrackerTarget* primaryTarget 
        = m_trackerSystem->getDefaultTrackerTarget( zspace::tracker::TrackerTarget::TYPE_PRIMARY );
    if( primaryTarget )
    {
        // Get the vibration capability from the primary target, if the vibration capability exists.
        m_vibrator = static_cast<zspace::tracker::ITrackerVibrateCapability*>(
            primaryTarget->getCapability( zspace::tracker::ITrackerCapability::TYPE_VIBRATE ) );
        m_vibrator->setEnabled( true );
    }
    else
    {
        m_vibrator = nullptr;
    }
}

spark::ZSpaceSystem
    ::~ZSpaceSystem()
{
    zspace::stereo::StereoLeftRightDetect::shutdown();
    if( m_stereoWindow )
    {
        m_stereoWindow->removeReference();
    }
    if( m_stereoViewport )
    {
        m_stereoViewport->removeReference();
    }
    if( m_trackerSystem )
    {
        m_trackerSystem->removeReference();
    }
}

void 
spark::ZSpaceSystem
::convertPoseToWorldSpace( const zspace::common::Matrix4& pose, 
                           const glm::mat4& modelViewMatrix, 
                           zspace::common::Matrix4& worldPose )
{
    zspace::common::Matrix4 cameraLocalToWorld   = zspace::common::Matrix4::IDENTITY();
    zspace::common::Matrix4 trackerToCameraSpace = zspace::common::Matrix4::IDENTITY();
    zspace::common::Vector3 viewportOffset       = zspace::common::Vector3::ZERO();

    // Convert the OpenGl model-view matrix to the zSpace Matrix4 format and
    // invert it to obtain the non-stereo camera's matrix.
    zspace::common::MathConverterGl::convertMatrixGlToMatrix4(glm::value_ptr( modelViewMatrix), cameraLocalToWorld);
    cameraLocalToWorld = cameraLocalToWorld.inverse();

    // Grab the display that the zSpace StereoWindow is currently on in order to 
    // calculate the tracker-to-camera space transform as well as the viewport offset.
    // Both the tracker-to-camera space transform and viewport offset are required
    // for transforming a tracker space pose into the application's world space.
    const zspace::common::DisplayInfo::Display* display = m_stereoWindow->getCurrentDisplay();

    if (display)
    {
        trackerToCameraSpace = zspace::common::DisplayInfo::getTrackerToCameraSpaceTransform(display);
        viewportOffset       = zspace::common::DisplayInfo::getViewportOffset(display,
            m_stereoWindow->getX(),
            m_stereoWindow->getY(),
            m_stereoWindow->getWidth(),
            m_stereoWindow->getHeight());
    }

    worldPose = cameraLocalToWorld * trackerToCameraSpace * zspace::common::Matrix4::getTrans(-viewportOffset) * pose;
}