

#include "GlfwInput.hpp"

#include "Utilities.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>


spark::GlfwMouseInputDevice
::GlfwMouseInputDevice( GLFWwindow* window )
: m_window( window )
{
    if( !window )
    {
        LOG_ERROR(g_log) << "Error attempting to initialize GLFW mouse "
        << "device with nullptr window.";
        assert(false);
    }
}

spark::GlfwMouseInputDevice
::~GlfwMouseInputDevice()
{
    // Noop
}

glm::vec3
spark::GlfwMouseInputDevice
::getPosition( void ) const
{
    return glm::vec3( getScreenPosition(), 0 );
}

glm::vec2 
spark::GlfwMouseInputDevice
::getScreenPosition( void ) const
{
    double x, y;
    glfwGetCursorPos( m_window, &x, &y );

    int width, height;
    glfwGetWindowSize( m_window, &width, &height );
    glm::vec2 screenPos( x/(float)width, 1.0f - (y/(float)height) );
    return screenPos;
}

//TODO move arcball implementation here
glm::mat4
spark::GlfwMouseInputDevice
::getTransform( void ) const
{
    double x, y;
    glfwGetCursorPos( m_window, &x, &y );
    glm::mat4 xform;
    xform = glm::translate( xform, glm::vec3(x, y, 0) );
    return xform;
}

bool
spark::GlfwMouseInputDevice
::isButtonPressed( int buttonNumber ) const
{
    return glfwGetMouseButton( m_window, buttonNumber );
}

//////////////////////////////////////////////////////////////////////

bool
spark::GlfwKeyboardInputDevice
::isKeyDown( int key ) const
{
    return glfwGetKey( m_window, key ) == GLFW_PRESS;
}

//////////////////////////////////////////////////////////////////////

spark::GlfwInputFactory
::GlfwInputFactory( OpenGLWindow& window )
: m_window( window )
{
    // Noop
}

std::unique_ptr<spark::KeyboardInputDevice>
spark::GlfwInputFactory
::createKeyboard( void ) const
{
    std::unique_ptr<KeyboardInputDevice> kid(
        new GlfwKeyboardInputDevice( m_window.glfwWindow() ) );
    return std::move( kid );
}

std::unique_ptr<spark::InputDevice>
spark::GlfwInputFactory
::createDevice( int index ) const
{
    if( index != 0 )
    {
        LOG_ERROR(g_log) << "GLFW only supports one mouse device";
    }
    std::unique_ptr<InputDevice> id( new GlfwMouseInputDevice(m_window.glfwWindow()) );
    return std::move( id );
}
