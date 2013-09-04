#ifndef SPARK_GUI_EVENT_PUBLISHER_HPP
#define SPARK_GUI_EVENT_PUBLISHER_HPP

#include "GuiEventSubscriber.hpp"

#include <set>

namespace spark
{
    /// Provides GuiEventSubscribers to receive notification of GUI events.
    /// Note that GuiEventPublisher only maintains weak_ptrs, so
    /// lifetime of subscribers is not affected.
    class GuiEventPublisher
    {
    public:
        GuiEventPublisher()  
        : m_left( 0 ), m_bottom( 0 ), m_width( 1024 ), m_height( 768 )
        { }

        /// The given aSubscriber will receive notifications of GUI events.
        /// The publisher holds only a weak pointer to subscriber, so
        /// the lifetime of the subscriber is not bound this publisher.
        void subscribe( GuiEventSubscriberPtr aSubscriber )
        {
            m_subscribers.insert( aSubscriber );
        }
        /// After called unsubscribe, aSubscriber will no longer receive
        /// any event notifications.
        void unsubscribe( GuiEventSubscriberPtr aSubscriber )
        {
            m_subscribers.erase( aSubscriber );
        }

        /// Notify subscribers that the window/viewport now has
        /// size width, height.
        void resizeViewport( int left, int bottom,
                             int width, int height )
        {
            if(    left   == m_left 
                && bottom == m_bottom
                && width  == m_width
                && height == m_height ) 
            {
                return;
            }
            m_left = left; m_bottom = bottom;
            m_width = width; m_height = height;
            for( auto i = m_subscribers.begin();
                 i != m_subscribers.end();
                 ++i )
            {
                spark::shared_ptr<GuiEventSubscriber> p = (*i).lock();
                if( p )
                {
                    p->resizeViewport( left, bottom, width, height );
                }
            }
        }

        void moveWindow( int xpos, int ypos )
        {
            resizeViewport( xpos, ypos, m_width, m_height );
        }

        int left( void ) const { return m_left; }
        int bottom( void ) const { return m_bottom; }
        int width( void ) const { return m_width; }
        int height( void ) const { return m_height; }
    private:
        int m_left;
        int m_bottom;
        int m_width;
        int m_height;

#ifdef USE_BOOST_SHARED_PTR
        std::set< GuiEventSubscriberWeakPtr > m_subscribers;
#else
        // Note-- weak_ptr's operator<() was removed from C++0x to C++11.
        // To use weak_ptr in a set in C++11, specify the owner_less comparator.
        std::set< GuiEventSubscriberWeakPtr,
                  std::owner_less< GuiEventSubscriberWeakPtr > > m_subscribers;
#endif
    };
    typedef spark::shared_ptr< GuiEventPublisher > GuiEventPublisherPtr;
}


#endif

