#include "Spark.hpp"

#include "Display.hpp"
#include "Renderable.hpp"
#include "Projection.hpp"
#include "TextureManager.hpp"
#include "RenderTarget.hpp"
#include "FileAssetFinder.hpp"
#include "SceneFacade.hpp"

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
#include <boost/thread.hpp>

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


void inputManagerUpdateThreadOp( OpenGLWindow* window, InputPtr inputManager )
{
    if( !window )
    {
        LOG_ERROR(g_log) << "Null OpenGLWindow passed to inputManagerUpdateThreadOp()";
        assert(false);
        return;
    }
    if( !inputManager )
    {        
        LOG_ERROR(g_log) << "Null inputManager in inputManagerUpdateThreadOp()";
        assert(false);
        return;
    }
    double lastUpdate = glfwGetTime();
    while( window->isRunning() )
    {
        inputManager->update( glfwGetTime() - lastUpdate );
        lastUpdate = glfwGetTime();
        boost::this_thread::sleep_for( boost::chrono::nanoseconds( 1 ) );
        //boost::this_thread::yield();
    }

}

void textureManagerCommandThreadOp( OpenGLWindow* window, TextureManager* textureManager )
{
    if( !window )
    {
        LOG_ERROR(g_log) << "Null OpenGLWindow passed to textureManagerCommandThreadOp()";
        assert(false);
        return;
    }
    if( !(window->glfwLoadingThreadWindow()) )
    {
        LOG_ERROR(g_log) << "Null glfwThreadWindow in textureManagerCommandThreadOp()";
        assert(false);
        return;
    }
    if( !textureManager )
    {        
        LOG_ERROR(g_log) << "Null textureManager in textureManagerCommandThreadOp()";
        assert(false);
        return;
    }
    LOG_DEBUG(g_log) << "Setting current context for loading thread";
    glfwMakeContextCurrent( window->glfwLoadingThreadWindow() );

    LOG_DEBUG(g_log) << "glewInit on resource thread.";
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if( err != GLEW_OK )
    {
        LOG_DEBUG(g_log) << "glewInit() failed!\n";
    }
    glewInit(); // needed for each thread

    while( window->isRunning() )
    {
        // Need to call glFlush() often (no more than ~30 ms
        double textureLoadStartTimeInSeconds = glfwGetTime();
        const double maxTextureLoadTimeInSeconds = (30.0)/(1000.0); // 30 ms in seconds
        while( textureManager->executeSingleQueuedCommand() 
               && (glfwGetTime() - textureLoadStartTimeInSeconds) < maxTextureLoadTimeInSeconds )
        {
            ;
        }
        //glFinish();
        glFlush();
        //boost::this_thread::sleep_for( boost::chrono::nanoseconds( 1 ) );
        boost::this_thread::yield();
    }
}

