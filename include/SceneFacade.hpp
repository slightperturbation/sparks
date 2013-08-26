#ifndef SPARK_SPARK_FACADE_HPP
#define SPARK_SPARK_FACADE_HPP

#include "Spark.hpp"
#include "Mesh.hpp"
#include "RenderPass.hpp"
#include "RenderTarget.hpp"
#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "FileAssetFinder.hpp"
#include "Scene.hpp"
#include "GuiEventPublisher.hpp"

#include "FontManager.hpp"

#include "LSpark.hpp"
#include "TexturedSparkRenderable.hpp"


namespace spark
{
    /// Wrapper for the Spark Rendering library
    /// Provides limited-dependency function exposing most of the spark lib.
    class SceneFacade
    {
    public:
        SceneFacade( ScenePtr scene,
                      FileAssetFinderPtr finder,
                      TextureManagerPtr tm,
                      ShaderManagerPtr sm,
                      PerspectiveProjectionPtr camera,
                      FrameBufferRenderTargetPtr frameBufferTarget,
                      InputPtr inputManager,
                      GuiEventPublisherPtr guiEventPublisher
                    );
        RenderTargetPtr getFrameBufferRenderTarget( void );
        
        ScenePtr getScene( void );

        FileAssetFinderPtr getFinder( void );
        
        TextureManagerPtr getTextureManager( void );
        
        ShaderManagerPtr getShaderManager( void );
        
        InputPtr getInput( void );
        
        /// Returns the camera used as a default PerspectiveProjection.
        /// This is the camera that will be jittered for 3d rendering.
        PerspectiveProjectionPtr getCamera( void );
        
        RenderTargetPtr getMainRenderTarget( void );
        void setMainRenderTarget( RenderTargetPtr target );
        
        /// Return the pass with given name.  Returns a null if not found.
        RenderPassPtr getPass( const RenderPassName& name );
        
        /// Create and return a TextureRenderTarget of same size as the
        /// main render target that allows rendering to the texture textureName
        RenderTargetPtr createTextureRenderTarget( const TextureName& textureName );

        /// Returns a newly allocated RenderTarget bound to write to 
        /// a new texture of textureName.
        /// The texture will resize (and become invalidated) when the 
        /// OpenGL context is resized, but the new target texture
        /// will always be sized as scaleFactor*(window width, window height) 
        RenderTargetPtr createScaledTextureRenderTarget( 
             const TextureName& textureName,
             float scaleFactor );
        
        /// Create a ScaledTextureRenderTarget that only writes depth
        RenderTargetPtr createScaledDepthTextureRenderTarget(
            const TextureName& depthTextureName,
            float scaleFactor );

        /// Creates newTargetTextureName as a texture and renders to it
        /// using an overlay projection
        /// TODO need a new name!
        RenderTargetPtr createPostProcessingRenderPassAndTarget( 
            float priority,
            const RenderPassName& name,
            const TextureName& sourceTextureName,
            const TextureName& newTargetTextureName,
            const ShaderName& effectShaderName );

        /// Creates newTargetTextureName as a texture and renders to it
        /// using an overlay projection. New texture is sized to
        /// scaleFactor * current main render target
        RenderTargetPtr createPostProcessingRenderPassAndScaledTarget(
            float priority,
            const RenderPassName& name,
            const TextureName& sourceTextureName,
            const TextureName& newTargetTextureName,
            const ShaderName& effectShaderName,
            float scaleFactor );

        /// Create a effects pass using shader effectShaderName
        /// that maps sourceTextureName texture on the target RenderTarget
        /// sourceTextureName -> target
        RenderPassPtr createPostProcessingRenderPass( 
            float priority,
            const RenderPassName& name,
            const TextureName& sourceTextureName,
            RenderTargetPtr target,
            const ShaderName& effectShaderName );
        
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
            MaterialPtr effectMaterial );

        /// Create a render pass using the default camera perspective
        RenderPassPtr createRenderPass( 
            float priority, 
            const RenderPassName& name,
            RenderTargetPtr target );

        /// Create a render pass
        RenderPassPtr createRenderPassWithProjection( 
            float priority, 
            const RenderPassName& name,
            ProjectionPtr camera,
            RenderTargetPtr target );
        
        /// Create a new RenderPass of name and a default orthographic
        /// projection and add it to the scene with render priority.
        RenderPassPtr createOverlayRenderPass( float priority,
                                               const RenderPassName& name,
                                               RenderTargetPtr target );

        /// Create and return a new Material based on shaderName
        MaterialPtr createMaterial( const ShaderName& shaderName );

        /// Load the mesh file from filename (using the AssetFinder attached
        /// to this SceneFacade) and assign material for passName.
        RenderablePtr loadMesh( const char* filename,
                                MaterialPtr material,
                                const RenderPassName& passName );
        
        /// Create a new Mesh as a cube with lower-left vertex at
        /// position and with sides of size.  Render on pass with material.
        RenderablePtr createCube( const glm::vec3& position, 
                                  const glm::vec3& size,
                                  MaterialPtr material,
                                  const RenderPassName& pass );

        /// Create a 2d quad in the z=0 plane.  Useful for HUD-style 
        /// overlays and displaying text.
        RenderablePtr createQuad( const glm::vec2& lowerLeft, 
                                  const glm::vec2& size,
                                  MaterialPtr material, 
                                  const RenderPassName& pass );

        RenderablePtr createLSpark( const glm::vec3& from, const glm::vec3& to,
            float intensity,
            float scale,
            int recursiveDepth,
            float forkProb,
            const RenderPassName& pass, MaterialPtr material );

        FontManagerPtr getFontManager( void );
        
        TextRenderablePtr createText( const std::string& fontName,
                                      int fontSize,
                                      MaterialPtr material,
                                      const RenderPassName& pass,
                                      const std::string& msg );
        
        RenderPassPtr getRenderPassByName( const RenderPassName& name );
        
        /// Release all held resources, release all linked resources.
        void reset( void );
        
    private:
        RenderTargetPtr m_mainRenderTarget;

        ScenePtr m_scene;
        // Managers
        FileAssetFinderPtr m_finder;
        TextureManagerPtr m_textureManager;
        ShaderManagerPtr m_shaderManager;
        FontManagerPtr m_fontManager;
        InputPtr m_input;
        // Common projections
        OrthogonalProjectionPtr m_overlayPerspective;
        PerspectiveProjectionPtr m_cameraPerspective;
        // FBOs
        FrameBufferRenderTargetPtr m_frameBufferTarget;
        GuiEventPublisherPtr m_guiEventPublisher;

    };
    typedef spark::shared_ptr< SceneFacade > SceneFacadePtr;
 }

 #endif
