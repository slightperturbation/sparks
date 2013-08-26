
#include "Input.hpp"

#include "InputDevice.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

spark::Input
::Input( void )
{
    // Noop
}

spark::Input
::~Input()
{
    // Noop
}

bool
spark::Input
::isKeyDown( int key ) const
{
    if( !m_keyboardDevice )
    {
        LOG_ERROR(g_log) << "No keyboard device assigned yet.";
        assert(false);
        return false;
    }
    return m_keyboardDevice->isKeyDown( key );
}

glm::mat4
spark::Input
::getTransform( const InputDeviceName& name ) const
{
    auto iter = m_devices.find( name );
    if( iter == m_devices.end() )
    {
        LOG_ERROR(g_log) << "Can't find input device with name \""
        << name << "\" in call to Input::getTransform().";
        assert(false);
        return glm::mat4();
    }
    return iter->second->getTransform();
}

glm::vec3
spark::Input
::getPosition( const InputDeviceName& name ) const
{
    auto iter = m_devices.find( name );
    if( iter == m_devices.end() )
    {
        LOG_ERROR(g_log) << "Can't find input device with name \""
        << name << "\" in call to Input::getPosition().";
        assert(false);
        return glm::vec3();
    }
    return iter->second->getPosition();
}

bool
spark::Input
::isButtonPressed( const InputDeviceName& name, int buttonNumber ) const
{
    auto iter = m_devices.find( name );
    if( iter == m_devices.end() )
    {
        LOG_ERROR(g_log) << "Can't find input device with name \""
        << name << "\" in call to Input::isButtonPressed().";
        assert(false);
        return false;
    }
    return iter->second->isButtonPressed( buttonNumber );
}

glm::vec3 
spark::Input
::getPositionRange( const InputDeviceName& name ) const
{
    auto iter = m_devices.find( name );
    if( iter == m_devices.end() )
    {
        LOG_ERROR(g_log) << "Can't find input device with name \""
            << name << "\" in call to Input::getPositionRange().";
        assert(false);
        return glm::vec3();
    }
    return iter->second->getPositionRange();
}

void
spark::Input
::acquireInputDevice( const InputDeviceName& name,
                      std::unique_ptr<InputDevice> device )
{
    m_devices[name] = std::move( device );
}

void
spark::Input
::acquireKeyboardDevice( std::unique_ptr<KeyboardInputDevice> device )
{
    m_keyboardDevice = std::move( device );
}
