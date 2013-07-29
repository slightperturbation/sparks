#include "EyeTracker.hpp"
#include "Projection.hpp"

//#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/signal_set.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

spark::NetworkEyeTracker
::NetworkEyeTracker( short listeningUdpPort )
: m_prevX( 0.5f ), m_prevY( 0.5f )
{
    m_work.reset( new boost::asio::io_service::work( m_ioService ) );
    try
    {
        // Register signal handlers so that the daemon may be shut down. You may
        // also want to register for other signals.
#ifdef WIN32
        // ???? How to dispatch signals on windows?
        // the code below seems to be catching signals quickly 
        // after startup.
#else
        boost::asio::signal_set signals( m_ioService, SIGINT, SIGTERM );
        signals.async_wait(
            boost::bind( &boost::asio::io_service::stop, &m_ioService ) );
#endif        
        m_server.reset( new EyeTrackerServer( m_ioService,
                                              listeningUdpPort ) );
        // Dispatch m_ioService.run() on its own thread
        // Which runs the EyeTrackerServer to listen for
        // UDP updates.
        m_listenerThread.reset( new boost::thread(
            boost::bind( &boost::asio::io_service::run,
                         &m_ioService) ) );
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

spark::NetworkEyeTracker
::~NetworkEyeTracker()
{
    try
    {
        // clear work holder to allow service to exit
        m_work.reset();
        m_ioService.stop();
        m_listenerThread->interrupt();
    }
    catch( ... )
    {
        LOG_DEBUG(g_log) << "NetworkEyeTracker no longer listening.";
    }
}

void
spark::NetworkEyeTracker
::updatePerspective( PerspectiveProjectionPtr persp )
{
    float x, y;
    m_server->getEyePos( x, y );
    float dx = x - m_prevX; m_prevX = x;
    float dy = y - m_prevY; m_prevY = y;
    if( dx != 0.0 || dy != 0.0 )
    {
        LOG_DEBUG(g_log) << "NetworkEyeTracker pos delta: "
                         << dx << ", " << dy ;
    }
    // move camera slightly
    // +y is up relative to viewer
    // +x is to the right from the perspective of the viewer

    
    // TODO - unit conversions from face-camera to scene
    // scale factors are arbitrary, but should depend on the size of the
    // scene, screen and the camera's FOV
    float cameraFOV_rad = 1.5f;
    float eyeDist = 1.5f;
    
    glm::vec3 up = persp->cameraUp();
    glm::vec3 in = persp->cameraPos() - persp->cameraTarget();
    glm::vec3 right = glm::normalize( glm::cross( in, up ) );
    
    float xOffset = cameraFOV_rad * eyeDist * dx;
    float yOffset = cameraFOV_rad * eyeDist * dy;
    glm::vec3 offset = xOffset * right;
    offset -= yOffset * up; // flip y
    persp->cameraPos( persp->cameraPos() + offset );
}

void
spark::NetworkEyeTracker
::resizeViewport( int left, int bottom,
                  int right, int top )
{
    
}

