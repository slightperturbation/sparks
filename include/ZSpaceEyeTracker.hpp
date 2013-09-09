#ifndef SPARK_ZSPACEEYETRACKER_HPP
#define SPARK_ZSPACEEYETRACKER_HPP

#include "EyeTracker.hpp"
#include "Updateable.hpp"
#include "ZSpaceSystem.hpp"

#include <memory>

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
        virtual void fixedUpdate( float dt ) override {}

        // ZSpace-specific Methods
        void setInterPupillaryDistance( float distInMeters );
    protected:
        virtual void implUpdatePerspective( PerspectiveProjectionPtr persp,
                                            PerspectiveEye eye ) override;
    private:
        // prevent copying
        ZSpaceEyeTracker( const ZSpaceEyeTracker& other ); // not implemented
        ZSpaceEyeTracker& operator=( const ZSpaceEyeTracker& other ); // not implemented

        int m_left;
        int m_bottom;
        int m_width;
        int m_height;
    };
}
#endif
