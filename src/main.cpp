#include "Spark.hpp"

#include "Display.hpp"
#include "Renderable.hpp"
#include "Projection.hpp"
#include "Viewport.hpp"
#include "TextureManager.hpp"
#include "RenderTarget.hpp"
#include "FileAssetFinder.hpp"
#include "SceneFacade.hpp"

#include "DBMSpark.hpp"
#include "PointSparkRenderable.hpp"

#include "LSpark.hpp"
#include "TexturedSparkRenderable.hpp"

#include "TextRenderable.hpp"


#include "Scene.hpp"
#include "ArcBall.hpp"
#include "EyeTracker.hpp"

#include "GuiEventPublisher.hpp"

#include "LuaInterpreter.hpp"

#include "Input.hpp"
#include "InputFactory.hpp"
#include "GlfwInput.hpp"

#include "NetworkEyeTracker.hpp"

#ifdef HAS_ZSPACE
  #include "ZSpaceEyeTracker.hpp"
  #include "ZSpaceInput.hpp"
#endif

#include "StateManager.hpp"
#include "State.hpp"
#include "SceneState.hpp"
#include "ScriptState.hpp"

#include "states/SimulationState.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <IL/il.h>

#include <memory>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdlib.h>


#include "freetype-gl.h"

using namespace std;
using namespace Eigen;
using namespace spark;

// Define the global Logger
cpplog::FilteringLogger* g_log = NULL;
boost::mutex g_logGuard;
cpplog::BaseLogger* g_baseLogger = NULL;

// Need globals to allow GLFW callbacks access.
static spark::GuiEventPublisherPtr g_guiEventPublisher;
static spark::ArcBallPtr g_arcBall;

class InteractionVars
{
public:
    InteractionVars()
    : fps( 0 ),
      isSavingFrames( false )
    {
        lightPos[0] = 0.0f;
        lightPos[1] = 0.5f;
        lightPos[2] = 0.5f;
        lightColor[0] = 0.7f;
        lightColor[0] = lightColor[0] = 0.3f;
    }
    float lightPos[3];
    float lightColor[3];
    
    float fps;
    bool isSavingFrames;
};

void mousePosCallback( GLFWwindow* glfwWindow, double x, double y )
{
    if( !g_arcBall ) return;
    g_arcBall->onMouseMove( x, y );
}

void mouseButtonCallback( GLFWwindow* glfwWindow,
                          int button, int action, int modifiers )
{
    if( !g_arcBall ) return;
    double x, y;
    glfwGetCursorPos( glfwWindow, &x, &y );
    if( button == GLFW_MOUSE_BUTTON_LEFT )
    {
        g_arcBall->onMouseRotationButton( x, y, GLFW_PRESS == action );
    }
    if( button == GLFW_MOUSE_BUTTON_RIGHT )
    {
        g_arcBall->onMouseDollyButton( x, y, GLFW_PRESS == action );
    }
    if( button == GLFW_MOUSE_BUTTON_MIDDLE )
    {
        g_arcBall->onMouseTrackingButton( x, y, GLFW_PRESS == action );
    }
}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    if( g_guiEventPublisher )
    {
        g_guiEventPublisher->resizeViewport( 0, 0, width, height );
    }
}

void windowPosCallback(GLFWwindow* window, int xpos, int ypos)
{
    if( g_guiEventPublisher )
    {
        g_guiEventPublisher->moveWindow( xpos, ypos );
    }
}

void setGLFWCallbacks( GLFWwindow* glfwWindow )
{
    // Set GLFW event callbacks
    // - Redirect window size changes to the callback function WindowSizeCB
    glfwSetCursorPosCallback( glfwWindow, mousePosCallback );
    glfwSetMouseButtonCallback( glfwWindow, mouseButtonCallback );
    
    glfwSetFramebufferSizeCallback( glfwWindow, frameBufferSizeCallback );
    glfwSetWindowPosCallback( glfwWindow, windowPosCallback );

#ifdef __APPLE__
    //glGetError(); // eat spurious GLFW-caused OpenGL errors on OSX/iOS
    // commented out because it violates the core profile
#endif
}

