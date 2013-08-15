#include "Spark.hpp"

#include "Display.hpp"
#include "Renderable.hpp"
#include "Projection.hpp"
#include "Viewport.hpp"
#include "TextureManager.hpp"
#include "RenderTarget.hpp"
#include "FileAssetFinder.hpp"
#include "SparkFacade.hpp"

#include "DBMSpark.hpp"
#include "PointSparkRenderable.hpp"

#include "LSpark.hpp"
#include "TexturedSparkRenderable.hpp"

#include "TextRenderable.hpp"

#include "SlicedVolume.hpp"
#include "RayCastVolume.hpp"
#include "Fluid.hpp"

#include "Scene.hpp"
#include "ArcBall.hpp"
#include "EyeTracker.hpp"

#include "GuiEventPublisher.hpp"

#include "LuaInterpreter.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

void resizeWindowCallback( GLFWwindow* glfwWindow, int width, int height )
{
    if( g_guiEventPublisher )
    {
        g_guiEventPublisher->resizeViewport( 0, 0, width, height );
    }
}

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

void setGLFWCallbacks( GLFWwindow* glfwWindow )
{
    // Set GLFW event callbacks
    // - Redirect window size changes to the callback function WindowSizeCB
    glfwSetCursorPosCallback( glfwWindow, mousePosCallback );
    glfwSetMouseButtonCallback( glfwWindow, mouseButtonCallback );
    
    glfwSetWindowSizeCallback( glfwWindow, resizeWindowCallback );
#ifdef __APPLE__
    //glGetError(); // eat spurious GLFW-caused OpenGL errors on OSX/iOS
#endif
}



////////////////////////////////////////////////////////////////////////////////
// TODO Replace with loading from file (or running a script, python? lua?)

MaterialPtr loadPhongMaterial( TextureManagerPtr textureManager,
                               ShaderManagerPtr shaderManager )
{
    ShaderInstancePtr phongShader = shaderManager
        ->createShaderInstance( "phongShader" );
    MaterialPtr phongMaterial( new Material( textureManager, phongShader ) );
    // Bind texture to the color sampler (s_color is used in shader)
    phongMaterial->addTexture( "s_color", "checker" );
    //Todo these uniforms should be set by illumination in scene?
    phongMaterial->setShaderUniform( "u_light.position_camera",
                                    glm::vec4(5.0f,10.0f,0.0f,1.0f) );
    phongMaterial->setShaderUniform( "u_light.diffuse",
                                    glm::vec4(0.8f,0.8f,0.8f,1.0f) );
    phongMaterial->setShaderUniform( "u_ambientLight",
                                    glm::vec4(0.3f,0.1f,0.1f,1.0f) );
    // Phong/Blinn material properties
    phongMaterial->setShaderUniform( "u_ka",
                                    glm::vec4(1.0f,1.0f,1.0f,1.0f) );
    phongMaterial->setShaderUniform( "u_kd",
                                    glm::vec4(1.0f,1.0f,1.0f,1.0f) );
    phongMaterial->setShaderUniform( "u_ks",
                                    glm::vec4(1.0f,1.0f,1.0f,1.0f) );
    phongMaterial->setShaderUniform( "u_ns", 100.0f );
    return phongMaterial;
}
////////////////////////////////////////////////////////////////////////////////


