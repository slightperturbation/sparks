
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
        /// 
        /// Some key codes are exposed to Lua in LuaInterpreter.cpp
        /// E.g., KEY_KP_1, KEY_KP_2, KEY_KP_ENTER, etc.
        /// 
        /// Alternatively, Lua callers can use string.byte(), e.g.:
        ///   input:isKeyDown( string.byte('X') ) -- Note, capital letters only!
        virtual bool isKeyDown( int keyCode ) const;
        
        /// Returns the current transform indicated by the named input device.
        virtual glm::mat4 getTransform( const InputDeviceName& name ) const;
        /// Returns the current transform of the default input device.
        virtual glm::mat4 getDefaultDeviceTransform( void ) const;
        
        /// Returns the 3d world-space position of the device's cursor.
        virtual glm::vec3 getPosition( const InputDeviceName& name ) const;
        /// Returns the current 3D world-space position of the default device.
        virtual glm::vec3 getDefaultDevicePosition( void ) const;

        /// Returns a 2d position in normalized screen coordinates, with
        /// (0,0) in the lower left of the screen and (1,1) in the upper-right.
        virtual glm::vec2 getScreenPosition( const InputDeviceName& name ) const;
        /// Returns the 2D position in normalized screen coordinates, with
        /// (0,0) in the lower left, of the default device.
        virtual glm::vec2 getDefaultDeviceScreenPosition( void ) const;

        /// Returns true if the button is currently pressed
        virtual bool isButtonPressed( const InputDeviceName& name,
                                      int buttonNumber ) const;
        /// Returns true if the given button is currently pressed on the
        /// default device.
        virtual bool isDefaultDeviceButtonPressed( int buttonNumber ) const;

        /// If the named input device supports vibration, start vibrating 
        /// the device until stopVibration is called.
        virtual void startVibration( const InputDeviceName& name );
        /// Stops vibration.  No effect if the device is not currently
        /// vibrating.
        virtual void stopVibration( const InputDeviceName& name );
        virtual void vibrateForSeconds( const InputDeviceName& name, double duration );
        
        /// Registers the input device for future use.
        /// Ownership of the InputDevice passes to this class.
        void acquireInputDevice( const InputDeviceName& name,
                                 std::unique_ptr<InputDevice> device );
        /// Registers the input device for future use, just as acquireInputDevice(),
        /// and also sets this device as the default device for calls
        /// that do not explicitly provide the InputDeviceName.
        void acquireInputDeviceAsDefault( const InputDeviceName& name,
                                          std::unique_ptr<InputDevice> device );
        /// Sets the default device to use for calls that do not explicitly
        /// provide an InputDeviceName.
        void setDefaultDeviceName( const InputDeviceName& name );

        /// Returns the current default device, as set by either acquireInputDeviceAsDefault()
        /// or setDefaultInputDeviceName()
        const InputDeviceName& getDefaultDeviceName( void ) const;

        /// Registers the one keyboard input device for future use.
        /// Ownership of the device passes to this class.
        void acquireKeyboardDevice( std::unique_ptr<KeyboardInputDevice> device );
    private:
        /// owns the InputDevice
        std::map< InputDeviceName, std::unique_ptr<InputDevice> > m_devices;
        std::unique_ptr<KeyboardInputDevice> m_keyboardDevice;
        InputDeviceName m_defaultDevice;
    };
    typedef spark::shared_ptr< Input > InputPtr;
}
#endif
