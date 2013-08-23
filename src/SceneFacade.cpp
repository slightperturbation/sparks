
#include "SceneFacade.hpp"

#include "Mesh.hpp"
#include "RenderPass.hpp"
#include "RenderTarget.hpp"
#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "FileAssetFinder.hpp"
#include "Scene.hpp"
#include "GuiEventPublisher.hpp"
#include "TextRenderable.hpp"

#include "LSpark.hpp"
#include "TexturedSparkRenderable.hpp"


spark::SceneFacade
::SceneFacade( ScenePtr scene,
               FileAssetFinderPtr finder,
               TextureManagerPtr tm,
               ShaderManagerPtr sm,
               PerspectiveProjectionPtr camera,
               FrameBufferRenderTargetPtr frameBufferTarget,
               InputPtr inputManager,
               GuiEventPublisherPtr guiEventPublisher
              )

: m_scene( scene ),
  m_finder( finder ),
  m_textureManager( tm ),
  m_shaderManager( sm ),
  m_input( inputManager ),
  m_overlayPerspective( new OrthogonalProjection ),
  m_cameraPerspective( camera ),
  m_frameBufferTarget( frameBufferTarget ),
  m_guiEventPublisher( guiEventPublisher )
{ }

spark::RenderTargetPtr
spark::SceneFacade
::getFrameBufferRenderTarget( void )
{
    return m_frameBufferTarget;
}

spark::ScenePtr
spark::SceneFacade
::getScene( void )
{
    return m_scene;
}

spark::FileAssetFinderPtr
spark::SceneFacade
::getFinder( void )
{
    return m_finder;
}

spark::TextureManagerPtr
spark::SceneFacade
::getTextureManager( void )
{
    return m_textureManager;
}

spark::ShaderManagerPtr
spark::SceneFacade
::getShaderManager( void )
{
    return m_shaderManager;
}

spark::InputPtr
spark::SceneFacade
::getInput( void )
{
    return m_input;
}

/// Returns the "main" camera as a PerspectiveProjection.
spark::PerspectiveProjectionPtr
spark::SceneFacade
::getCamera( void )
{
    return m_cameraPerspective;
}

void
spark::SceneFacade
::setMainRenderTarget( RenderTargetPtr target )
{
    m_mainRenderTarget = target;
}

spark::RenderTargetPtr
spark::SceneFacade
::getMainRenderTarget( void )
{
    return m_mainRenderTarget;
}

/// Return the pass with given name.  Returns a null if not found.
spark::RenderPassPtr
spark::SceneFacade
::getPass( const RenderPassName& name )
{
    return m_scene->getPass( name );
}

spark::RenderTargetPtr
spark::SceneFacade
::createTextureRenderTarget( const TextureName& textureName )
{
    return createScaledTextureRenderTarget( textureName, 1.0f );
}

spark::RenderTargetPtr
spark::SceneFacade
::createScaledTextureRenderTarget( const TextureName& textureName,
                                   float scaleFactor )
{
    ScaledTextureRenderTargetPtr target(
        new ScaledTextureRenderTarget( textureName,
                                       m_guiEventPublisher->width(),
                                       m_guiEventPublisher->height(),
                                       scaleFactor,
                                       scaleFactor )
    );
    target->initialize( m_textureManager );
    m_guiEventPublisher->subscribe( target );
    return target;
}

spark::RenderTargetPtr
spark::SceneFacade
::createScaledDepthTextureRenderTarget( const TextureName& depthTextureName,
                                        float scaleFactor )
{
    ScaledTextureRenderTargetPtr target(
        new ScaledTextureRenderTarget( depthTextureName,
                                       m_guiEventPublisher->width(),
                                       m_guiEventPublisher->height(),
                                       scaleFactor,
                                       scaleFactor,
                                       ScaledTextureRenderTarget::DepthOnly )
    );
    target->initialize( m_textureManager );
    m_guiEventPublisher->subscribe( target );
    return target;
}


spark::RenderTargetPtr
spark::SceneFacade
::createPostProcessingRenderPassAndTarget( float priority,
                                           const RenderPassName& name,
                                           const TextureName& sourceTextureName,
                                           const TextureName& newTargetTextureName,
                                           const ShaderName& effectShaderName )
{
    return createPostProcessingRenderPassAndScaledTarget(
                                                         priority, name, 
                                                         sourceTextureName, newTargetTextureName, 
                                                         effectShaderName, 1.0f );
}

spark::RenderTargetPtr
spark::SceneFacade
::createPostProcessingRenderPassAndScaledTarget( float priority,
                                                 const RenderPassName& name,
                                                 const TextureName& sourceTextureName,
                                                 const TextureName& newTargetTextureName,
                                                 const ShaderName& effectShaderName,
                                                 float scaleFactor )
{
    RenderTargetPtr target = createScaledTextureRenderTarget( newTargetTextureName, scaleFactor );
    createPostProcessingRenderPass( priority, name,
                                    sourceTextureName, target, effectShaderName );
    return target;
}

