#include "ZSpaceEyeTracker.hpp"
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
: m_left( 0 ), m_bottom( 0 ), m_width( 0 ), m_height( 0 ),
  m_stereoFrustum( nullptr )
{
    m_stereoWindow.reset( new zspace::stereo::StereoWindow() );
    m_stereoViewport.reset( new zspace::stereo::StereoViewport() );
    m_stereoWindow->addStereoViewport( m_stereoViewport.get() );
    m_stereoViewport->setUsingWindowSize( true ); // request to handle resize
    m_stereoFrustum = m_stereoViewport->getStereoFrustum();

    zspace::stereo::StereoLeftRightDetect::initialize( m_stereoWindow.get(), 
        zspace::stereo::StereoLeftRightDetect::WINDOW_TYPE_GL );

    m_trackerSystem.reset( new zspace::tracker::TrackerSystem() );
}

spark::ZSpaceEyeTracker
::~ZSpaceEyeTracker()
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
spark::ZSpaceEyeTracker
::setInterPupillaryDistance( float distInMeters )
{
    if( m_stereoFrustum )
    {
        m_stereoFrustum->setInterPupillaryDistance( distInMeters );
    }
}

void 
spark::ZSpaceEyeTracker
::updatePerspective( PerspectiveProjectionPtr persp,
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
        return;
    }
    //m_stereoFrustum->setFieldOfViewScale( m_fov );

    // Set view matrix
    GLfloat matrixGl[16];
    GLfloat monoModelViewGl[16];

    // Get the view matrix from the zSpace StereoFrustum for a specified eye
    // and convert it into OpenGl matrix format.
    zspace::common::Matrix4 viewMatrix;
    m_stereoFrustum->getViewMatrix( zspaceEye, viewMatrix );
    zspace::common::MathConverterGl::convertMatrix4ToMatrixGl( viewMatrix, matrixGl );
    persp->setViewMatrix( glm::make_mat4( matrixGl ) );

    // set projection matrix
    // Get the projection matrix from the zSpace StereoFrustum for a specified eye
    // and convert it into OpenGl matrix format.
    zspace::common::Matrix4 projectionMatrix;
    m_stereoFrustum->getProjectionMatrix( zspaceEye, projectionMatrix );
    zspace::common::MathConverterGl::convertMatrix4ToMatrixGl( projectionMatrix, matrixGl );
    persp->setProjectionMatrix( glm::make_mat4( matrixGl ) );
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
    if( !m_stereoWindow || !m_trackerSystem || !m_stereoViewport )
    {
        return;
    }
    // Update zSpace Left/Right Frame Detection.
    zspace::stereo::StereoLeftRightDetect::update();

    if(    m_stereoWindow->getX() != m_left 
        || m_stereoWindow->getY() != m_bottom )
    {
        //m_stereoWindow->move( m_left, m_bottom );
    }
    if(    m_stereoWindow->getWidth() != m_width 
        || m_stereoWindow->getHeight() != m_height )
    {
        //m_stereoWindow->resize( m_width, m_height );
    }

    // Take the tracking data and pass it into the m_stereoFrustum
    // for use when rendering
    m_trackerSystem->captureTargets();

    zspace::tracker::TrackerTarget* headTarget 
        = m_trackerSystem->getDefaultTrackerTarget( zspace::tracker::TrackerTarget::TYPE_HEAD );
    if( headTarget )
    {
        zspace::common::Matrix4 headPose = zspace::common::Matrix4::IDENTITY();
        headTarget->getPose( headPose );
        m_stereoFrustum->setHeadPose( headPose );
    }
}

void 
spark::ZSpaceEyeTracker
::fixedUpdate( float dt ) 
{
    // Noop
}
