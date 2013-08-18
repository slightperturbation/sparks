#ifndef SPARK_TIME_HPP
#define SPARK_TIME_HPP

#include "Spark.hpp"

// TODO -- separate GLFW dependency
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace spark
{
    /// Abstract class for reporting current time.
    class Time
    {
    public:
        virtual ~Time() {}
        virtual double seconds( void ) = 0;
    };

    class AbstractFactory
    {
    public:
        virtual TimePtr makeTime( void ) = 0;
    };
 
    ////////////////////////////////////////////////////////////////////
    // TODO -- separate GLFW dependency
    /// Concrete time class
    class GlfwTime : public Time
    {
    public:
        virtual ~GlfwTime() {}
        virtual double seconds( void ) override
        { return glfwGetTime(); }
    };
    
    class GlfwFactory : public AbstractFactory
    {
    public:
        virtual TimePtr makeTime( void ) override
        { return TimePtr( new GlfwTime ); }
    };
}

#endif
