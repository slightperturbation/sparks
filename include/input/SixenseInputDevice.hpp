#ifndef SPARK_SIXENSEINPUTDEVICE_HPP
#define SPARK_SIXENSEINPUTDEVICE_HPP

#include "InputDevice.hpp"

#include <glm/glm.hpp>

namespace spark
{
    /// Support a single Hydra magnetic tracking device.
    class SixenseInputDevice : public InputDevice
    {
    public:
        SixenseInputDevice() {}
        virtual ~SixenseInputDevice() {}
        virtual glm::vec3 getPosition( void ) const override;
        virtual glm::vec2 getScreenPosition( void ) const;
        virtual glm::mat4 getTransform( void ) const override;
        virtual bool isButtonPressed( int buttonNumber ) const override;
    };
}
#endif
