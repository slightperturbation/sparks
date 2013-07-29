#ifndef SPARK_SPARK_FACADE_HPP
#define SPARK_SPARK_FACADE_HPP

#include "Mesh.hpp"
#include "RenderPass.hpp"
#include "RenderTarget.hpp"
#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "FileAssetFinder.hpp"
#include "Scene.hpp"
#include "GuiEventPublisher.hpp"

#include "LSpark.hpp"
#include "TexturedSparkRenderable.hpp"


namespace spark
{
    /// Wrapper for the Spark Rendering library
    /// Provides limited-dependency function exposing most of the spark lib.
    class SparkFacade
    {
    public:
        SparkFacade( ScenePtr scene,
                      FileAssetFinderPtr finder,
                      TextureManagerPtr tm,
                      ShaderManagerPtr sm,
                      PerspectiveProjectionPtr camera,
                      FrameBufferRenderTargetPtr frameBufferTarget,
                      GuiEventPublisherPtr guiEventPublisher
                    )
        : m_scene( scene ),
          m_finder( finder ),
          m_textureManager( tm ),
          m_shaderManager( sm ),
          m_overlayPerspective( new OrthogonalProjection ),
          m_cameraPerspective( camera ),
          m_frameBufferTarget( frameBufferTarget ),
          m_guiEventPublisher( guiEventPublisher )
        { }

        RenderTargetPtr getFrameBufferRenderTarget( void )
        {
            return m_frameBufferTarget;
        }

        /// Returns the "main" camera as a PerspectiveProjection.
        PerspectiveProjectionPtr getCamera( void )
        {
            return m_cameraPerspective;
        }
        
        RenderTargetPtr createTextureRenderTarget( 
            const TextureName& textureName )
        {
            return createScaledTextureRenderTarget( textureName, 1.0f );
        }

        /// Returns a newly allocated RenderTarget bound to write to 
        /// a new texture of textureName.
        /// The texture will resize (and become invalidated) when the 
        /// OpenGL context is resized, but the new target texture
        /// will always be sized as scaleFactor*(window width, window height) 
        RenderTargetPtr createScaledTextureRenderTarget( 
             const TextureName& textureName,
             float scaleFactor )
        {
            ScaledTextureRenderTargetPtr target(
                new ScaledTextureRenderTarget( textureName,
                m_guiEventPublisher->width(), 
                m_guiEventPublisher->height(),
                scaleFactor,
                scaleFactor ) );
            target->initialize( m_textureManager );
            m_guiEventPublisher->subscribe( target );
            return target;
        }
        
        RenderTargetPtr createScaledDepthTextureRenderTarget(
            const TextureName& depthTextureName,
            float scaleFactor )
        {
            ScaledTextureRenderTargetPtr target(
                new ScaledTextureRenderTarget( depthTextureName,
                    m_guiEventPublisher->width(),
                    m_guiEventPublisher->height(),
                    scaleFactor,
                    scaleFactor,
                    ScaledTextureRenderTarget::DepthOnly ) );
            target->initialize( m_textureManager );
            m_guiEventPublisher->subscribe( target );
            return target;
        }

        /// Return the pass with given name.  Returns a null if not found.
        RenderPassPtr getPass( const RenderPassName& name )
        {
            return m_scene->getPass( name );
        }

        /// Creates newTargetTextureName as a texture and renders to it
        /// using an overlay projection
        /// TODO need a new name!
        RenderTargetPtr createPostProcessingRenderPassAndTarget( 
            float priority,
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

        RenderTargetPtr createPostProcessingRenderPassAndScaledTarget( 
            float priority,
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
        
        /// Create a effects pass using shader effectShaderName
        /// that maps sourceTextureName texture on the target RenderTarget
        /// sourceTextureName -> target
        RenderPassPtr createPostProcessingRenderPass( 
            float priority,
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
        
        /// Create a effects pass using the given effect material
        /// that render the postprocess material on the target RenderTarget
        /// effectMaterial -> target
        /// See also createPostProcessingRenderPass(
        ///        float priority,
        ///        const RenderPassName& name,
        ///        const TextureName& sourceTextureName,
        ///        RenderTargetPtr target,
        ///        const ShaderName& effectShaderName )
        RenderPassPtr createPostProcessingRenderPass(
            float priority,
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

        /// Create a render pass using the default camera perspective
        RenderPassPtr createRenderPass( 
            float priority, 
            const RenderPassName& name,
            RenderTargetPtr target )
        {
            RenderPassPtr pass( new RenderPass( name ) );
            pass->initialize( target, m_cameraPerspective, priority );
            m_scene->add( pass );
            return pass;
        }

        /// Create a render pass
        RenderPassPtr createRenderPassWithProjection( 
            float priority, 
            const RenderPassName& name,
            ProjectionPtr camera,
            RenderTargetPtr target )
        {
            RenderPassPtr pass( new RenderPass( name ) );
            pass->initialize( target, camera, priority );
            m_scene->add( pass );
            return pass;
        }

        RenderPassPtr createOverlayRenderPass( float priority,
                                               const RenderPassName& name,
                                               RenderTargetPtr target )
        {
            RenderPassPtr pass( new RenderPass( name ) );
            ProjectionPtr ortho( new OrthogonalProjection );
            pass->initialize( target, ortho, priority );
            m_scene->add( pass );
            return pass;
        }


        void setMainRenderTarget( RenderTargetPtr target )
        {
            m_mainRenderTarget = target;
        }
        RenderTargetPtr getMainRenderTarget( void )
        {
            return m_mainRenderTarget;
        }

        MaterialPtr createMaterial( const ShaderName& shaderName )
        {
            return MaterialPtr( new Material( m_textureManager, 
                ShaderInstancePtr( new ShaderInstance( shaderName, 
                                                       m_shaderManager ) ) ) );
        }

        RenderablePtr loadMesh( const char* filename, 
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

        RenderablePtr createCube( const glm::vec3& position, 
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

        /// Create a 2d quad in the z=0 plane.  Useful for HUD-style 
        /// overlays and displaying text.
        RenderablePtr createQuad( const glm::vec2& lowerLeft, 
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
            quad->setMaterialForPassName( pass, material );
            return quad;
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

        RenderablePtr createLSpark( const glm::vec3& from, const glm::vec3& to,
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

        /// Delete all owned resources, release all linked resources.
        void reset( void ) 
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
        
    private:
        RenderTargetPtr m_mainRenderTarget; // tmp -- for debugging

        ScenePtr m_scene;
        // Managers
        FileAssetFinderPtr m_finder;
        TextureManagerPtr m_textureManager;
        ShaderManagerPtr m_shaderManager;
        // Common projections
        OrthogonalProjectionPtr m_overlayPerspective;
        PerspectiveProjectionPtr m_cameraPerspective;
        // FBOs
        FrameBufferRenderTargetPtr m_frameBufferTarget;
        GuiEventPublisherPtr m_guiEventPublisher;

    };
    typedef spark::shared_ptr< SparkFacade > SparkFacadePtr;
 }

 #endif
