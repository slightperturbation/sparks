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

        /// Modify the given perspective to reflect recent input.
        virtual void updatePerspective( PerspectiveProjectionPtr persp,
                                        PerspectiveEye eye = monoEye ) = 0;

        /// Update the viewport
        virtual void resizeViewport( int left, int bottom,
                                     int right, int top ) = 0;

        virtual void update( float dt ) override {}
        virtual void fixedUpdate( float dt ) override {}
    };
    typedef spark::shared_ptr< EyeTracker > EyeTrackerPtr;
    
    /// Create an eyetracker based on available hardware
    EyeTrackerPtr createEyeTracker( void );
}

#endif
