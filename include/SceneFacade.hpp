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
                     OpenGLWindow* window,
                      FileAssetFinderPtr finder,
                      TextureManagerPtr tm,
                      ShaderManagerPtr sm,
                      PerspectiveProjectionPtr camera,
                      FrameBufferRenderTargetPtr frameBufferTarget,
                      InputPtr inputManager,
                      FontManagerPtr fontManager,
                      AudioManagerPtr audioManager,
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
        
        /// Returns a new orthographic projection.
        /// NOTE this returns a ProjectionPtr instead of an OrthogonalProjectionPtr
        /// due to createRenderPassWithProjection() not accepting the 
        /// subclass when called from Lua.  (Lua throws a runtime error
        /// of "No matching overload candidates found".)
        /// Until resolved, the Lua caller needs a "pure" ProjectionPtr
        ProjectionPtr createOrthogonalProjection( float left,
                                                  float right,
                                                  float bottom,
                                                  float top,
                                                  float nearPlane,
                                                  float farPlane,
                                                  glm::vec3 direction );

        /// Requires that proj be an OrthogonalProjectionPtr
        /// Modifies the orthogonalProjection with the new parameters.
        void updateOrthogonalProjection( ProjectionPtr proj, 
                                         float left, float right,
                                         float bottom, float top,
                                         float nearPlane, float farPlane,
                                         glm::vec3 direction );

        /// Returns a camera with the eye at pos, looking at target.
        /// See note for createOrthogonalProjection for why this returns a ProjectionPtr.
        ProjectionPtr createPerspectiveProjection( const glm::vec3& pos,
                                                              const glm::vec3& target,
                                                              const glm::vec3& up,
                                                              float fov,
                                                              float nearPlane,
                                                              float farPlane );

        /// Return the default/framebuffer render target
        /// DEPRECATED
        RenderTargetPtr getMainRenderTarget( void );
        void setMainRenderTarget( RenderTargetPtr target );
        
        /// Return the pass with given name.  Returns a null if not found.
        RenderPassPtr getRenderPass( const RenderPassName& name );
        
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
        RenderTargetPtr createDepthMapRenderTarget(
            const TextureName& depthTextureName,
            int width, int height );

        /// Creates newTargetTextureName as a texture and renders to it
        /// using an overlay projection
        /// \todo need a better name!
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

        /// Load the mesh file from filename and assign the given material
        /// and updater to modify the mesh as needed.
        /// NOTE -- this is only an example of how to add a class, 
        /// probably not very useful.  To make updateable meshes,
        /// probably want to pass in a client class that knows how to do
        /// the actual updating.
        RenderablePtr loadUpdateableMesh( const char* filename,
                                          MaterialPtr material,
                                          const RenderPassName& passName );
        
        /// Create a new Mesh as a cube with lower-left vertex at
        /// position and with sides of size.  Render on pass with material.
        RenderablePtr createCube( const glm::vec3& position, 
                                  float size,
                                  MaterialPtr material,
                                  const RenderPassName& pass );

        /// Create a 2d quad in the z=0 plane.  Assumes for HUD-style
        /// overlays and displaying text, as it sets requireExplicitMaterial
        /// to true.
        RenderablePtr createQuad( const glm::vec2& lowerLeft, 
                                  const glm::vec2& size,
                                  MaterialPtr material, 
                                  const RenderPassName& pass );
        
        /// Create a plane segment centered at center with normal (0,1,0)
        /// Is added to the scene, attached to material for pass RenderPass.
        RenderablePtr createPlane( const glm::vec3& center,
                                   const glm::vec2& scale,
                                   const glm::ivec2& subdivisions,
                                   MaterialPtr material,
                                   const RenderPassName& pass );
        
        /// Create the simulation tissue
        TissueMeshPtr createTissue( const RenderableName& name,
                                    float lengthInMeters,
                                    size_t numVoxelsPerDim );

        TexturedSparkRenderablePtr createLSpark( const glm::vec3& from, const glm::vec3& to,
                                    float intensity,
                                    float scale,
                                    int recursiveDepth,
                                    float forkProb,
                                    const RenderPassName& pass, 
                                    MaterialPtr material );

        /// Returns the fraction of the pixels of the given depth texture 
        /// with values between lowerBound and upperBound.
        float calculateAreaOfTexture( const TextureName& name, 
                                      float lowerBound, 
                                      float upperBound );

        FontManagerPtr getFontManager( void );
        
        TextRenderablePtr createText( const std::string& fontName,
                                      int fontSize,
                                      MaterialPtr material,
                                      const RenderPassName& pass,
                                      const std::string& msg );
        
        RenderPassPtr getRenderPassByName( const RenderPassName& name );
        
        glm::vec2 pixelsToScreenCoords( const glm::vec2& pixelPosition );
        glm::vec2 screenCoordsToPixels( const glm::vec2& screenCoord );

        glm::vec2 getWindowSize( void );
        
        void playSound( void );
        void stopSound( void );
        
        /// Release all held resources, release all linked resources.
        void reset( void );
        
    private:
        RenderTargetPtr m_mainRenderTarget;
        
        OpenGLWindow* m_window;

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
        AudioManagerPtr m_audioManager;
        GuiEventPublisherPtr m_guiEventPublisher;

    };
    typedef spark::shared_ptr< SceneFacade > SceneFacadePtr;
 }

 #endif