/// Online simulation and display of fluid
int runSimulation(int argc, char** argv)
{
    // legacy logging is great, but conflicts with nSight debugger
    const bool enableLegacyOpenGlLogging  = false;
    // Stero view, e.g., with ZSpace
    bool useStereo = false;
    // If should use full screen mode on the zspace or primary monitor (if no zspace)
    const bool enableFullScreen = false;

    // If true, sequence of frames is stored to disk
    bool isSavingFrames = false;

    // Create a separate thread to load background textures
    const bool useBackgroundResourceLoading = false;
    // Create a separate thread for input updates
    const bool useBackgroundInputUpdates = false;

    OpenGLWindow window( "Spark", 
                         enableLegacyOpenGlLogging, 
                         useStereo, 
                         useBackgroundResourceLoading,
                         enableFullScreen );
    
    // Setup Gui Callbacks
    g_guiEventPublisher = GuiEventPublisherPtr( new GuiEventPublisher );
    
    window.setMousePositionCallback( mousePosCallback );
    window.setMouseButtonCallback( mouseButtonCallback );
    window.setFrameBufferSizeCallback( frameBufferSizeCallback );
    window.setWindowPositionCallback( windowPosCallback );

    // Open must follow callbacks being set
    window.open();

    // And Input manager
    // Choose windowing library -- glfw for now
    InputPtr inputManager( new Input );
    std::unique_ptr<InputFactory> glfwInputFactory(
        new GlfwInputFactory( window ) );
    inputManager->acquireKeyboardDevice( glfwInputFactory->createKeyboard() );
    
    //
    // Setup input devices.  The order specifies the priority--
    // First, trakStar, then zSpace then mouse
    //
    inputManager->acquireInputDeviceAsDefault( "mouse",
                                               glfwInputFactory->createDevice(0) );
    
    // See ZSpaceInputFactory
    // See AscensionTechInputFactory

    // Create common managers and tell them how to find file resources
    FileAssetFinderPtr finder( new FileAssetFinder );
    finder->addRecursiveSearchPath( DATA_PATH );
    finder->addRecursiveSearchPath( DATA_PATH "/Examples/" );

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
    const bool shouldCreateEyeTracker = false;
    if( shouldCreateEyeTracker )
    {
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

    StateManager stateManager;

    PerspectiveProjectionPtr cameraPerspective( new PerspectiveProjection );
    g_guiEventPublisher->subscribe( cameraPerspective );
    
    FrameBufferRenderTargetPtr frameBufferTarget(
        new FrameBufferRenderTarget( width, height ) );
    frameBufferTarget->initialize( textureManager );
    frameBufferTarget->setClearColor( glm::vec4( 0,0,0,0 ) );
    g_guiEventPublisher->subscribe( frameBufferTarget );
    
    /// Create the displays for stereo and mono.
    DisplayPtr display;
    std::vector< DisplayPtr > allDisplays;
    
    DisplayPtr theQuadBufferStereoDisplay( new QuadBufferStereoDisplay );
    allDisplays.push_back( theQuadBufferStereoDisplay );

    DisplayPtr theSideBySideDisplay( new SideBySideDisplay );
    allDisplays.push_back( theSideBySideDisplay );

    DisplayPtr theMonoDisplay( new MonoDisplay );
    allDisplays.push_back( theMonoDisplay );
    
    for( auto d = allDisplays.begin(); d != allDisplays.end(); ++d )
    {
        (*d)->setPerspective( cameraPerspective );
        (*d)->setEyeTracker( eyeTracker );
        (*d)->setFrameBufferRenderTarget( frameBufferTarget );
        (*d)->setWindow( &window );
        g_guiEventPublisher->subscribe( *d );
    }
    
    // Choose the starting display type
    if( useStereo )
    {
        //display = theQuadBufferStereoDisplay;
        display = theSideBySideDisplay;
    }
    else
    {
        display = theMonoDisplay;
    }
    
    FontManagerPtr fontManager( new FontManager( textureManager, 
                                                 "FontAtlasTexture" ) );

    // Create the "special" simulation states from the C++ class
    // The specific functionality is provided in the Lua script,
    // the C++ class provides the tissue & smoke.
    std::vector< std::string > simStates;
    
    simStates.push_back( "Simulation" );
    simStates.push_back( "ShadowExample" );
    
    for( auto iter = simStates.begin(); iter != simStates.end(); ++iter )
    {
        StatePtr simState( new SimulationState( *iter, SceneFacadePtr( 
            new SceneFacade( ScenePtr( new Scene ),
                            &window,
                            finder,
                            textureManager,
                            shaderManager,
                            cameraPerspective,
                            frameBufferTarget,
                            inputManager,
                            fontManager,
                            g_guiEventPublisher ) ) ) );
        stateManager.addState( simState );
    }

    std::vector<std::string> scriptStates;
    // Examples
    scriptStates.push_back( "ShadowTest" );
//    scriptStates.push_back( "ShadowExample" );
    scriptStates.push_back( "ButtonExample" );
    scriptStates.push_back( "Example" );
    scriptStates.push_back( "SparkExample" );
    scriptStates.push_back( "ShaderExample" );
    scriptStates.push_back( "GlowExample" );

    // Actual States
    // \todo get all State.lua files in the States directory(?)
    scriptStates.push_back( "Startup" );
//    scriptStates.push_back( "Loading" );
//    scriptStates.push_back( "Menu" );
//    scriptStates.push_back( "Instructions" );
//    scriptStates.push_back( "Calibration" );
//    scriptStates.push_back( "ModeInstruction" );

    for( auto iter = scriptStates.begin(); iter != scriptStates.end(); ++iter )
    {
        StatePtr newState( new ScriptState( *iter,
                                            ScenePtr( new Scene ),
                                            &window,
                                            finder,
                                            textureManager,
                                            shaderManager,
                                            cameraPerspective,
                                            frameBufferTarget,
                                            inputManager,
                                            fontManager,
                                            g_guiEventPublisher ) );
        stateManager.addState( newState );
    }
    // Modify the StartupState.lua script to change the starting state
    stateManager.setCurrState( "Startup" );
    
    // Need to initialize the textures and framebuffers with the
    // starting window size, before any resize events are triggered
    // so inject a resize event.
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
    double prevUpdateTime = 0;
    
    // for fps counter
    double lastTimingUpdateTime = glfwGetTime();
    int framesSinceLastReport = 0;
    //
    std::map< std::string, double > secondsInComponentSinceLastReport;

    // Start Threads

    // Allow texture manager to process asynchronously using a second "window"
    if( useBackgroundResourceLoading )
    {
        // Lifespan of Window.isRunning is guaranteed shorter than textureManager
        TextureManager* tmRawPtr = textureManager.get();
        boost::thread textureManagerCommandThread( textureManagerCommandThreadOp, &window, tmRawPtr );
    }

    if( useBackgroundInputUpdates )
    {
        // Lifespan of Window.isRunning is guaranteed shorter than inputManager
        boost::thread inputManagerUpdateThread( inputManagerUpdateThreadOp, &window, inputManager );
    }

    // Main event loop -- responsible for input, rendering and synchronous updating
    while( window.isRunning() )
    {
        if( g_log->isTrace() )
        {
            LOG_TRACE(g_log) << ".................................................";
            textureManager->logTextures();
        }

        lastTime = currTime;
        currTime = glfwGetTime();

        ////////////////////////////////////////////////////////////////////////
        // Report profiling (every few seconds)
        // 
        double secondsBetweenProfileReports = 3.0;
        if(   (currTime - lastTimingUpdateTime >= secondsBetweenProfileReports)
            && framesSinceLastReport > 0 )
        {
            LOG_TRACE(g_log) << "\t" << (1000.0*secondsBetweenProfileReports)/(double)(framesSinceLastReport) << " ms/frame\n";
            double totalSeconds = 0.0;
            for( auto iter = secondsInComponentSinceLastReport.begin();
                 iter != secondsInComponentSinceLastReport.end(); 
                 ++iter )
            {
                LOG_TRACE(g_log) << "\t\t" << setw(20) << iter->first << "\t"
                    << setw(5) << (1000.0)*iter->second/(double)(framesSinceLastReport) 
                    << " ms\n";
                totalSeconds += iter->second;
                iter->second = 0.0; // reset accumulated time
            }
            LOG_TRACE(g_log) << "\t" << setw(20) << "Total:" << "\t"
                << setw(5) << (1000.0)*totalSeconds/(double)(framesSinceLastReport) 
                << " ms\n";
            framesSinceLastReport = 0;
            lastTimingUpdateTime = currTime;
        }
        framesSinceLastReport++;

        
        ////////////////////////////////////////////////////////////////////////
        // Update System (physics, collisions, etc.)
        //
        // Periodic updates roughly every dt
        const float dt = 1.0f/60.0f;
        double updateStartTime = glfwGetTime();
        if( (currTime - prevUpdateTime) > dt )
        {
            LOG_TRACE(g_log) << "Update at " << currTime;
            stateManager.update( dt );
            if( eyeTracker )
            {
                eyeTracker->update( dt );
            }
            prevUpdateTime = currTime;
        }
        stateManager.updateState( currTime );
        secondsInComponentSinceLastReport["Update"] += glfwGetTime() - updateStartTime;


        ////////////////////////////////////////////////////////////////////////
        // Data load
        // parallel updates are handled once per frame,
        // handle the queue'd opengl requests before rendering next frame
        if( ! useBackgroundResourceLoading )
        {
            double textureLoadStartTime = glfwGetTime();
            double maxTextureLoadMillisecondsPerFrame = (10.0)/(1000.0); // 10 ms in seconds
            while( textureManager->executeSingleQueuedCommand() &&
                (glfwGetTime()-textureLoadStartTime) < maxTextureLoadMillisecondsPerFrame )
            {
                ;
            }
            secondsInComponentSinceLastReport["TextureLoad"] += glfwGetTime() - textureLoadStartTime;
        }

        ////////////////////////////////////////////////////////////////////////
        // Input handlers 
        // 
        double inputUpdateStartTime = glfwGetTime();
        if( ! useBackgroundInputUpdates )
        {
            LOG_TRACE(g_log) << "Update at " << currTime;
            if( inputManager )
            {
                inputManager->update( currTime - lastTime );
            }
        }
        if( g_arcBall )
        {
            g_arcBall->updatePerspective( cameraPerspective );
        }
        secondsInComponentSinceLastReport["InputUpdate"] += glfwGetTime() - inputUpdateStartTime;

        ////////////////////////////////////////////////////////////////////////
        // Render Setup
        // 
        double renderSetupStartTime = glfwGetTime();
        
        display->render( stateManager );

        secondsInComponentSinceLastReport["RenderSetup"] += glfwGetTime() - renderSetupStartTime;
        

        ////////////////////////////////////////////////////////////////////////
        // Render
        // 
        double renderStartTime = glfwGetTime();
        window.swapBuffers();
        secondsInComponentSinceLastReport["Render"] += glfwGetTime() - renderStartTime;

        LOG_TRACE(g_log) << "Scene end - glfwSwapBuffers()";
        if( isSavingFrames ) window.writeFrameBufferToFile( "sparks_" );


        ////////////////////////////////////////////////////////////////////////
        // Process Inputs
        // See: http://www.glfw.org/docs/3.0/group__keys.html
        double inputProcessStartTime = glfwGetTime();
        if( window.getKey( GLFW_KEY_UP ) == GLFW_PRESS )
        {
            //stateManager.currState()->reset();
        }
        if( window.getKey( GLFW_KEY_DOWN ) == GLFW_PRESS )
        {
            //
        }
        if( window.getKey( GLFW_KEY_ENTER ) == GLFW_PRESS )
        {
            LOG_INFO(g_log) << "Reloading all shaders";
            shaderManager->reloadAllShaders();
        }
        if( window.getKey( GLFW_KEY_ESCAPE  ) == GLFW_PRESS )
        {
            // quit
            break;
        }
        //if( window.getKey( GLFW_KEY_BACKSPACE ) == GLFW_PRESS )
        //{
        //    LOG_DEBUG(g_log) << "Reset sim.\n";
        //}
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
        if( window.getKey( 'S' ) == GLFW_PRESS )
        {
            SideBySideDisplay& d = *(spark::dynamic_pointer_cast<SideBySideDisplay>( display ));
            d.enableOculusDistortion();
        }
        if( window.getKey( 'Q' ) == GLFW_PRESS )
        {
            display = theQuadBufferStereoDisplay;
        }
        if( window.getKey( 'M' ) == GLFW_PRESS )
        {
            display = theMonoDisplay;
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
        if( window.getKey( GLFW_KEY_HOME ) == GLFW_PRESS )
        {
            isSavingFrames = true;
        }
        if( window.getKey( GLFW_KEY_END ) == GLFW_PRESS )
        {
            isSavingFrames = false;
        }
        secondsInComponentSinceLastReport["InputProcessing"] += glfwGetTime() - inputProcessStartTime;

    }
    //if( useBackgroundResourceLoading )
    //{
    //    textureManagerCommandThread.join();
    //}
    
    // Join scene background threads, which should be closing
    // due to Scene::FixedTaskUpdate::m_isStopped == true
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
    return 0;
}

