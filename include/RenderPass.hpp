//
//  RenderPass.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_RenderPass_hpp
#define sparks_RenderPass_hpp

#include "Spark.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>

namespace spark
{
    bool createRenderCommand( RenderCommand& outRC, 
                              ConstRenderPassPtr aRenderPass, 
                              ConstRenderablePtr aRenderable );

    /// Encapsulates a single rendering pass over all of the renderables in
    /// a given scene.  A scene with only a single pass will likely render
    /// directly to the display and render every renderable.  Multiple
    /// passes can be used to implement shadow maps, reflections, full-screen
    /// overlay effects, etc.  In those cases, the target is generally an
    /// OpenGL FrameBufferObject that will be used by subsequent passes.
    /// A RenderPass's role is only to create RenderCommands, not to do 
    /// any rendering itself.
    /// RenderPass must be registered with a Scene object  (via Scene::add)
    /// to be rendered.
    class RenderPass
    {
    public:
        RenderPass( const RenderPassName& aName = "UNLABELED_RENDER_PASS" );
        ~RenderPass();
        void initialize( RenderTargetPtr aTarget, 
                         ProjectionPtr aPerspective );
        void initialize( RenderTargetPtr aTarget, 
                         ProjectionPtr aPerspective,
                         float aPriority );

        ConstMaterialPtr getMaterialForRenderable( ConstRenderablePtr aRenderable ) const;

        RenderPassName name( void ) const;
        std::string targetName( void ) const;
        glm::vec2 targetSize( void ) const;
        void setName( const RenderPassName& aName );

        /// Defines how this pass will blend into its target.
        /// If not set, the pass uses interpolation blending, which is
        /// the same as calling setBlending( GL_SRC_ALPHA, 
        ///                                  GL_ONE_MINUS_SRC_ALPHA, 
        ///                                  GL_FUNC_ADD )
        ///
        /// Some common settings:
        /// interpolation       : setBlending( GL_SRC_ALPHA, 
        ///                                    GL_ONE_MINUS_SRC_ALPHA );
        /// additive (glow)     : setBlending( GL_ONE, GL_ONE );
        /// max                 : setBlending( GL_ONE, GL_ONE, GL_MAX );
        /// 
        /// Note that some blend modes (e.g., interpolation) are heavily 
        /// dependent on the rendering order.
        /// Blending is disabled if equation == (GLenum)-1 
        void setBlending( GLenum sourceFactor, 
                          GLenum destinationFactor,
                          GLenum equation = GL_FUNC_ADD );
        void useAdditiveBlending( void );
        void useInterpolatedBlending( void );
        void useMaxBlending( void );
        void disableBlending( void );
        void enableBlending( void );
        
        /// If defaultMaterial is non-null, it will be used for all scene
        /// renderables without a material assignment for this pass.
        void useDefaultMaterial( ConstMaterialPtr defaultMaterial );
        void useDefaultMaterial( MaterialPtr defaultMaterial );
        ConstMaterialPtr defaultMaterial( void ) const;

        /// If true (default), this pass will test depth before writing.
        /// If false, fragments will not be discarded due to depth occlusion.
        void setDepthTest( bool isDepthTestEnabled );
        bool depthTest( void ) const;
        
        /// If true (default), then this pass
        /// will write normally to the depth buffer.
        /// If false, then the depth buffer will not be changed.
        void setDepthWrite( bool isWritingToDepthBuffer );
        bool depthWrite( void ) const;
        
        void setColorWrite( bool isWritingColor );
        bool colorWrite( void ) const;
        
        /// Passes with higher priority are rendered first.
        float priority( void ) const  { return m_priority; }

        //TODO-implement per-pass render order
        // void setRenderCommandComparison( )
        // void farToNearRenderOrder()
        // void nearToFarRenderOrder()
        
        /// Sets OpenGL state to draw to the render target
        /// (e.g., display device or render-to-texture)
        void preRender(  ConstRenderPassPtr prevPass ) const;
        void postRender( ConstRenderPassPtr nextPass ) const;
        void startFrame( ConstRenderPassPtr prevPass ) const;

        /// Fill the outRC RenderCommand using the pass and renderable 
        /// specified.
        friend bool createRenderCommand( RenderCommand& outRC, 
                                         ConstRenderPassPtr aRenderPass, 
                                         ConstRenderablePtr aRenderable );
        friend std::ostream& operator<<( std::ostream& out,
                                         ConstRenderPassPtr pass );
        friend std::ostream& operator<<( std::ostream& out,
                                         RenderPassPtr pass );
    private:
        RenderPassName m_name;
        RenderTargetPtr m_target;
        ProjectionPtr m_perspective;

        /// Orders pass wrt other passes.  Higher priorities render first.
        float m_priority;

        /// Determines the blending to be used writing in this pass
        bool   m_isBlendingEnabled;
        GLenum m_blendSourceFactor;
        GLenum m_blendDestinationFactor;
        GLenum m_blendEquation;

        /// Can be null, which indicates no default
        /// If null, renderables without a specific material set for this
        /// pass will not be drawn.
        ConstMaterialPtr m_defaultMaterial;

        /// True if the pass has depth test enabled
        bool m_depthTest;
        /// True if the pass writes to the depth buffer
        bool m_depthMask;
        /// True if the pass writes to the color buffer
        bool m_colorMask;
        /// True if the pass should cull backfaces (see m_cullFace)
        bool m_backfaceCulling;
        /// OpenGL enum for facet to cull:  GL_FRONT, GL_BACK, GL_FRONT_AND_BACK
        GLenum m_cullFace;
    };

    bool renderPassCompareByPriority( ConstRenderPassPtr a, 
                                      ConstRenderPassPtr b );
    bool createRenderCommand( RenderCommand& outRC, 
                              ConstRenderPassPtr aRenderPass, 
                              ConstRenderablePtr aRenderable );
} // end namespace spark
#endif
