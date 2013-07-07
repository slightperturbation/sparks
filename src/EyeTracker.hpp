#include "Spark.hpp"
#include "GuiEventSubscriber.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace spark
{
    /// ABC for head/eye tracking system
    /// Repsonsible for conversion of motion in camera updates
    class EyeTracker : public GuiEventSubscriber
    {
    public:
        virtual ~EyeTracker() {}
        
        /// Modify the given perspective to reflect recent input.
        virtual void updatePerspective( PerspectiveProjectionPtr persp ) = 0;

        /// Update the viewport
        virtual void resizeViewport( int left, int bottom,
                                     int right, int top ) = 0;
    };
    typedef spark::shared_ptr< EyeTracker > EyeTrackerPtr;
    
    /// Concrete EyeTracker that gets data from a UDP connection.
    /// Responsible for turning normalized camera coords ([0-1],[0-1])
    /// into changes to the PerspectiveProjection during updatePerspective
    class NetworkEyeTracker : public EyeTracker
    {
        /// Helper class
        /// Server listens on a UDP port for eye pos updates
        /// Thead-safe
        class EyeTrackerServer
        {
        public:
            EyeTrackerServer(boost::asio::io_service& io_service, short port)
            : socket_(io_service,
                      boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),
                                                     port)),
              m_x( 0.5f ), m_y( 0.5f )
            {
                socket_.async_receive_from(
                    boost::asio::buffer(data_, max_length),
                    sender_endpoint_,
                    boost::bind(&EyeTrackerServer::handle_receive_from,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred) );
            }

            void handle_receive_from(const boost::system::error_code& error,
                                     size_t bytes_recvd)
            {
                if (!error && bytes_recvd > 0)
                {
                    data_[bytes_recvd+1] = '\0';
                    std::istringstream iss( data_ );
                    iss >> m_x;
                    iss >> m_y;
                }
                boost::this_thread::interruption_point( );
                socket_.async_receive_from(
                    boost::asio::buffer(data_, max_length),
                    sender_endpoint_,
                    boost::bind(&EyeTrackerServer::handle_receive_from,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred) );
            }

            void getEyePos( float& x, float& y ) const
            {
                x = m_x;  y = m_y;
            }
            private:
                boost::asio::ip::udp::socket socket_;
                boost::asio::ip::udp::endpoint sender_endpoint_;
                enum { max_length = 1024 };
                char data_[max_length];
                float m_x;
                float m_y;
        };
    public:
        NetworkEyeTracker( short listeningUdpPort = 5005 /* display size & viewport pos */ );
        virtual ~NetworkEyeTracker();
    
        /// Modify the given perspective to reflect recent input.
        virtual void updatePerspective( PerspectiveProjectionPtr persp ) override;
        
        /// Update the viewport
        virtual void resizeViewport( int left, int bottom,
                                    int right, int top ) override;
    private:
        std::unique_ptr< EyeTrackerServer > m_server;
        std::unique_ptr< boost::thread > m_listenerThread;
        boost::asio::io_service m_ioService;
        float m_prevX;
        float m_prevY;
    };
}
