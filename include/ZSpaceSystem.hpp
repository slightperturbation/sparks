#ifndef SPARK_ZSPACESYSTEM_HPP
#define SPARK_ZSPACESYSTEM_HPP

#include <glm/glm.hpp>


// forward decls for zSpace objects
namespace zspace { 
    namespace stereo {
        class StereoWindow;
        class StereoViewport;
        class StereoFrustum;
    }
    namespace tracker {
        class TrackerSystem;
    }
    namespace common {
        class Matrix4;
    }
}

namespace spark
{
    class ZSpaceEyeTracker;
    class ZSpaceInputDevice;

    class ZSpaceSystem
    {
        friend class ZSpaceEyeTracker;
        friend class ZSpaceInputDevice;

        void convertPoseToWorldSpace( const zspace::common::Matrix4& pose, 
                                      const glm::mat4& modelViewMatrix, 
                                      zspace::common::Matrix4& worldPose);
    private:
        static ZSpaceSystem& get( void )
        {
            static ZSpaceSystem system;
            return system;
        }
        ZSpaceSystem();
        ~ZSpaceSystem();
        ZSpaceSystem( const ZSpaceSystem& ); // Disabled
        ZSpaceSystem& operator=( const ZSpaceSystem& ); // Disabled

        zspace::stereo::StereoWindow* m_stereoWindow;
        zspace::stereo::StereoViewport* m_stereoViewport;

        /// Owned by m_stereoViewport
        zspace::stereo::StereoFrustum* m_stereoFrustum;
        zspace::tracker::TrackerSystem* m_trackerSystem; //< TODO -- ownership is unclear!

        /// The "monoscopic" perspective
        glm::mat4 m_modelViewMatrix;
    };
}

#endif
