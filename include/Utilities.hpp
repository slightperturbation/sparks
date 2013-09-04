//
//  Utilities.hpp
//  sparks
//
//  Created by Brian Allen on 3/26/13.
//
//

#ifndef sparks_Utilities_hpp
#define sparks_Utilities_hpp

#include "Spark.hpp"

#include "EyeTracker.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp>

#include <string>
#include <ostream>

namespace spark
{
    /// Simple encapsulation of GLFW main window.  Handles clean-up.
    class OpenGLWindow
    {
    public:
        OpenGLWindow( const char* programName, 
                      bool enableLegacyOpenGlLogging,
                      bool enableStereo ); 
        ~OpenGLWindow();
        void makeContextCurrent( void );
        bool isOK( void ) { return m_isOK; }
        bool isRunning( void );
        int getKey( int key );
        void swapBuffers( void );
        void getSize( int* width, int* height );
        void getPosition( int* xPos, int* yPos );
        GLFWwindow* glfwWindow( void ) { return m_glfwWindow; }
        EyeTrackerPtr getEyeTracker( void )  { return m_eyeTracker; }
    private:
        EyeTrackerPtr m_eyeTracker;
        GLFWwindow* m_glfwWindow;
        bool m_isOK;
    };

    std::string readFileToString( const char* filename );
    GLuint createShaderWithErrorHandling( GLuint shaderType, 
                                          const std::string& shaderSource );
    GLuint loadShaderFromFile( const char* vertexShaderFilepath, 
                               const char* fragmentShaderFilepath );
    /// Returns a trivial shader that will be used when a shader cannot be
    /// loaded or created.  Should be very noticeable (e.g., bright yellow)
    GLuint getErrorShader( void );

    /// Write a 24-bit color binary PPM image file for the current frame buffer
    /// files are named sequentially starting at 1, padded to 4 digits.
    void writeFrameBufferToFile( const std::string& frameBaseFileName );

    /// Load the image specified by filepath.
    /// Returns true if the texture is loaded and bound to current texture id.
    /// Pre:  filepath must be a full path or in the system path.
    /// Pre:  OpenGL must have a bound texture to load.
    bool loadTextureFromFile( const char* filepath );
    /// Pre:  OpenGL must have a bound texture to load.
    bool loadCheckerTexture( void );
    /// Pre:  OpenGL must have a bound texture to load.
    bool loadTestTexture( void );

    /// Load the given file and build new Mesh instances for contained assets.
    void createMeshesFromFile( const char* filePath, 
                               FileAssetFinderPtr finder, 
                               std::vector< MeshPtr >& outMeshes );

    /// Pretty printing for GLM types
    std::ostream& operator<<( std::ostream& out, const glm::vec2& v );
    std::ostream& operator<<( std::ostream& out, const glm::vec3& v );
    std::ostream& operator<<( std::ostream& out, const glm::vec4& v );
    std::ostream& operator<<( std::ostream& out, const glm::mat3& m );
    std::ostream& operator<<( std::ostream& out, const glm::mat4& m );

    /// Debugging
    void checkOpenGLErrors( void ); 
    void checkOpenGLErrors( const char* aCodeStatement, 
                            const char* aFileName, 
                            int aLineNumber );
} // end namespace spark


# ifdef _DEBUG 
#  define GL_CHECK(stmt) do { \
    stmt ; \
    spark::checkOpenGLErrors( #stmt, __FILE__, __LINE__ ); \
} while( 0 )
# else
#  define GL_CHECK(stmt) stmt ;
# endif

#endif
