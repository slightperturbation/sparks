#include "ZSpaceEyeTracker.hpp"
#include "ZSpaceSystem.hpp"
#include "Projection.hpp"


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

spark::ZSpaceEyeTracker
::ZSpaceEyeTracker()
: m_left( 0 ), m_bottom( 0 ), m_width( 0 ), m_height( 0 )
{
}

spark::ZSpaceEyeTracker
::~ZSpaceEyeTracker()
{
}

void 
spark::ZSpaceEyeTracker
::setInterPupillaryDistance( float distInMeters )
{
    if( ZSpaceSystem::get().m_stereoFrustum )
    {
        ZSpaceSystem::get().m_stereoFrustum
            ->setInterPupillaryDistance( distInMeters );
    }
}

void 
spark::ZSpaceEyeTracker
::implUpdatePerspective( PerspectiveProjectionPtr persp,
                         PerspectiveEye eye ) 
{
    zspace::stereo::StereoFrustum::StereoEye zspaceEye;
    switch( eye )
    {
    case EyeTracker::leftEye:
        zspaceEye = zspace::stereo::StereoFrustum::STEREO_EYE_LEFT;
        break;
    case EyeTracker::rightEye:
        zspaceEye = zspace::stereo::StereoFrustum::STEREO_EYE_RIGHT;
        break;
    case EyeTracker::monoEye:
        zspaceEye = zspace::stereo::StereoFrustum::STEREO_EYE_CENTER;
        break;
    default:
        LOG_WARN(g_log) << "Unexpected eye specified?";
        return;
    }
    update(0);
    ZSpaceSystem::get().m_stereoFrustum->setNearClip( 0.05 /*meters*/ );

    // store the monoscopic "modelview" matrix
    ZSpaceSystem::get().m_modelViewMatrix = persp->viewMatrix();

    // Set view matrix
    // Get the view matrix from the zSpace StereoFrustum for a specified eye
    // and convert it into OpenGl matrix format.
    // 
    // The frustum's view matrix contains only the *offset* for the eye
    // so needs to be multiplied by the current 
    {
        GLfloat matrixGl[16];
        zspace::common::Matrix4 viewMatrix;
        ZSpaceSystem::get().m_stereoFrustum->getViewMatrix( zspaceEye, viewMatrix );
        zspace::common::MathConverterGl::convertMatrix4ToMatrixGl( viewMatrix, matrixGl );
        persp->setEyeViewMatrix( glm::make_mat4( matrixGl) );
    }

    // set projection matrix
    // Get the projection matrix from the zSpace StereoFrustum for a specified eye
    // and convert it into OpenGl matrix format.
    {
        GLfloat projectionMatrixGl[16];
        zspace::common::Matrix4 projectionMatrix;
        ZSpaceSystem::get().m_stereoFrustum
            ->getProjectionMatrix( zspaceEye, projectionMatrix );
        zspace::common::MathConverterGl::convertMatrix4ToMatrixGl( projectionMatrix, projectionMatrixGl );
        persp->setProjectionMatrix( glm::make_mat4( projectionMatrixGl ) );
    }
}

void 
spark::ZSpaceEyeTracker
::resizeViewport( int left, int bottom,
                  int right, int top ) 
{
    m_left = left;
    m_bottom = bottom;
    m_width = right - left;
    m_height = top - bottom;
}

void 
spark::ZSpaceEyeTracker
::update( float dt ) 
{
    ZSpaceSystem& sys = ZSpaceSystem::get();
    if( !sys.m_stereoWindow || !sys.m_trackerSystem || !sys.m_stereoViewport )
    {
        return;
    }
    // Update zSpace Left/Right Frame Detection.
    zspace::stereo::StereoLeftRightDetect::update();

    if(    sys.m_stereoWindow->getX() != m_left 
        || sys.m_stereoWindow->getY() != m_bottom )
    {
        sys.m_stereoWindow->move( m_left, m_bottom );
    }
    if(    sys.m_stereoWindow->getWidth() != m_width 
        || sys.m_stereoWindow->getHeight() != m_height )
    {
        sys.m_stereoWindow->resize( m_width, m_height );
    }
    GL_CHECK( glViewport( 0, 0, (GLsizei)m_width, (GLsizei)m_height ) );

    // Take the tracking data and pass it into the m_stereoFrustum
    // for use when rendering
    sys.m_trackerSystem->captureTargets();

    zspace::tracker::TrackerTarget* headTarget 
        = sys.m_trackerSystem->getDefaultTrackerTarget( zspace::tracker::TrackerTarget::TYPE_HEAD );
    if( headTarget )
    {
        zspace::common::Matrix4 headPose = zspace::common::Matrix4::IDENTITY();
        headTarget->getPose( headPose );
        sys.m_stereoFrustum->setHeadPose( headPose );
    }
}
