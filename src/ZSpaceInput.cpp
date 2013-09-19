#include "Utilities.hpp" // for debug printing 

#include "ZSpaceInput.hpp"
#include "ZSpaceSystem.hpp"

#include "ZSpace/Common/Math/MathConverterGl.h"
#include "ZSpace/Common/Math/Matrix4.h"
#include "ZSpace/Common/Math/Vector3.h"
#include "ZSpace/Common/Math/Ray.h"

#include "ZSpace/Common/System/DisplayInfo.h"

#include "ZSpace/Tracker/ITrackerButtonCapability.h"
#include "ZSpace/Tracker/TrackerSystem.h"
#include "ZSpace/Tracker/TrackerTarget.h"

std::unique_ptr<spark::InputDevice> 
spark::ZSpaceInputFactory
::createDevice( int index ) const 
{
    std::unique_ptr<InputDevice> device( new ZSpaceInputDevice() );
    return std::move( device );
}

///////////////////////////////////////////////////////////////////////////
spark::ZSpaceInputDevice
::ZSpaceInputDevice()
{
    zspace::tracker::TrackerTarget* primaryTarget 
        = ZSpaceSystem::get().m_trackerSystem->getDefaultTrackerTarget(
        zspace::tracker::TrackerTarget::TYPE_PRIMARY);

    // Get the primary tracker target's button capability if it exists.
    zspace::tracker::ITrackerButtonCapability* trackerButtonCapability =
        static_cast<zspace::tracker::ITrackerButtonCapability*>(
        primaryTarget->getCapability( zspace::tracker::ITrackerCapability::TYPE_BUTTON ) );
    if( trackerButtonCapability )
    {
        unsigned int numButtons = trackerButtonCapability->getNumButtons();
        m_previousButtonStates.resize( numButtons ); // initialized to false
    }
}

spark::ZSpaceInputDevice
::~ZSpaceInputDevice()
{

}

void
spark::ZSpaceInputDevice
::update( double dt  )
{
    // Grab the latest 6DOF stylus pose (position and orientation) and transform it
    // into the scene's world space.  After the pose has been transformed to world space,
    // pass it off to the OpenGlScene so the stylus can be rendered correctly.
    // Note: "primaryTarget" refers to the primary hand's device.  Currently this is the
    // stylus, but could easily be swapped out for some other 6DOF track-able device.
    zspace::tracker::TrackerTarget* primaryTarget 
        = ZSpaceSystem::get().m_trackerSystem->getDefaultTrackerTarget(
        zspace::tracker::TrackerTarget::TYPE_PRIMARY);

    // Check to make sure the primary target is not NULL.  If the primary target is NULL,
    // this signifies that the default primary target cannot be found.
    if (primaryTarget)
    {
        zspace::common::Matrix4 primaryPose      = zspace::common::Matrix4::IDENTITY();
        zspace::common::Matrix4 primaryWorldPose = zspace::common::Matrix4::IDENTITY();

        primaryTarget->getPose(primaryPose);
        ZSpaceSystem::get().convertPoseToWorldSpace(primaryPose, ZSpaceSystem::get().m_modelViewMatrix, primaryWorldPose);

        // Convert the stylus world pose to OpenGl matrix format and apply it to
        // the virtual stylus's transform.
        GLfloat stylusTransformGl[16];
        zspace::common::MathConverterGl::convertMatrix4ToMatrixGl(
            primaryPose, stylusTransformGl);

        m_transform = glm::make_mat4( stylusTransformGl );
        
        //zspace::common::Vector3 position = primaryPose.getTrans();
        zspace::common::Vector3 position = primaryPose.getTrans();
        m_position = glm::vec3( position.x, position.y, position.z );

//         zspace::common::Vector3 direction(-primaryPose[0][2], -primaryPose[1][2], -primaryPose[2][2]);
//         zspace::common::Ray ray = zspace::common::Ray(position, direction);
    }
    // Get the primary tracker target's button capability if it exists.
    zspace::tracker::ITrackerButtonCapability* trackerButtonCapability =
        static_cast<zspace::tracker::ITrackerButtonCapability*>(
        primaryTarget->getCapability( zspace::tracker::ITrackerCapability::TYPE_BUTTON ) );
    if( trackerButtonCapability )
    {
        unsigned int numButtons = trackerButtonCapability->getNumButtons();
        assert( numButtons == m_previousButtonStates.size() );

        for( unsigned int buttonId = 0; buttonId < numButtons; ++buttonId )
        {
            // Check if the specified button is up or down.
            bool isButtonPressed = trackerButtonCapability->isButtonPressed(buttonId);

            // Detect any changes in state.
            if( isButtonPressed && !m_previousButtonStates[buttonId] )
            {
                std::cerr << "Button " << buttonId << ": Pressed\n" ;
            }
            else if( !isButtonPressed && m_previousButtonStates[buttonId] )
            {
                std::cerr << "Button " << buttonId << ": Released\n" ;
            }
            // Store off the current button state into the previous button states array
            // so that it can be referenced in the next frame.
            m_previousButtonStates[buttonId] = isButtonPressed;
        }
    }
}

glm::vec3 
spark::ZSpaceInputDevice
::getPosition( void ) const 
{
    return m_position;
}

glm::mat4 
spark::ZSpaceInputDevice
::getTransform( void ) const 
{
    return m_transform;
}

bool 
spark::ZSpaceInputDevice
::isButtonPressed( int buttonNumber ) const 
{
    return m_previousButtonStates[buttonNumber];
}

glm::vec3 
spark::ZSpaceInputDevice
::getPositionRange( void ) const 
{
    return glm::vec3();
}

