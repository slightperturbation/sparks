#ifndef SPARK_GUI_EVENT_SUBSCRIBER_HPP
#define SPARK_GUI_EVENT_SUBSCRIBER_HPP

#include <memory>

namespace spark
{
    // Abstract superclass for a receiver of events from the GUI.
    class GuiEventSubscriber 
    {
    public:
        virtual ~GuiEventSubscriber() {}
        virtual void resizeViewport( int left, int bottom,
                                     int width, int height ) {}
    };
    typedef std::shared_ptr< GuiEventSubscriber > GuiEventSubscriberPtr;
    typedef std::weak_ptr< GuiEventSubscriber > GuiEventSubscriberWeakPtr;
}
#endif
