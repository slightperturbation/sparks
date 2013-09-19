#ifndef SPARK_INPUTDEVICE_HPP
#define SPARK_INPUTDEVICE_HPP

#include "Spark.hpp"
#include "Updateable.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace spark
{
    /// Abstract class providing queries of user input.
    /// See class Input
    class InputDevice : public Updateable
    {
    public:
        virtual ~InputDevice() {}
        virtual void update( double dt ) override = 0;
        virtual glm::vec3 getPosition( void ) const = 0;
        virtual glm::mat4 getTransform( void ) const = 0;
        virtual bool isButtonPressed( int buttonNumber ) const = 0;
        virtual glm::vec3 getPositionRange( void ) const = 0;
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
        virtual glm::mat4 getTransform( void ) const override;
        virtual bool isButtonPressed( int buttonNumber ) const override;
    };
    

}

#endif
