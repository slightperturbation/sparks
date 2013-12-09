#ifndef SPARK_EYETRACKER_HPP
#define SPARK_EYETRACKER_HPP

#include "Spark.hpp"
#include "Updateable.hpp"
#include "GuiEventSubscriber.hpp"

namespace spark
{
    /// Interface for head/eye tracking system
    /// Responsible for conversion of motion in camera updates
    class EyeTracker : public GuiEventSubscriber, public Updateable
    {
    public:
        virtual ~EyeTracker() {}

        typedef enum { monoEye, leftEye, rightEye } PerspectiveEye;

        /// Modify the given perspective to reflect the position
        /// of the specified eye.  Use "monoEye" (the default) to
        /// get the position of the head or face.
        void updatePerspective( PerspectiveProjectionPtr persp,
            PerspectiveEye eye = monoEye )
        {
            implUpdatePerspective( persp, eye );
        }

        /// Update the viewport using the newly calculated positions of the
        /// the tracked eyes.
        virtual void resizeViewport( int left, int bottom,
                                     int right, int top ) = 0;

        /// Allows the EyeTracker to update it's estimate of the user's Eyes.
        virtual void update( double dt ) override = 0;
    protected:
        virtual void implUpdatePerspective( PerspectiveProjectionPtr persp, 
            PerspectiveEye eye ) = 0;
    };
    typedef spark::shared_ptr< EyeTracker > EyeTrackerPtr;
    
    /// Create an eyetracker based on available hardware
    EyeTrackerPtr createEyeTracker( void );
}

#endif
