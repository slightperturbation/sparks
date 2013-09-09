#ifndef SPARK_ZSPACEINPUT_HPP
#define SPARK_ZSPACEINPUT_HPP

#include "InputFactory.hpp"
#include "InputDevice.hpp"
#include "ZSpaceSystem.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>


namespace spark 
{
    /// Provides tracking of the ZSpace stylus
    class ZSpaceInputDevice : public InputDevice
    {
    public:
        ZSpaceInputDevice();
        virtual ~ZSpaceInputDevice();
        virtual void update( float dt ) override;
        virtual glm::vec3 getPosition( void ) const override;
        virtual glm::mat4 getTransform( void ) const override;
        virtual bool isButtonPressed( int buttonNumber ) const override;
        virtual glm::vec3 getPositionRange( void ) const override;
    private:
        glm::mat4 m_transform;
        glm::vec3 m_position;
        bool m_isButtonPressed;
        std::vector< bool > m_previousButtonStates;
    };

    class ZSpaceInputFactory : public InputFactory
    {
    public:
        ZSpaceInputFactory( void ) {}
        /// TODO -- ZSpace is an example of an Input that doesn't know about keyboards
        ///  should (could) separate keyboard input to avoid this nasty throw.
        ///  Alternatively, could return a dummy keyboard that illicits no events.
        virtual std::unique_ptr<KeyboardInputDevice> createKeyboard( void ) const override
        {
            throw "ZSpace input doesn't have a keyboard, so none can be created.";
        }
        /// This creates a stylus device.  Handling the ZSpace stereo-view
        /// components is done by class ZSpaceEyeTracker
        virtual std::unique_ptr<InputDevice> createDevice( int index ) const override;
    };
}


#endif

