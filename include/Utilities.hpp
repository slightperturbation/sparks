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

#include "SDL.h"
#undef main // stupid fucking SDL, redefines main

namespace spark
{

    class AudioManager
    {
    public:
        AudioManager( FileAssetFinderPtr finder );
        ~AudioManager();
        void init( void );
        void playSound( void );
        void stopSound( void );
        static void audio_callback(void *userdata, Uint8 *stream, int len);
    private:
        bool is_playing;
        FileAssetFinderPtr m_finder;
        Uint8 *audio_pos; // global pointer to the audio buffer to be played
        Uint32 audio_len; // remaining length of the sample we have to play

        // Per sample
        Uint32 wav_length; // length of our sample
        Uint8 *wav_buffer; // buffer containing our audio file
        SDL_AudioSpec wav_spec; // the specs of our piece of music
    };
    typedef spark::shared_ptr< AudioManager > AudioManagerPtr;

    /// Simple encapsulation of GLFW main window.  Handles clean-up.
    class OpenGLWindow
    {
    public:
        OpenGLWindow( const char* programName, 
                      bool enableLegacyOpenGlLogging,
                      bool enableStereo,
                      bool createLoadingContext = false,
                      bool enableFullScreen = false ); 
        ~OpenGLWindow();
        /// Open the window, possibly destroying the current window if needed.
        void open();

        void makeContextCurrent( void );
        bool isOK( void ) { return m_isOK; }
        bool isRunning( void );
        int getKey( int key );
        void swapBuffers( void );
        void getSize( int* width, int* height );
        void getPosition( int* xPos, int* yPos );
        GLFWwindow* glfwWindow( void ) const { return m_glfwRenderWindow; }
        GLFWwindow* glfwLoadingThreadWindow( void ) { return m_glfwLoadingThreadWindow; }
        GLFWwindow* glfwRenderThreadWindow( void ) { return m_glfwRenderWindow; }
        EyeTrackerPtr getEyeTracker( void )  { return m_eyeTracker; }
        
        /// Returns the "screen coords" (lower-left origin, extents 1,1)
        /// of the given pixel position relative to the window
        glm::vec2 pixelsToScreenCoords( const glm::vec2& pixelPosition );
        glm::vec2 screenCoordsToPixels( const glm::vec2& screenCoord );
        void writeFrameBufferToFile( const std::string& frameBaseFileName );
 
        /// Parameters for window creation
        /// only used when open() is called.
        void setProgramName( const std::string& name ) { m_programName = name; }
        void setQuadBufferedStereo( bool isStereo ) { m_enableStereo = isStereo; }
        void setFullScreen( bool isFullScreen ) { m_enableFullScreen = isFullScreen; }
        void setLoadingContext( bool isLoadingContext ) { m_enableLoadingContext = isLoadingContext; }
        void setLegacyOpenGlLogging( bool isLegacyLogging ) { m_enableLegacyOpenGlLogging = isLegacyLogging; }

        bool isStereo() const { return m_enableStereo; }
        bool isFullScreen() const { return m_enableFullScreen; }

        /// Callbacks, store to allow re-registering if window is re-open()'d
        void setMousePositionCallback( GLFWcursorposfun callbackFunc ) { m_mousePosCallback = callbackFunc; }
        void setMouseButtonCallback( GLFWmousebuttonfun callbackFunc ) { m_mouseButtonCallback = callbackFunc; }
        void setFrameBufferSizeCallback( GLFWframebuffersizefun callbackFunc ) { m_frameBufferSizeCallback = callbackFunc; }
        void setWindowPositionCallback( GLFWwindowposfun callbackFunc ) { m_windowPositionCallback = callbackFunc; }

    private:
        std::string m_programName;
        bool m_enableLegacyOpenGlLogging;
        bool m_enableStereo;
        bool m_enableLoadingContext;
        bool m_enableFullScreen;

        EyeTrackerPtr m_eyeTracker;
        GLFWwindow* m_glfwRenderWindow;
        GLFWwindow* m_glfwLoadingThreadWindow;
        bool m_isOK;

        GLFWcursorposfun m_mousePosCallback;
        GLFWmousebuttonfun m_mouseButtonCallback;
        GLFWframebuffersizefun m_frameBufferSizeCallback;
        GLFWwindowposfun m_windowPositionCallback;
    };

    std::string readFileToString( const char* filename );
    GLuint createShaderWithErrorHandling( GLuint shaderType, 
                                          const std::string& shaderSource,
                                          const std::string& filename );
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

    double getTime( void );
    
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
