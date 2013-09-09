#include "EyeTracker.hpp"

#include "NetworkEyeTracker.hpp"

spark::EyeTrackerPtr
spark::createEyeTracker( void )
{
return EyeTrackerPtr( new NetworkEyeTracker() );
}
