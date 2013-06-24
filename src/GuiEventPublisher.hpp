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
    private:
        // Note-- weak_ptr's operator<() was removed from C++0x to C++11.
        // To use weak_ptr in a set in C++11, specify the owner_less comparator.
        std::set< GuiEventSubscriberWeakPtr, 
                  std::owner_less< GuiEventSubscriberWeakPtr > > m_subscribers;
    };
    typedef spark::shared_ptr< GuiEventPublisher > GuiEventPublisherPtr;
}


#endif

