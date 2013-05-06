//
//  Utilities.hpp
//  sparks
//
//  Created by Brian Allen on 3/26/13.
//
//

#ifndef sparks_Utilities_hpp
#define sparks_Utilities_hpp

#include "SoftTestDeclarations.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp>

#include <string>
#include <ostream>

class OpenGLWindow
{
public:
    OpenGLWindow( const char* programName ); 
    ~OpenGLWindow();
    bool isOK( void ) { return m_isOK; }
    bool isRunning( void )
    {
        return glfwGetWindowParam( GLFW_OPENED );
    }
private:
    bool m_isOK;
};

std::string readFileToString( const char* filename );
GLuint createShaderWithErrorHandling( GLuint shaderType, const std::string& shaderSource );
GLuint loadShaderFromFile( const char* vertexShaderFilepath, const char* fragmentShaderFilepath );

/// Write a 24-bit color binary PPM image file for the current frame buffer
/// files are named sequentially starting at 1, padded to 4 digits.
void writeFrameBufferToFile( const std::string& frameBaseFileName );

// Pre:  OpenGL must have a bound texture to load.
bool loadTextureFromFile( const char* filepath );

/// Pretty printing for GLM types
std::ostream& operator<<( std::ostream& out, glm::vec3 v );
std::ostream& operator<<( std::ostream& out, glm::vec4 v );
std::ostream& operator<<( std::ostream& out, glm::mat3 m );
std::ostream& operator<<( std::ostream& out, glm::mat4 m );

/// Debugging
void checkOpenGLErrors( void ); 
void checkOpenGLErrors( const char* aCodeStatement, 
                        const char* aFileName, 
                        int aLineNumber );
# ifdef _DEBUG 
#  define GL_CHECK(stmt) do { \
    stmt ; \
    checkOpenGLErrors( #stmt, __FILE__, __LINE__ ); \
} while( 0 )
# else
#  define GL_CHECK(stmt) stmt ;
# endif

#endif