spark::RenderPassPtr
spark::SceneFacade
::createPostProcessingRenderPass( float priority,
                                  const RenderPassName& name,
                                  const TextureName& sourceTextureName,
                                  RenderTargetPtr target,
                                  const ShaderName& effectShaderName )
{
    ShaderInstancePtr colorShader = m_shaderManager
    ->createShaderInstance( effectShaderName );
    MaterialPtr effectMaterial( new Material( m_textureManager,
                                              colorShader ) );
    effectMaterial->addTexture( "s_color", sourceTextureName );
    //effectMaterial->setShaderUniform<float>( "u_blurRadius", 0.25f );
    return createPostProcessingRenderPass( priority,
                                           name,
                                           target,
                                           effectMaterial );
}

spark::RenderPassPtr
spark::SceneFacade
::createPostProcessingRenderPass( float priority,
                                  const RenderPassName& name,
                                  RenderTargetPtr target,
                                  MaterialPtr effectMaterial )
{
    RenderPassPtr pass( new RenderPass( name ) );
    pass->initialize( target, m_overlayPerspective, priority );
    m_scene->add( pass );
    MeshPtr overlay( new Mesh() );
    overlay->name( std::string("OverlayQuad-")
                  + name + "-" + effectMaterial->name() );
    overlay->addQuad( glm::vec3(0,0,0), glm::vec2(0,1), // Lower Left
                     glm::vec3(1,0,0), glm::vec2(1,1), // Lower Right
                     glm::vec3(0,1,0), glm::vec2(0,0), // Upper Left
                     glm::vec3(1,1,0), glm::vec2(1,0), // Upper Right
                     glm::vec3(0,0,-1) ); // Normal points according to right-hand system
    overlay->setRequireExplicitMaterial( true );
    overlay->setMaterialForPassName( name, effectMaterial );
    m_scene->add( overlay );
    return pass;
}


spark::RenderPassPtr
spark::SceneFacade
::createRenderPass( float priority,
                    const RenderPassName& name,
                    RenderTargetPtr target )
{
    RenderPassPtr pass( new RenderPass( name ) );
    pass->initialize( target, m_cameraPerspective, priority );
    m_scene->add( pass );
    return pass;
}


spark::RenderPassPtr
spark::SceneFacade
::createRenderPassWithProjection( float priority,
                                  const RenderPassName& name,
                                  ProjectionPtr camera,
                                  RenderTargetPtr target )
{
    RenderPassPtr pass( new RenderPass( name ) );
    pass->initialize( target, camera, priority );
    m_scene->add( pass );
    return pass;
}


spark::RenderPassPtr
spark::SceneFacade
::createOverlayRenderPass( float priority,
                           const RenderPassName& name,
                           RenderTargetPtr target )
{
    RenderPassPtr pass( new RenderPass( name ) );
    ProjectionPtr ortho( new OrthogonalProjection );
    pass->initialize( target, ortho, priority );
    m_scene->add( pass );
    return pass;
}


spark::MaterialPtr
spark::SceneFacade
::createMaterial( const ShaderName& shaderName )
{
    return MaterialPtr( new Material( m_textureManager,
                                     ShaderInstancePtr( new ShaderInstance( shaderName,
                                                                           m_shaderManager ) ) ) );
}


spark::RenderablePtr
spark::SceneFacade
::loadMesh( const char* filename,
            MaterialPtr material,
            const RenderPassName& passName )
{
    std::vector< MeshPtr > meshes;
    createMeshesFromFile( filename, m_finder, meshes );
    if( meshes.empty() || meshes.size() > 1 )
    {
        LOG_ERROR(g_log) << "Loading meshes from \"" << filename
        << "\", expected one mesh but got " << meshes.size();
    }
    MeshPtr lastMesh;
    for( auto meshItr = meshes.begin(); meshItr != meshes.end();
        ++meshItr )
    {
        MeshPtr mesh = *meshItr;
        mesh->setMaterialForPassName( passName, material );
        m_scene->add( mesh );
        lastMesh = mesh;
    }
    return lastMesh;
}

spark::RenderablePtr
spark::SceneFacade
::createCube( const glm::vec3& position,
              const glm::vec3& size,
              MaterialPtr material,
              const RenderPassName& pass )
{
    MeshPtr cube( new Mesh() );
    cube->unitCube();
    cube->name( "Cube" );
    cube->setMaterialForPassName( pass, material );
    glm::mat4 xform( 1.0f );
    xform = glm::translate( xform, position );
    xform = glm::scale( xform, size );
    cube->setTransform( xform );
    m_scene->add( cube );
    return cube;
}


