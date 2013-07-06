#include "EyeTracker.hpp"


#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

spark::NetworkEyeTracker
::NetworkEyeTracker()
{
    try
    {
        m_server.reset( new EyeTrackerServer( m_ioService, 5005 ) );
        m_ioService.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        throw;
    }
}

void
spark::NetworkEyeTracker
::updatePerspective( PerspectiveProjectionPtr persp )
{
    float x, y;
    m_server->getEyePos( x, y );
}

void
spark::NetworkEyeTracker
::resizeViewport( int left, int bottom,
                  int right, int top )
{
    
}