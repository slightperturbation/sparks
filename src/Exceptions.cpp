
#include "Spark.hpp"
#include "Exceptions.hpp"


spark
::SparkRunTimeException
::SparkRunTimeException( const std::string& msg )
: runtime_error( msg )
{
    LOG_ERROR(g_log) << "SparkRunTimeException thrown: " << msg;
    std::cerr << "\nSparkRunTimeException thrown: " << msg << "\n\n";
}

spark
::ShaderCompilationException
::ShaderCompilationException( const std::string& msg,
                              const std::string& shaderSource )
: SparkRunTimeException( msg )
{
    m_msg = msg; m_shaderSource = shaderSource;
    LOG_DEBUG(g_log) << "ShaderCompilationException on shader source: "
                     << shaderSource;
}

const char*
spark
::ShaderCompilationException
::what() const throw()
{
    return m_msg.c_str();
}
