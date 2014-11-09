#ifndef SPARK_INPUTDEVICE_HPP
#define SPARK_INPUTDEVICE_HPP

#include "Spark.hpp"
#include "Updateable.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace spark
{
    /// Abstract class providing queries of user input.
    /// See class Input for the public interface.
    class InputDevice : public Updateable
    {
    public:
        virtual ~InputDevice() {}
        /// Allows the input device to poll the hardware.  Called once
        /// per frame.
        virtual void update( double dt ) override = 0;

        /// Returns the 3D world-space position of the input device.
        virtual glm::vec3 getPosition( void ) const = 0;
        /// Returns the position on the screen (z=0 in the default projection)
        /// that the input device is located or pointing at.
        virtual glm::vec2 getScreenPosition( void ) const = 0;
        /// Returns the transformation from world-space origin to the
        /// position of the input.
        virtual glm::mat4 getTransform( void ) const = 0;
        /// Returns true if the given buttonNumber is currently being pressed.
        /// 0 is the "default" or main button, 1 is the alternate button, 
        /// 2 may or may not have meaning on an input device
        virtual bool isButtonPressed( int buttonNumber ) const = 0;
        /// If supported, activates the vibro-tacile feedback.
        virtual void startVibration( void ) {}
        /// Stops vibrations. May interrupt an on-going vibration set
        /// by a previous call to vibrateForSeconds()
        virtual void stopVibration( void ) {} 
        /// If supported, activates the vibro-tacile feedback for the 
        /// given number of seconds, then stops vibrations.
        virtual void vibrateForSeconds( double duration ) { }
    };

    /// Interface for handling keyboard queries
    /// For callback event support see class GuiEventPublisher
    class KeyboardInputDevice
    {
    public:
        virtual ~KeyboardInputDevice() {}
        virtual bool isKeyDown( int key ) const = 0;
    };
    typedef std::string InputDeviceName;
    
    
    
    
    
    
    
    
    // TODO move to hydra
    class HydraInputDevice : public InputDevice
    {
    public:
        HydraInputDevice();
        virtual ~HydraInputDevice();
        virtual glm::vec3 getPosition( void ) const override;
        virtual glm::vec2 getScreenPosition( void ) const;
        virtual glm::mat4 getTransform( void ) const override;
        virtual bool isButtonPressed( int buttonNumber ) const override;
    };
    

}

#endif
