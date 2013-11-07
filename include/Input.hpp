
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
    
    /// Manages the Input devices and provides a single query interface
    /// for all active devices.
    class Input
    {
    public:
        Input( void );
        virtual ~Input();

        /// Allow each input device to update itself.
        void update( double dt );

        /// Returns true if the key is currently down.
        /// http://www.glfw.org/docs/3.0/group__keys.html
        /// TODO Using GLFW key codes; should abstract from GLFW
        /// in lua, can use string.byte(), e.g.:
        ///   input:isKeyDown( string.byte('X') ) -- Note, capital letters only
        virtual bool isKeyDown( int keyCode ) const;
        
        /// Returns the current transform indicated by the named input device.
        virtual glm::mat4 getTransform( const InputDeviceName& name ) const;
        
        /// Returns the 3d world-space position of the device's cursor.
        virtual glm::vec3 getPosition( const InputDeviceName& name ) const;

        /// Returns a 2d position in normalized screen coordinates, with
        /// (0,0) in the lower left of the screen and (1,1) in the upper-right.
        virtual glm::vec2 getScreenPosition( const InputDeviceName& name ) const;

        /// Returns true if the button is currently pressed
        virtual bool isButtonPressed( const InputDeviceName& name,
                                      int buttonNumber ) const;

        virtual void startVibration( const InputDeviceName& name );
        virtual void stopVibration( const InputDeviceName& name );
        virtual void vibrateForSeconds( const InputDeviceName& name, double duration );
        
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
