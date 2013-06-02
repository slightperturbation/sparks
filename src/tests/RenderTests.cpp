
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <algorithm>

#include "SoftTestDeclarations.hpp"
#include "config.hpp"
#include "FileAssetFinder.hpp"
#include "ShaderManager.hpp"

#include "Renderable.hpp"
#include "RenderCommand.hpp"
#include "ShaderInstance.hpp"
#include "TextureManager.hpp"
#include "RenderTarget.hpp"
#include "Projection.hpp"
#include "RenderPass.hpp"
#include "Scene.hpp"

using namespace spark;

class TestRenderable : public Renderable
{
public:
    TestRenderable( TextureManagerPtr tm, ShaderManagerPtr sm ) 
        : Renderable( "TestRenderable" ), 
          renderedCount( 0 ) 
    { }
    mutable int renderedCount;
    virtual void render( void ) const
    {
        std::cout << "rendering TestRender";
        renderedCount++;
    }
    virtual void attachShaderAttributes( GLuint shaderIndex )
    {
        // glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId );
        // glVertexAttribPointer( ... );
        // glEnableVertexAttribArray( position_loc_in_shader );
    }
};

BOOST_AUTO_TEST_SUITE( RenderPipelineSuite )
BOOST_AUTO_TEST_CASE( CreateRenderCommands )
{
    OpenGLWindow( "Unit Tests - RenderPipelineSuite" );
    // busy wait until window opens
    FileAssetFinderPtr finder( new FileAssetFinder() );
    finder->addRecursiveSearchPath( DATA_PATH );
    TextureManagerPtr textureManager( new TextureManager( finder ) );
    ShaderManagerPtr shaderManager( new ShaderManager( finder ) );
    
    PerspectiveProjectionPtr camera( new PerspectiveProjection );
    int width = 400; int height = 400;
    RenderTargetPtr frameBufferTarget( new FrameBufferRenderTarget( width, height ) );
    frameBufferTarget->initialize( textureManager );

    ScenePtr scene( new Scene );

    RenderPassPtr primaryRenderPass( new RenderPass("TestRenderPass") );
    primaryRenderPass->initialize( frameBufferTarget, camera );
    scene->add( primaryRenderPass );

    std::shared_ptr< TestRenderable > testObject( new TestRenderable( textureManager, shaderManager ) );
    shaderManager->loadShaderFromFiles( "ColorShader", "colorVertexShader.glsl", "colorFragmentShader.glsl" );
    ShaderInstancePtr testShader( new ShaderInstance( "ColorShader", shaderManager ) );
    MaterialPtr material( new Material( textureManager, testShader ) );
    testObject->setMaterialForPassName( "TestRenderPass", material );
    scene->add( testObject ); 

    scene->prepareRenderCommands();

    // expect output of "rendering TestRender"
    BOOST_REQUIRE_EQUAL( testObject->renderedCount, 0 );   
    scene->prepareRenderCommands();
    scene->render();
    BOOST_REQUIRE_EQUAL( testObject->renderedCount, 1 );    
    scene->render();
    BOOST_REQUIRE_EQUAL( testObject->renderedCount, 1 );    
    scene->prepareRenderCommands();
    scene->render();
    BOOST_REQUIRE_EQUAL( testObject->renderedCount, 2 );    

}
BOOST_AUTO_TEST_SUITE_END()


//TODO move to own test file
BOOST_AUTO_TEST_SUITE( FileAssetSuite )

BOOST_AUTO_TEST_CASE( FailWithNoPaths )
{
    FileAssetFinder finder;
    std::string sparkPath;
    BOOST_REQUIRE( ! finder.findFile( DATA_PATH "textures/spark.png", sparkPath ) );
}
BOOST_AUTO_TEST_CASE( FileAsset_NonRecursiveSearch )
{
    FileAssetFinder finder;
    finder.addSearchPath( DATA_PATH "textures/" );
    std::string sparkPath;
    BOOST_REQUIRE( finder.findFile( "spark.png", sparkPath ) );
    BOOST_REQUIRE_EQUAL( sparkPath, DATA_PATH "textures/spark.png" );
}
BOOST_AUTO_TEST_CASE( FileAsset_RecursiveSearch )
{
    FileAssetFinder finder;
    finder.addRecursiveSearchPath( DATA_PATH );
    std::string sparkPath;
    BOOST_REQUIRE( finder.findFile( "spark.png", sparkPath ) );
    std::replace( sparkPath.begin(), sparkPath.end(), '\\', '/' ); // standardize slashes
    BOOST_REQUIRE_EQUAL( sparkPath, DATA_PATH "textures/spark.png" );
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ManagerSuite )

BOOST_AUTO_TEST_CASE( TextureMangerTests )
{
    FileAssetFinderPtr finder(new FileAssetFinder);
    finder->addRecursiveSearchPath( DATA_PATH );
    TextureManager tm;
    tm.setAssetFinder( finder );
    tm.loadTextureFromImageFile( "spark.png", "TestSpark" );
    BOOST_REQUIRE_NE( tm.getTextureIdForHandle( "TestSpark" ), -1 );
}
BOOST_AUTO_TEST_CASE( ShaderMangerTests )
{
    FileAssetFinderPtr finder(new FileAssetFinder);
    finder->addRecursiveSearchPath( DATA_PATH );
    ShaderManager sm;
    sm.setAssetFinder( finder );

}

BOOST_AUTO_TEST_SUITE_END()

