#ifndef SPARK_GLFWINPUT_HPP
#define SPARK_GLFWINPUT_HPP

#include "InputDevice.hpp"
#include "InputFactory.hpp"

// Forward Decl
struct GLFWwindow;

namespace spark
{
    /// Concrete input device for the GLFW mouse
    class GlfwMouseInputDevice : public InputDevice
    {
    public:
        GlfwMouseInputDevice( GLFWwindow* window );
        virtual ~GlfwMouseInputDevice();
        virtual void update( double dt ) override {}
        virtual glm::vec3 getPosition( void ) const override;
        virtual glm::vec2 getScreenPosition( void ) const override;
        virtual glm::mat4 getTransform( void ) const override;
        virtual bool isButtonPressed( int buttonNumber ) const override;
    private:
        GLFWwindow* m_window; // non-owning
    };
    
    /// Concrete input device for the keyboard exposed by GLFW
    class GlfwKeyboardInputDevice : public KeyboardInputDevice
    {
    public:
        GlfwKeyboardInputDevice( GLFWwindow* window )
        : m_window( window ) {}
        virtual ~GlfwKeyboardInputDevice() {}
        virtual bool isKeyDown( int key ) const override;
    private:
        GLFWwindow* m_window; // not owned
    };
    
    /// Concrete factory for creating input devices using GLFW
    class GlfwInputFactory : public InputFactory
    {
    public:
        GlfwInputFactory( GLFWwindow* window );

        virtual std::unique_ptr<KeyboardInputDevice> createKeyboard( void ) const override;
        
        virtual std::unique_ptr<InputDevice> createDevice( int index ) const override;
    private:
        GLFWwindow* m_window;
    };
}
#endif
