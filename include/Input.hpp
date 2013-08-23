
#ifndef SPARK_INPUT_HPP
#define SPARK_INPUT_HPP

#include "Spark.hpp"

#include <string>
#include <map>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace spark
{
    /// Forward decl classes used internally
    class InputDevice;
    class KeyboardInputDevice;
    
    //TODO consistent nameing with GuiEventPublisher
    /// Manages the Input devices and provides a single query interface
    /// for all active devices.
    class Input
    {
    public:
        Input( void );
        virtual ~Input();

        /// Returns true if the key is currently down.
        /// TODO Using GLFW key codes; should abstract from GLFW
        virtual bool isKeyDown( int keyCode ) const;
        
        /// Returns the current transform indicated by the named input device.
        virtual glm::mat4 getTransform( const InputDeviceName& name ) const;
        
        /// Returns the position of the device's cursor. For 2d controllers, the
        /// z coordinate will be set to zero and can be ignored.
        virtual glm::vec3 getPosition( const InputDeviceName& name ) const;
        virtual bool isButtonPressed( const InputDeviceName& name,
                                      int buttonNumber ) const;
        
        /// Registers the input device for future use.
        /// Ownership of the InputDevice passes to this class.
        void acquireInputDevice( const InputDeviceName& name,
                                 std::unique_ptr<InputDevice> device );
        /// Registers the one keyboard input device for future use.
        /// Ownership of the device passes to this class.
        void acquireKeyboardDevice( std::unique_ptr<KeyboardInputDevice> device );
    private:
        /// owns the InputDevice
        std::map< InputDeviceName, std::unique_ptr<InputDevice> > m_devices;
        std::unique_ptr<KeyboardInputDevice> m_keyboardDevice;
    };
    typedef spark::shared_ptr< Input > InputPtr;
}
#endif