/// Online simulation and display of fluid
int runSimulation(int argc, char** argv)
{
    // Create Window
    // legacy logging is great, but conflicts with nSight debugger
    const bool enableLegacyOpenGlLogging  = false;

    bool useStereo = false;
#ifdef HAS_ZSPACE
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Too slow for testing.  Maybe a cmd line flag?  Setting in GUI?
    // Also, must be off for nVidia debugging
    useStereo = false;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
    OpenGLWindow window( "Spark", enableLegacyOpenGlLogging, useStereo );

    using namespace std;
    InteractionVars vars;
    
    // Setup Gui Callbacks
    g_guiEventPublisher = GuiEventPublisherPtr( new GuiEventPublisher );
    setGLFWCallbacks( window.glfwWindow() );

    // And Input manager
    // Choose windowing library -- glfw for now
    InputPtr inputManager( new Input );
    std::unique_ptr<InputFactory> glfwInputFactory(
        new GlfwInputFactory( window.glfwWindow() ) );
    inputManager->acquireKeyboardDevice( glfwInputFactory->createKeyboard() );
    inputManager->acquireInputDevice( "mouse",
                                     glfwInputFactory->createDevice(0) );

#ifdef HAS_ZSPACE
    ZSpaceInputFactory zSpaceInputFactory;
    inputManager->acquireInputDevice( "stylus", 
                                      zSpaceInputFactory.createDevice(0) );
#endif
    // Create common managers and tell them how to find file resources
    FileAssetFinderPtr finder( new FileAssetFinder );
    finder->addRecursiveSearchPath( DATA_PATH );
    ShaderManagerPtr shaderManager( new ShaderManager );
    shaderManager->setAssetFinder( finder );
    TextureManagerPtr textureManager( new TextureManager );
    textureManager->setAssetFinder( finder );
    
    int width = 0; int height = 0;
    window.getSize( &width, &height );

    g_arcBall = ArcBallPtr( new spark::ArcBall );
    g_arcBall->resizeViewport( 0, 0, width, height );
    g_guiEventPublisher->subscribe( g_arcBall );
    
    // Eye Tracker
    EyeTrackerPtr eyeTracker;
    // Create eyeTracker if desired
    {
#ifdef HAS_ZSPACE
        eyeTracker = window.getEyeTracker();
#endif
        if( !eyeTracker )
        {
            // by default, use NetworkEyeTracker
            eyeTracker = EyeTrackerPtr( new NetworkEyeTracker );
        }
        if( eyeTracker )
        {
            g_guiEventPublisher->subscribe( eyeTracker );
        }
    }

    // Set window/textures sizes by sending signals to listeners
    // Note that the current design has a circular dependency
    // between guiEventPublisher and textures, calling resize explicitly
    // helps break the cycle.  -- TODO
    {
        int width = 0; int height = 0;
        int xPos = 0; int yPos = 0;
        window.getSize( &width, &height );
        window.getPosition( &xPos, &yPos );
        g_guiEventPublisher->resizeViewport( 0, 0, width, height );
    }

    StateManager stateManager;

    PerspectiveProjectionPtr cameraPerspective( new PerspectiveProjection );
    g_guiEventPublisher->subscribe( cameraPerspective );
    
    FrameBufferRenderTargetPtr frameBufferTarget(
        new FrameBufferRenderTarget( width, height ) );
    frameBufferTarget->initialize( textureManager );
    frameBufferTarget->setClearColor( glm::vec4( 0,0,0,0 ) );
    g_guiEventPublisher->subscribe( frameBufferTarget );

    // Dummy scene for loading common resources
    ScenePtr sceneOne( new Scene );
    SceneFacadePtr facade( new SceneFacade( sceneOne,
                                            finder,
                                            textureManager,
                                            shaderManager,
                                            cameraPerspective,
                                            frameBufferTarget,
                                            inputManager,
                                            g_guiEventPublisher ) );
    // lua handles loading default objects
    LuaInterpreter lua( finder );
    lua.setFacade( facade );
    lua.runScriptFromFile( "loadShaders.lua" );
    lua.runScriptFromFile( "loadTextures.lua" );
    lua.runScriptFromFile( "loadRenderPasses.lua" );
    stateManager.addState( StatePtr(new SceneState( "sceneOne", 
                                                    sceneOne )) );
    
    //std::vector<std::string> states = { "Loading", "Menu", "Simulation" } ;
    std::vector<std::string> states;
    states.push_back( "Loading" );
    states.push_back( "Menu" );
    //states.push_back( "Simulation" );  // (If Simulation not already loaded)

    // Create the "special" simulation state from the C++ class
    //   Note that this class augments the SimulationLua script as well.
    stateManager.addState( StatePtr(new SimulationState( "Simulation", 
        facade ) ) ); 

    for( auto iter = states.begin(); iter != states.end(); ++iter )
    {
        StatePtr newState( new ScriptState( *iter,
                                            ScenePtr( new Scene ),
                                            finder,
                                            textureManager,
                                            shaderManager,
                                            cameraPerspective,
                                            frameBufferTarget,
                                            inputManager,
                                            g_guiEventPublisher ) );
        stateManager.addState( newState );
        
    }
    //stateManager.setCurrState( "Simulation" );
    stateManager.setCurrState( "Loading" );

    // spark renders to sparkRenderTexture
    // overlay renders to g_transparentRenderPass using a glow shader

    
    // Set window/textures sizes by sending signals to listeners
    // Note that the current design has a circular dependency
    // between guiEventPublisher and textures, calling resize explicitly
    // helps break the cycle.  -- TODO
    {
        int width = 0; int height = 0;
        int xPos = 0; int yPos = 0;
        window.getSize( &width, &height );
        window.getPosition( &xPos, &yPos );
        g_guiEventPublisher->resizeViewport( 0, 0, width, height );
    }
    const double startTime = glfwGetTime();
    double currTime = startTime;
    double lastTime = startTime;
    double totalTime = 0;
    double lastReportedFps = 0;
    double fpsReportThreshold = 1.0;
    double prevUpdateTime = 0;
    while( window.isRunning() )
    {
        LOG_TRACE(g_log) << ".................................................";
        if( g_log->isTrace() )
        {
            textureManager->logTextures();
        }

        glm::mat4 viewTransform;

        lastTime = currTime;
        currTime = glfwGetTime();
        vars.fps = 1.0f/(currTime - lastTime);

        // Report moving average of frame time
        const int framesPerSegment = 32;
        totalTime = totalTime 
            + (currTime-lastTime) // time this frame
            - (totalTime / framesPerSegment ); // moving avg removed
        if( std::abs( lastReportedFps - (framesPerSegment/totalTime) ) 
            > fpsReportThreshold )
        {
            std::cerr << "FPS over last " << framesPerSegment 
                << " frames:  \t\t" 
                << (framesPerSegment/totalTime) << "\n";
            lastReportedFps = (framesPerSegment/totalTime);
        }

        // UPDATE
        const float dt = 1.0f/60.0f;
        
        ////////////////////////////////////////////////////////////////////////
        // Update System (physics, collisions, etc.
        // 
        
        inputManager->update( dt );
        
        if( (currTime - prevUpdateTime) > dt )
        {
            LOG_TRACE(g_log) << "Fixed update at " << currTime;
            stateManager.fixedUpdate( dt );
            if( eyeTracker ) 
            {
                eyeTracker->fixedUpdate( dt );
            }
            prevUpdateTime = currTime;
        }
        LOG_TRACE(g_log) << "Update at " << currTime;
        stateManager.update( currTime - lastTime );
        if( eyeTracker ) 
        {
            eyeTracker->update( currTime - lastTime );
        }
        if( g_arcBall )
        {
            g_arcBall->updatePerspective( cameraPerspective );
        }

        ////////////////////////////////////////////////////////////////////////
        // Pre-render
        // parallel updates are complete, so handle queue'd opengl requests
        // before rendering next frame
        textureManager->executeQueuedCommands();

        ////////////////////////////////////////////////////////////////////////
        // Render
        window.makeContextCurrent();
        if( eyeTracker && !useStereo )
        {
            eyeTracker->updatePerspective( cameraPerspective );
        }
        if( eyeTracker && useStereo )
        {
            glDrawBuffer( GL_BACK_RIGHT );
            LOG_TRACE(g_log) << "Switching to RIGHT Buffer";
            eyeTracker->updatePerspective( cameraPerspective, EyeTracker::rightEye );
            stateManager.render(); // Extra scene render in stereo mode

            glDrawBuffer( GL_BACK_LEFT );
            LOG_TRACE(g_log) << "Switching to LEFT Buffer";
            eyeTracker->updatePerspective( cameraPerspective, EyeTracker::leftEye );
        }
        stateManager.render();
        window.swapBuffers();

        LOG_TRACE(g_log) << "Scene end - glfwSwapBuffers()";
        if( vars.isSavingFrames ) writeFrameBufferToFile( "sparks_" );
        
        stateManager.updateState( currTime );
        
        ////////////////////////////////////////////////////////////////////////
        // Process Inputs
        // See: http://www.glfw.org/docs/3.0/group__keys.html
        if( window.getKey( GLFW_KEY_UP ) == GLFW_PRESS )
        {
            //stateManager.currState()->reset();
        }
        if( window.getKey( GLFW_KEY_ENTER ) == GLFW_PRESS )
        {
            LOG_INFO(g_log) << "Reloading all shaders";
            shaderManager->reloadAllShaders();
        }
        if( window.getKey( GLFW_KEY_ESCAPE  ) == GLFW_PRESS )
        {
            break;
        }
        if( window.getKey( GLFW_KEY_BACKSPACE ) == GLFW_PRESS )
        {
            LOG_DEBUG(g_log) << "Reset sim.\n";
        }
        if( window.getKey( GLFW_KEY_F1 ) == GLFW_PRESS )
        {
            boost::mutex::scoped_lock lock( g_logGuard );
            delete g_log;
            g_log = new cpplog::FilteringLogger( LL_TRACE, g_baseLogger );
        }
        if( window.getKey( GLFW_KEY_F2 ) == GLFW_PRESS )
        {
            boost::mutex::scoped_lock lock( g_logGuard );
            delete g_log;
            g_log = new cpplog::FilteringLogger( LL_DEBUG, g_baseLogger );
        }
        if( window.getKey( GLFW_KEY_F3 ) == GLFW_PRESS )
        {
            boost::mutex::scoped_lock lock( g_logGuard );
            delete g_log;
            g_log = new cpplog::FilteringLogger( LL_INFO, g_baseLogger );
        }
        if( window.getKey( 'T' ) == GLFW_PRESS )
        {
            textureManager->logTextures();
        }
        if( window.getKey( 'P' ) == GLFW_PRESS )
        {
            //currScene->logPasses();
        }
        if( window.getKey( 'R' ) == GLFW_PRESS )
        {
            //currScene->reset();
            //lua.runScriptFromFile( "defaultScene.lua" );
        }
        //const float eyeSeparationStep = 0.02;
        if( window.getKey( GLFW_KEY_UP ) == GLFW_PRESS )
        {
            //SideBySideDisplay* d = dynamic_cast<SideBySideDisplay*>(g_display);
            //if( d )
            //{
            //    float dist = d->getEyeSeparation() + eyeSeparationStep;
            //    d->setEyeSeparation( dist );
            //    LOG_DEBUG(g_log) << "Increasing interoccular distance to " << dist << "\n";
            //}
        }
        if( window.getKey( GLFW_KEY_DOWN ) == GLFW_PRESS )
        {
            //SideBySideDisplay* d = dynamic_cast<SideBySideDisplay*>(g_display);
            //if( d )
            //{
            //    float dist = d->getEyeSeparation() - eyeSeparationStep;
            //    d->setEyeSeparation( dist );
            //    LOG_DEBUG(g_log) << "Decreasing interoccular distance to " << dist << "\n";
            //}
        }
    }
    stateManager.shutdown();
    textureManager->releaseAll();
    return 0;
}

