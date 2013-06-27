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
            RenderPassPtr pass( new RenderPass( name ) );
            pass->initialize( target, m_overlayPerspective, priority );
            m_scene->add( pass );
            MeshPtr overlay( new Mesh() );
            overlay->name( std::string("OverlayQuad-") + sourceTextureName 
                + "-" + name + "-" + effectShaderName );
            overlay->addQuad( glm::vec3(0,0,0), glm::vec2(0,1), // Lower Left
                              glm::vec3(1,0,0), glm::vec2(1,1), // Lower Right
                              glm::vec3(0,1,0), glm::vec2(0,0), // Upper Left
                              glm::vec3(1,1,0), glm::vec2(1,0), // Upper Right
                              glm::vec3(0,0,-1) ); // Normal points according to right-hand system
            overlay->setRequireExplicitMaterial( true );
            ShaderInstancePtr colorShader = m_shaderManager
                ->createShaderInstance( effectShaderName );
            MaterialPtr colorMaterial( new Material( m_textureManager, 
                                                     colorShader ) );
            colorMaterial->addTexture( "s_color", sourceTextureName );
            overlay->setMaterialForPassName( name, colorMaterial );
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
            PerspectiveProjectionPtr camera, 
            RenderTargetPtr target )
        {
            RenderPassPtr pass( new RenderPass( name ) );
            pass->initialize( target, camera, priority );
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

        //        boost::shared_ptr< Material > createMaterial( )
//        {
//            
//        }
//        
//        void assignMaterial( boost::shared_ptr< Material > aMaterial,
//                            boost::shared_ptr< Renderable > aRenderable )
//        {
//            
//        }
//        
//        boost::shared_ptr< Mesh > loadMeshFile( const char* filename )
//        {
//            
//        }
        
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
