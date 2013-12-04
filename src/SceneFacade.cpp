
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
#include "TissueMesh.hpp"
#include "LSpark.hpp"
#include "TexturedSparkRenderable.hpp"
#include "Projection.hpp"
#include "Utilities.hpp"

spark::SceneFacade
::SceneFacade( ScenePtr scene,
               OpenGLWindow* window,
               FileAssetFinderPtr finder,
               TextureManagerPtr tm,
               ShaderManagerPtr sm,
               PerspectiveProjectionPtr camera,
               FrameBufferRenderTargetPtr frameBufferTarget,
               InputPtr inputManager,
               FontManagerPtr fontManager,
               GuiEventPublisherPtr guiEventPublisher
              )

: m_scene( scene ),
  m_window( window ),
  m_finder( finder ),
  m_textureManager( tm ),
  m_shaderManager( sm ),
  m_input( inputManager ),
  m_overlayPerspective( new OrthogonalProjection ),
  m_cameraPerspective( camera ),
  m_frameBufferTarget( frameBufferTarget ),
  m_fontManager( fontManager ),
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

spark::ProjectionPtr
spark::SceneFacade
::createOrthogonalProjection( float left, float right,
                              float bottom, float top,
                              float nearPlane, float farPlane,
                               glm::vec3 direction )
{
    OrthogonalProjectionPtr ortho
        = OrthogonalProjectionPtr( new OrthogonalProjection() );
    ortho->left( left );
    ortho->right( right );
    ortho->bottom( bottom );
    ortho->top( top );
    ortho->nearPlaneDistance( nearPlane );
    ortho->farPlaneDistance( farPlane );
    ortho->setLookAtDirection( direction );
    return ortho;
}

spark::ProjectionPtr
spark::SceneFacade
::createPerspectiveProjection( const glm::vec3& pos,
                               const glm::vec3& target,
                               const glm::vec3& up,
                               float fov, 
                               float nearPlane,
                               float farPlane )
{
    PerspectiveProjectionPtr ppp( new PerspectiveProjection() );
    ppp->cameraPos( pos );
    ppp->cameraTarget( target );
    ppp->cameraUp( up );
    ppp->fov( fov );
    ppp->nearPlaneDistance( nearPlane );
    ppp->farPlaneDistance( farPlane );
    return ppp;
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
::getRenderPass( const RenderPassName& name )
{
    return m_scene->getRenderPass( name );
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
    //    GuiEventSubscriberPtr gesp = boost::dynamic_pointer_cast<GuiEventSubscriber>(target);
    m_guiEventPublisher->subscribe( target );
    return target;
}

spark::RenderTargetPtr
spark::SceneFacade
::createDepthMapRenderTarget( const TextureName& depthTextureName,
                              int width, int height )
{
    DepthMapRenderTargetPtr target(
        new DepthMapRenderTarget( depthTextureName, width, height )
    );
    target->initialize( m_textureManager );
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
    overlay->bindDataToBuffers();
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
              float size,
              MaterialPtr material,
              const RenderPassName& pass )
{
    MeshPtr cube( new Mesh() );
    cube->cube( size );
    cube->name( "Cube" );
    cube->setMaterialForPassName( pass, material );
    glm::mat4 xform_trans = glm::translate( glm::mat4(), position );
    cube->setTransform( xform_trans );
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
    quad->name( "Quad-MAT:" + material->name() + "-PASS:" + pass );
    quad->addQuad(
                  glm::vec3( lowerLeft.x, lowerLeft.y, 0 ), glm::vec2(0,1), // Lower Left
                  glm::vec3( lowerLeft.x + size.x, lowerLeft.y, 0 ), glm::vec2(1,1), // Lower Right
                  glm::vec3( lowerLeft.x, lowerLeft.y + size.y, 0 ), glm::vec2(0,0), // Upper Left
                  glm::vec3( lowerLeft.x + size.x, lowerLeft.y + size.y, 0 ), glm::vec2(1,0), // Upper Right
                  glm::vec3( 0,0,-1 ) // Normal
                  );
    quad->bindDataToBuffers();
    quad->setRequireExplicitMaterial( true );
    m_scene->add( quad );
    quad->setMaterialForPassName( pass, material );
    return quad;
}

spark::RenderablePtr
spark::SceneFacade
::createPlane( const glm::vec3& center,
               const glm::vec2& scale,
               const glm::ivec2& subdivisions,
               MaterialPtr material,
               const RenderPassName& pass )
{
    MeshPtr plane( new Mesh() );
    plane->name( "Plane-MAT:" + material->name() + "-PASS:" + pass );
    plane->plane( center, scale, subdivisions );
    m_scene->add( plane );
    plane->setMaterialForPassName( pass, material );
    return plane;
}

spark::TexturedSparkRenderablePtr
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
    m_scene->add( sparkRenderable );
    m_scene->addUpdateable( sparkRenderable );
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
    m_fontManager.reset();
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
    m_scene->addUpdateable( text );
    return text;
}

spark::TissueMeshPtr
spark::SceneFacade
::createTissue( const RenderableName& name,
                float lengthInMeters,
                size_t numVoxelsPerDim )
{
    TissueMeshPtr tissue( new TissueMesh( name,
                                          m_textureManager,
                                          lengthInMeters,
                                          numVoxelsPerDim ) );
    return tissue;
}

spark::RenderPassPtr
spark::SceneFacade
::getRenderPassByName( const RenderPassName& name )
{
    return m_scene->getRenderPass( name );
}

glm::vec2
spark::SceneFacade
::pixelsToScreenCoords( const glm::vec2& pixelPosition )
{
    glm::vec2 out;
    if( m_window )
    {
        out = m_window->pixelsToScreenCoords( pixelPosition );
    }
    return out;
}

glm::vec2
spark::SceneFacade
::screenCoordsToPixels( const glm::vec2& screenCoord )
{
    glm::vec2 out;
    if( m_window )
    {
        out = m_window->screenCoordsToPixels( screenCoord );
    }
    return out;
}

glm::vec2
spark::SceneFacade
::getWindowSize( void )
{
    glm::vec2 out;
    if( m_window )
    {
        int width, height;
        m_window->getSize( &width, &height );
        out.x = width;
        out.y = height;
    }
    return out;
}


