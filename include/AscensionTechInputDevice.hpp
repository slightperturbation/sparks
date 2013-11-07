#ifndef SPARK_ASCENSIONTECHINPUTDEVICE_HPP
#define SPARK_ASCENSIONTECHINPUTDEVICE_HPP

#include "InputDevice.hpp"
#include "InputFactory.hpp"

#include "ATC3DG.h"


namespace spark
{
    /// Wraps the AscensionTech API (ATC3DG) to provide support for the 
    /// magnetically tracked input device.
    class AscensionTechInputDevice : public InputDevice
    {
    public:
        AscensionTechInputDevice( void );
        virtual ~AscensionTechInputDevice();
        
        virtual void update( double dt ) override;
        virtual glm::vec3 getPosition( void ) const override;
        virtual glm::vec2 getScreenPosition( void ) const override;
        virtual glm::mat4 getTransform( void ) const override;

        //////////////////////////////////////////////////////////////////
        // Null functions for unsupported functionality
        virtual bool isButtonPressed( int buttonNumber ) const override
            { return false; }
        virtual void startVibration( void ) override {}
        virtual void stopVibration( void ) override {}
        virtual void vibrateForSeconds( double duration ) override {}
    private:
        void handleErrorCode( int errorCode );
    private:
        glm::mat4 m_transform;
        glm::vec3 m_position;
        glm::vec2 m_screenPosition;

        SYSTEM_CONFIGURATION m_systemConfig; //CSystem ATC3DG
        std::vector< SENSOR_CONFIGURATION > m_sensorConfigs; //CSensor pSensor
        std::vector< TRANSMITTER_CONFIGURATION > m_transmitterConfig; //CXmtr pXmtr
    };

    class AscensionTechInputFactory : public InputFactory
    {
    public:
        AscensionTechInputFactory( void );
        virtual ~AscensionTechInputFactory();
        virtual std::unique_ptr<KeyboardInputDevice> createKeyboard( void ) const override
        {
            throw "AscensionTech input doesn't have a keyboard, so none can be created.";
        }
        /// This creates a tracking device.
        virtual std::unique_ptr<InputDevice> createDevice( int index ) const override;
    };
}


#endif