int main( int argc, char* argv[] )
{
#if defined(_DEBUG) && defined(WIN32)
    int debugFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    //_CrtSetDbgFlag( debugFlags | _CRTDBG_CHECK_ALWAYS_DF |  _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  // SLOWEST
    //_CrtSetDbgFlag( debugFlags | _CRTDBG_CHECK_EVERY_1024_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); // SLOW
    //_CrtSetDbgFlag( debugFlags | _CRTDBG_DELAY_FREE_MEM_DF );
    _CrtSetDbgFlag( debugFlags |  _CRTDBG_LEAK_CHECK_DF ); // FAST
#endif 
    
    {
#ifdef CPPLOG_THREADING
        boost::mutex::scoped_lock lock( g_logGuard );
        cpplog::BackgroundLogger backgroundLogger( g_baseLogger );
        g_baseLogger = &backgroundLogger;
#endif // CPPLOG_THREADING
        if( argc > 1 && std::string(argv[1]) == std::string("-stderr") )
        {
            g_baseLogger = new cpplog::StdErrLogger;
        } else {
            g_baseLogger = new cpplog::FileLogger( "sparks.log" );
        }
        
        if( argc > 1 && std::string(argv[1]) == std::string("-trace") )
        {
            g_log = new cpplog::FilteringLogger( LL_TRACE, g_baseLogger );
            LOG_INFO(g_log) << "TRACE level logging.";
        } else {
            g_log = new cpplog::FilteringLogger( LL_DEBUG, g_baseLogger );
            LOG_INFO(g_log) << "DEBUG level logging.";
        }
    }
    runSimulation( argc, argv );
    {
        boost::mutex::scoped_lock lock( g_logGuard );
        delete g_log;
        delete g_baseLogger;
    }
}