/// Online simulation and display of fluid
int runSimulation(int argc, char** argv)
{
    bool enableLegacyOpenGlLogging  = false;
    OpenGLWindow window( "Spark", enableLegacyOpenGlLogging );
    using namespace std;
    InteractionVars vars;
    
    // Setup Gui Callbacks
    g_guiEventPublisher = GuiEventPublisherPtr( new GuiEventPublisher );
    setGLFWCallbacks( window.glfwWindow() );
        
    // Tell managers where to find file resources
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
    //EyeTrackerPtr eyeTracker( new NetworkEyeTracker );
    //eyeTracker->resizeViewport( 0, 0, width, height );
    //g_guiEventPublisher->subscribe( eyeTracker );

    ScenePtr scene( new Scene );
    ScenePtr sceneTwo( new Scene );
    
    OrthogonalProjectionPtr overlayPerspective( new OrthogonalProjection );
    PerspectiveProjectionPtr cameraPerspective( new PerspectiveProjection );
    cameraPerspective->cameraPos( 1.0f, 4.0f, -5.0f );
    cameraPerspective->cameraTarget( 0.5f, 0.5f, 0.0f );
    
    FrameBufferRenderTargetPtr frameBufferTarget(
        new FrameBufferRenderTarget( width, height ) );
    frameBufferTarget->initialize( textureManager );
    frameBufferTarget->setClearColor( glm::vec4( 0,0,0,0 ) );
    g_guiEventPublisher->subscribe( frameBufferTarget );

    SparkFacadePtr facade( new SparkFacade( scene,
                                            finder,
                                            textureManager,
                                            shaderManager,
                                            cameraPerspective,
                                            frameBufferTarget,
                                            g_guiEventPublisher ) );
    LuaInterpreter lua( finder );
    lua.setFacade( facade );
    lua.setTextureManager( textureManager );
    lua.setShaderManager( shaderManager );
    lua.runScriptFromFile( "loadShaders.lua" );
    lua.runScriptFromFile( "loadTextures.lua" );
    lua.runScriptFromFile( "loadRenderPasses.lua" );
    lua.runScriptFromFile( "main.lua" );

///////////////////////
    // sceneTwo only needs minimal passes
    SparkFacadePtr facadeTwo( new SparkFacade( sceneTwo,
        finder,
        textureManager,
        shaderManager,
        cameraPerspective,
        frameBufferTarget,
        g_guiEventPublisher ) );

    LuaInterpreter luaTwo( finder );
    luaTwo.setFacade( facadeTwo );
    luaTwo.setTextureManager( textureManager );
    luaTwo.setShaderManager( shaderManager );
    luaTwo.runScriptFromFile( "loadRenderPasses.lua" );
    
    
    /// FontManager fm( textureManager, "TextureAtlas" );
    /// const std::string fontName = "Sans";
    /// const int fontSize = 72;
    /// fm->addFont( fontName, fontSize, "HelveticaNeue.ttf" );
    /// TextRenderable tr( fm, "Message" );
    /// tr.intialize( fm, fontName, fontSize );
    /// tr.setText( "Hello, World" );
    
    FontManagerPtr fontManager( new FontManager(textureManager, 
        "FontAtlasTexture" ) );
    const std::string fontName = "Sans";
    const int fontSize = 72;
    fontManager->addFont( fontName, fontSize, "HelveticaNeueLight.ttf" );

    TextRenderablePtr textMsg( new TextRenderable("TestText") );
    textMsg->initialize( fontManager, fontName, fontSize );
    
    MaterialPtr textMaterial = facadeTwo->createMaterial( "TextShader" );
    textMaterial->setShaderUniform("u_color", glm::vec4( 1, 1, 1, 1 ) );
    textMaterial->addTexture( "s_color", fontManager->getFontAtlasTextureName() );
    textMsg->requiresExplicitMaterial();
    textMsg->setMaterialForPassName( "HUDPass", textMaterial );
    sceneTwo->add( textMsg );
    
    glm::vec4 color( 0.067,0.833, 0.086, 0.85 );
    float x = 0.0;
    float y = 0.5;
    glm::mat4 xform;
    xform = glm::translate( xform, x, y, 0.0f );
    textMsg->setTransform( xform );
    MaterialPtr textMat = textMsg->getMaterialForPassName( "HUDPass" );
    textMat->setShaderUniform( "u_color", color );
    //////////

//    // Second text using same font -- make sure doesn't get reloaded
//    TextRenderablePtr text2Msg( new TextRenderable("TestText") );
//    text2Msg->initialize( fontManager );
//    MaterialPtr text2Material = facade->createMaterial( "TextShader" );
//    text2Material->setShaderUniform("u_color", glm::vec4( 1, 0.3, 0.3, 1 ) );
//    text2Material->addTexture( "s_color", fontManager->getFontAtlasTextureName() );
//    text2Msg->requiresExplicitMaterial();
//    text2Msg->setMaterialForPassName( "HUDPass", text2Material );
//    scene->add( text2Msg );
//    
//    {
//        glm::vec4 color( 1,1,1,1 );
//        float x = 0.5;
//        float y = 0.5;
//        glm::mat4 xform;
//        xform = glm::translate( xform, x, y, 0.0f );
//        text2Msg->setTransform( xform );
//        MaterialPtr text2Mat = text2Msg->getMaterialForPassName( "HUDPass" );
//        text2Mat->setShaderUniform( "u_color", color );
//        text2Msg->setText( "VEST" );
//        
//    }
    //////////

    
    
    
    // spark renders to sparkRenderTexture
    // overlay renders to g_transparentRenderPass using a glow shader

    FluidPtr fluidData;
    if( false )
    {
        fluidData.reset( new Fluid(22) );
        fluidData->setDiffusion( 0.0 );
        fluidData->setVorticity( 1e4 );
        fluidData->setGravityFactor( 0, -5000, 0.0 );
        ////fluidData->loadFromFile( "test.fluid" );

        spark::shared_ptr< spark::SlicedVolume > slices( new
                SlicedVolume( textureManager,
                             shaderManager,
                             g_transparencyRenderPassName,
                             256, fluidData ) );
    //    RayCastVolumePtr rayCastFluid( new RayCastVolume( "fluid_raycastvolume",
    //                                               textureManager,
    //                                               shaderManager,
    //                                               fluidData ) );
        glm::mat4 xform = glm::translate( glm::mat4( 1.0f ),
                                          glm::vec3( 0.075f, 0.75f, 0.05f ) );
        //xform = glm::scale( xform, glm::vec3( 0.5f, 0.5f, 0.5f ) );
        //xform = glm::rotate( xform, 90.0f, glm::vec3(1,0,0) );
        
        //rayCastFluid->setTransform( xform );
        slices->setTransform( xform );
        //scene->add( rayCastFluid );
        scene->add( slices );
    }
    {
        // Set window/textures sizes by sending signals to listeners
        int width = 0; int height = 0;
        window.getSize( &width, &height );
        g_guiEventPublisher->resizeViewport( 0, 0, width, height );
    }

    ScenePtr currScene = sceneTwo;
    LuaInterpreter* currLua = &luaTwo;

    const double startTime = glfwGetTime();
    double nextSceneTime = startTime + 10.0;
    double currTime = startTime;
    double lastTime = startTime;
    while( window.isRunning() )
    {
        if( currTime > nextSceneTime )
        {
            currScene = (currScene == scene) ? sceneTwo : scene ;
            nextSceneTime = currTime + 10;
        }

        LOG_TRACE(g_log) << ".................................................";
        if( g_log->isTrace() )
        {
            textureManager->logTextures();
            currScene->logPasses();
        }

        glm::mat4 viewTransform;

        lastTime = currTime;
        currTime = glfwGetTime();
        vars.fps = 1.0f/(currTime - lastTime);

        std::stringstream ss;
        ss << "Time: " << currTime << "\nFPS: " << vars.fps << "\nThis is a test\n\tAnd another";
        textMsg->setText( ss.str() );

        //text2Msg->setText( "VEST", 1.0f);//*((int)(currTime) % 10) );

        // UPDATE
        const float dt = 1.0f/60.0f;
        if( g_arcBall )
        {
            g_arcBall->updatePerspective( cameraPerspective );
        }
        if( eyeTracker )
        {
            eyeTracker->updatePerspective( cameraPerspective );
        }

        currScene->prepareRenderCommands();
        currScene->render();
        window.swapBuffers();
        
        LOG_TRACE(g_log) << "Scene end - glfwSwapBuffers()";

        currScene->update( dt );
        
        if( vars.isSavingFrames ) writeFrameBufferToFile( "sparks_" );

        ////////////////////////////////////////////////////////////////////////
        // Process Inputs
        // See: http://www.glfw.org/docs/3.0/group__keys.html
        if( window.getKey( GLFW_KEY_UP ) == GLFW_PRESS )
        {
            fluidData->reset();
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
            currScene->logPasses();
        }
        if( window.getKey( 'R' ) == GLFW_PRESS )
        {
            currScene->reset();
            lua.runScriptFromFile( "defaultScene.lua" );
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

