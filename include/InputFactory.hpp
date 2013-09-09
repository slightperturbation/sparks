#ifndef SPARK_INPUTFACTORY_HPP
#define SPARK_INPUTFACTORY_HPP

#include <memory>

namespace spark
{
    class KeyboardInputDevice;
    class InputDevice;
    
    /// Abstract Factory defining the interface for creating devices.
    class InputFactory
    {
    public:
        virtual std::unique_ptr<KeyboardInputDevice> createKeyboard( void ) const = 0;
        virtual std::unique_ptr<InputDevice> createDevice( int index ) const = 0;
    };
}
#endif
