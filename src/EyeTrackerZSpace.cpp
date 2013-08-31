#include "EyeTracker.hpp"

#include "ZSpaceEyeTracker.hpp"

spark::EyeTrackerPtr
spark::createEyeTracker()
{
    return EyeTrackerPtr( new ZSpaceEyeTracker );
}
