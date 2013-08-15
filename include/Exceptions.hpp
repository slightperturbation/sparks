#ifndef SPARK_EXCEPTIONS_HPP
#define SPARK_EXCEPTIONS_HPP

#include <string>
#include <exception>

namespace spark
{
    /// Super-class for Runtime errors in the Spark library.
    class SparkRunTimeException : public std::runtime_error
    {
    public:
        SparkRunTimeException( const std::string& msg );
    };

    /// Thrown when the GLSL compilier reports an error
    /// in a shader.
    class ShaderCompilationException : public SparkRunTimeException
    {
    public:
        ShaderCompilationException( const std::string& msg, 
                                    const std::string& shaderSource );

        virtual const char* what() const throw();
        
        std::string m_msg;
        std::string m_shaderSource;
    };

    /// Thrown when Open GL reports an error in API use.
    class OpenGLException : public SparkRunTimeException
    {
    public:
        OpenGLException( const std::string& msg )
            : SparkRunTimeException( msg ) {}
    };


}

#endif
