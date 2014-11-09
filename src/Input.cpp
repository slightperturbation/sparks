
#include "input/Input.hpp"

#include "input/InputDevice.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

spark::Input
::Input( void )
: m_defaultDevice( "NO_DEFAULT_DEVICE_SPECIFIED" )
{
    // Noop
}

spark::Input
::~Input()
{
    // Noop
}

void
spark::Input
::update( double dt )
{
    for( auto iter = m_devices.begin(); 
         iter != m_devices.end(); 
         ++iter )
    {
        std::unique_ptr<InputDevice>& device = iter->second;
        device->update( dt );
    }
}

bool
spark::Input
::isKeyDown( int key ) const
{
    if( !m_keyboardDevice )
    {
        LOG_ERROR(g_log) << "No keyboard device assigned yet.";
        //assert(false);
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
        //assert(false);
        return glm::mat4();
    }
    return iter->second->getTransform();
}

glm::mat4
spark::Input
::getDefaultDeviceTransform( void ) const
{
    return getTransform( m_defaultDevice );
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
        //assert(false);
        return glm::vec3();
    }
    return iter->second->getPosition();
}

glm::vec3
spark::Input
::getDefaultDevicePosition( void ) const 
{
    return getPosition( m_defaultDevice );
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
        //assert(false);
        return false;
    }
    return iter->second->isButtonPressed( buttonNumber );
}

bool
spark::Input
::isDefaultDeviceButtonPressed( int buttonNumber ) const
{
    return isButtonPressed( m_defaultDevice, buttonNumber );
}

glm::vec2
spark::Input
::getScreenPosition( const InputDeviceName& name ) const
{
    auto iter = m_devices.find( name );
    if( iter == m_devices.end() )
    {
        LOG_ERROR(g_log) << "Can't find input device with name \""
            << name << "\" in call to Input::getScreenPosition().";
        //assert(false);
        return glm::vec2();
    }
    return iter->second->getScreenPosition();
}

glm::vec2
spark::Input
::getDefaultDeviceScreenPosition( void ) const
{
    return getScreenPosition( m_defaultDevice );
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
::acquireInputDeviceAsDefault( const InputDeviceName& name,
                               std::unique_ptr<InputDevice> device )
{
    acquireInputDevice( name, std::move( device ) );
    setDefaultDeviceName( name );
}

void 
spark::Input
::setDefaultDeviceName( const InputDeviceName& name )
{
    m_defaultDevice = name;
}

const spark::InputDeviceName& 
spark::Input
::getDefaultDeviceName( void ) const
{
    return m_defaultDevice;
}

void
spark::Input
::acquireKeyboardDevice( std::unique_ptr<KeyboardInputDevice> device )
{
    m_keyboardDevice = std::move( device );
}

void 
spark::Input
::startVibration( const InputDeviceName& name )
{
    auto iter = m_devices.find( name );
    if( iter == m_devices.end() )
    {
        LOG_ERROR(g_log) << "Can't find input device with name \""
            << name << "\" in call to Input::startVibration().";
    }
    else
    {
        iter->second->startVibration();
    }
}

void 
spark::Input
::stopVibration( const InputDeviceName& name )
{
    auto iter = m_devices.find( name );
    if( iter == m_devices.end() )
    {
        LOG_ERROR(g_log) << "Can't find input device with name \""
            << name << "\" in call to Input::stopVibration().";
    }
    else
    {
        iter->second->stopVibration();
    }
}

void 
spark::Input
::vibrateForSeconds( const InputDeviceName& name, double duration )
{
    auto iter = m_devices.find( name );
    if( iter == m_devices.end() )
    {
        LOG_ERROR(g_log) << "Can't find input device with name \""
            << name << "\" in call to Input::vibrateForSeconds().";    
    }
    else
    {
        iter->second->vibrateForSeconds( duration );
    }
}
