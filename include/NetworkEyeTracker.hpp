#ifndef SPARK_NETWORKEYETRACKER_HPP
#define SPARK_NETWORKEYETRACKER_HPP

#include "EyeTracker.hpp"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace spark
{
    /// Concrete EyeTracker that gets data from a UDP connection.
    /// Responsible for turning normalized camera coords ([0-1],[0-1])
    /// into changes to the PerspectiveProjection during updatePerspective
    class NetworkEyeTracker : public EyeTracker
    {
        /// Helper class
        /// Server listens on a UDP port for eye pos updates
        /// Thread-safe
        class EyeTrackerServer
        {
        public:
            EyeTrackerServer(boost::asio::io_service& io_service, short port)
                : socket_(io_service,
                boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),
                //boost::asio::ip::address_v4::any(),
                port)),
                m_x( 0.5f ), m_y( 0.5f ),
                m_isListening( true )
            {
                try
                {
                    socket_.set_option(boost::asio::socket_base::reuse_address(true));
                    socket_.async_receive_from(
                        boost::asio::buffer(data_, max_length),
                        sender_endpoint_,
                        boost::bind(&EyeTrackerServer::handle_receive_from,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred) );
                }
                catch( const std::exception& e )
                {
                    LOG_WARN(g_log) << "Socket error: " << e.what();
                }
            }

            void handle_receive_from(const boost::system::error_code& error,
                size_t bytes_recvd)
            {
                if (!error && bytes_recvd > 0)
                {
                    std::string data;
                    for( int i = 0; i < bytes_recvd; ++i ) data += data_[i];
                    std::istringstream iss( data );
                    boost::lock_guard<boost::mutex> guard( lock_ );
                    iss >> m_x;
                    iss >> m_y;
                }
                if( error )
                {
                    LOG_ERROR(g_log) << "Error on recving network packet: "
                        << error << " - " << error.message() << " - " << error.category().name();
                }
                if( m_isListening )
                {
                    boost::this_thread::interruption_point( );
                    try
                    {
                        socket_.async_receive_from(
                            boost::asio::buffer(data_, max_length),
                            sender_endpoint_,
                            boost::bind(&EyeTrackerServer::handle_receive_from,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred) );
                    }
                    catch( const std::exception& e )
                    {
                        LOG_WARN(g_log) << "Socket error: " << e.what();
                    }

                }
            }

            void getEyePos( float& x, float& y ) const
            {
                boost::lock_guard<boost::mutex> guard( lock_ );
                x = m_x;  y = m_y;
            }
            void stop( void ) { m_isListening = false; }
        private:
            mutable boost::mutex lock_;
            boost::asio::ip::udp::socket socket_;
            boost::asio::ip::udp::endpoint sender_endpoint_;
            enum { max_length = 1024 };
            char data_[max_length];
            float m_x;
            float m_y;
            bool m_isListening; 
        };
    public:
        NetworkEyeTracker( short listeningUdpPort = 5005 /* display size & viewport pos */ );
        virtual ~NetworkEyeTracker();

        /// Modify the given perspective to reflect recent input.
        virtual void updatePerspective( PerspectiveProjectionPtr persp,
                                        PerspectiveEye eye = monoEye ) override;

        /// Update the viewport
        virtual void resizeViewport( int left, int bottom,
            int right, int top ) override;
    private:
        std::unique_ptr< EyeTrackerServer > m_server;
        std::unique_ptr< boost::thread > m_listenerThread;
        boost::asio::io_service m_ioService;
        std::unique_ptr< boost::asio::io_service::work > m_work;
        float m_prevX;
        float m_prevY;
    };

}


#endif