spark::RenderablePtr
spark::SceneFacade
::createQuad( const glm::vec2& lowerLeft,
              const glm::vec2& size,
              MaterialPtr material,
              const RenderPassName& pass )
{
    MeshPtr quad( new Mesh() );
    quad->name( "Quad-" + material->name() + "-" + pass );
    quad->addQuad(
                  glm::vec3( lowerLeft.x, lowerLeft.y, 0 ), glm::vec2(0,1), // Lower Left
                  glm::vec3( lowerLeft.x + size.x, lowerLeft.y, 0 ), glm::vec2(1,1), // Lower Right
                  glm::vec3( lowerLeft.x, lowerLeft.y + size.y, 0 ), glm::vec2(0,0), // Upper Left
                  glm::vec3( lowerLeft.x + size.x, lowerLeft.y + size.y, 0 ), glm::vec2(1,0), // Upper Right
                  glm::vec3( 0,0,-1 ) // Normal
                  );
    quad->setRequireExplicitMaterial( true );
    m_scene->add( quad );
    quad->setMaterialForPassName( pass, material );
    return quad;
}

spark::RenderablePtr
spark::SceneFacade
::createLSpark( const glm::vec3& from, const glm::vec3& to,
                float intensity,
                float scale,
                int recursiveDepth,
                float forkProb,
                const RenderPassName& pass, MaterialPtr material )
{
    LSparkPtr theSpark( new LSpark );
    theSpark->setViewProjection( m_cameraPerspective );
    theSpark->create( Eigen::Vector3f(from.x,from.y,from.z),
                     Eigen::Vector3f(to.x,to.y,to.z),
                     intensity, // intensity
                     scale, // scale
                     recursiveDepth,    // recursive depth
                     forkProb // fork probability
                     );
    TexturedSparkRenderablePtr sparkRenderable(
                                               new TexturedSparkRenderable( theSpark ) );
    sparkRenderable->name( "Spark" );
    sparkRenderable->setMaterialForPassName( pass,
                                            material );
    //sparkRenderable->setMaterialForPassName( g_transparencyRenderPassName,
    //                                         sparkColorMaterial );
    m_scene->add( sparkRenderable );
    return sparkRenderable;
}

void
spark::SceneFacade
::reset( void )
{
    m_mainRenderTarget.reset();
    m_scene.reset();
    m_textureManager.reset();
    m_shaderManager.reset();
    m_overlayPerspective.reset();
    m_cameraPerspective.reset();
    m_frameBufferTarget.reset();
    m_guiEventPublisher.reset();
}

spark::FontManagerPtr
spark::SceneFacade
::getFontManager( void )
{
    if( !m_fontManager )
    {
        m_fontManager = FontManagerPtr(
            new FontManager( m_textureManager,
                            "FontAtlasTexture" ) );
    }
    return m_fontManager;
}

spark::TextRenderablePtr
spark::SceneFacade
::createText( const std::string& fontName,
              int fontSize,
              MaterialPtr material,
              const RenderPassName& pass,
              const std::string& msg )
{
    std::stringstream uniqueName;
    uniqueName << fontName << "_" << fontSize << "_"
        << pass << "_" << msg;
    TextRenderablePtr text( new TextRenderable(uniqueName.str()) );
    text->initialize( m_fontManager, fontName, fontSize );
    text->requiresExplicitMaterial();
    text->setMaterialForPassName( pass, material );
    text->setText( msg );
    m_scene->add( text );
    return text;
}


///// Example of creating a full-screen quad for HUD-style overlay
//MeshPtr createOverlayQuad( TextureManagerPtr tm,
//                           ShaderManagerPtr sm,
//                           const TextureName& textureName,
//                           const RenderPassName& renderPassName,
//                           const ShaderName& shaderName )
//{
//    using namespace Eigen;
//    MeshPtr overlay( new Mesh() );
//    overlay->name( std::string("OverlayQuad-") + textureName
//        + "-" + renderPassName + "-" + shaderName );
//    overlay->addQuad( Vector3f(0,0,0), Vector2f(0,1), // Lower Left
//                      Vector3f(1,0,0), Vector2f(1,1), // Lower Right
//                      Vector3f(0,1,0), Vector2f(0,0), // Upper Left
//                      Vector3f(1,1,0), Vector2f(1,0), // Upper Right
//                      Vector3f(0,0,-1) ); // Normal points according to right-hand system
//    overlay->setRequireExplicitMaterial( true );
//    ShaderInstancePtr colorShader = sm->createShaderInstance( shaderName );
//    MaterialPtr colorMaterial( new Material( tm, colorShader ) );
//    colorMaterial->addTexture( "s_color", textureName );
//    overlay->setMaterialForPassName( renderPassName, colorMaterial );
//    return overlay;
//}




