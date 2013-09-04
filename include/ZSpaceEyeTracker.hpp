#ifndef SPARK_ZSPACEEYETRACKER_HPP
#define SPARK_ZSPACEEYETRACKER_HPP

#include "EyeTracker.hpp"
#include "Updateable.hpp"

#include <memory>

// forward Decls for zSpace objects
namespace zspace { 
    namespace stereo {
        class StereoWindow;
        class StereoViewport;
        class StereoFrustum;
    }
    namespace tracker {
        class TrackerSystem;
    }
}

namespace spark
{
    /// Concrete class that updates the camera perspective to follow the
    /// user's eyes.
    class ZSpaceEyeTracker : public EyeTracker
    {
    public:
        ZSpaceEyeTracker();
        virtual ~ZSpaceEyeTracker();

        virtual void resizeViewport( int left, int bottom,
            int right, int top ) override;

        // From Updateable
        virtual void update( float dt ) override;
        virtual void fixedUpdate( float dt ) override;

        // ZSpace-specific Methods
        void setInterPupillaryDistance( float distInMeters );
    protected:
        virtual void implUpdatePerspective( PerspectiveProjectionPtr persp,
            PerspectiveEye eye ) override;
    private:
        int m_left;
        int m_bottom;
        int m_width;
        int m_height;
        std::unique_ptr< zspace::stereo::StereoWindow > m_stereoWindow;
        std::unique_ptr< zspace::stereo::StereoViewport > m_stereoViewport;
        
        /// Owned by m_stereoViewport
        zspace::stereo::StereoFrustum* m_stereoFrustum;
        std::unique_ptr< zspace::tracker::TrackerSystem > m_trackerSystem;
    };
}
#endif
