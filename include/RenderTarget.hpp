
#ifndef sparks_RENDER_TARGET_HPP
#define sparks_RENDER_TARGET_HPP

#include "Spark.hpp"
#include "GuiEventSubscriber.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace spark
{
    /// Handles sending the GL rendering output to the correct target.
    /// Concrete targets can be the usual framebuffer (FrameBufferRenderTarget),
    /// shadow maps texture objects, framebuffer object textures for reflections
    /// or full-screen effects.
    /// RenderTargets are generally held by a RenderPass.
    /// see RenderPass
    class RenderTarget : public GuiEventSubscriber
    {
    public:
        RenderTarget() : m_clearColor( 0,1,0,0 ) {}
        virtual ~RenderTarget();

        /// Returns the debug-friendly name of the RenderTarget.
        virtual std::string name( void ) const = 0;

        /// Returns the size (in pixels) of this target.
        virtual glm::vec2 size( void ) const = 0;

        /// Allow one-time initialization of the RenderTarget.
        virtual void initialize( TextureManagerPtr& mgr ) = 0;

        /// Called just before render to this target, allows setup.
        virtual void preRender( void ) const = 0;

        /// Called after rendering to this target, allows clean-up.
        virtual void postRender( void ) const = 0;

        /// Called when the frame is starting.
        virtual void startFrame( void ) const = 0;

        /// Handle the event of the viewport resizing.
        virtual void resizeViewport( int left, int bottom,
            int width, int height ) override {}

        /// Set the color used for clearing the RenderTarget.
        void setClearColor( const glm::vec4& c ) { m_clearColor = c; }
        virtual std::ostream& debugInfo( std::ostream& out ) const { return out; }
        friend std::ostream& operator<<( std::ostream& out,
                                         RenderTargetPtr target );
    protected:
        glm::vec4 m_clearColor;
    };

    /// Draws to the default display framebuffer.
    class FrameBufferRenderTarget 
        : public RenderTarget
    {
    public:
        FrameBufferRenderTarget( int aWidth, int aHeight );
        FrameBufferRenderTarget( int aLeft, int aBottom, int aWidth, int aHeight );
        virtual ~FrameBufferRenderTarget();
        virtual std::string name( void ) const override
        { return "FrameBuffer"; }
        virtual glm::vec2 size( void ) const override;

        virtual void resizeViewport( int left, int bottom,
                                     int width, int height ) override;

        virtual void initialize( TextureManagerPtr& mgr ) override {}
        virtual void preRender( void ) const override;
        virtual void postRender( void ) const override;
        virtual void startFrame( void ) const override;
        virtual std::ostream& debugInfo( std::ostream& out ) const override;
        
        int left() const { return m_left; }
        int bottom() const { return m_bottom; }
        int width() const { return m_width; }
        int height() const { return m_height; }
    protected:
        int m_left;
        int m_bottom;
        int m_width;
        int m_height;
    };
    typedef spark::shared_ptr< FrameBufferRenderTarget > FrameBufferRenderTargetPtr;

    //////////////////////////////////////////////////////////////////////////////
    // TextureRenderTarget

    /// Directs render output to a fixed-size texture.
    class TextureRenderTarget 
        : public RenderTarget
    {
    public:
        TextureRenderTarget( const TextureName& aName,
                             int aWidth, int aHeight );
        virtual ~TextureRenderTarget();
        virtual std::string name( void ) const override
        { return getTextureName(); }
        const TextureName& getTextureName( void ) const 
        { return m_textureHandle; }
        virtual glm::vec2 size( void ) const override;

        virtual void initialize( TextureManagerPtr& mgr ) override;
        virtual void preRender( void ) const override;
        virtual void postRender( void ) const override;
        virtual void startFrame( void ) const override;

        virtual std::ostream& debugInfo( std::ostream& out ) const override;
    protected:
        TextureManagerPtr m_textureManager;
        TextureName m_textureHandle;
        int m_width;
        int m_height;
        GLuint m_framebufferId;
        GLuint m_depthRenderbufferId;
    };
    typedef spark::shared_ptr< TextureRenderTarget > TextureRenderTargetPtr;

    ///////////////////////////////////////////////////////////////////////////
    // DepthMapRenderTarget
    class DepthMapRenderTarget
    : public RenderTarget
    {
    public:
        DepthMapRenderTarget( const TextureName& aName,
                            int aWidth, int aHeight );
        virtual ~DepthMapRenderTarget();
        virtual std::string name( void ) const override
        { return getTextureName(); }
        const TextureName& getTextureName( void ) const
        { return m_textureHandle; }
        virtual glm::vec2 size( void ) const override;
        
        virtual void initialize( TextureManagerPtr& mgr ) override;
        virtual void preRender( void ) const override;
        virtual void postRender( void ) const override;
        virtual void startFrame( void ) const override;
        
        virtual std::ostream& debugInfo( std::ostream& out ) const override;
    protected:
        TextureManagerPtr m_textureManager;
        TextureName m_textureHandle;
        int m_width;
        int m_height;
        GLuint m_framebufferId;
        GLuint m_depthRenderbufferId;
    };
    typedef spark::shared_ptr< DepthMapRenderTarget > DepthMapRenderTargetPtr;
    
    ///////////////////////////////////////////////////////////////////////////
    // ScaledTextureRenderTarget

    /// A ScaledTexureRenderTarget that maintains a size that is a scale factor
    /// of the viewport's size.  Listens to GUI callbacks to keep size linked.
    class ScaledTextureRenderTarget : public TextureRenderTarget
    {
    public:
        ScaledTextureRenderTarget( const TextureName& aName,
            int aViewportWidth, int aViewportHeight, 
            float aScaleX, float aScaleY );
        virtual ~ScaledTextureRenderTarget();

        virtual void initialize( TextureManagerPtr& mgr ) override;

        virtual void resizeViewport( int left, int bottom,
                                    int width, int height ) override;

        /// Set the scale factor, making the target texture this fraction
        /// of the current viewport.
        void scaleFactor( float x, float y );

        virtual std::ostream& debugInfo( std::ostream& out ) const override;
    private:
        void resetTexture( void );
        float m_scaleX;                //< Scale factor vs viewport size
        float m_scaleY;                //< Scale factor vs viewport size
    };
    typedef spark::shared_ptr< ScaledTextureRenderTarget >
        ScaledTextureRenderTargetPtr;


    void checkFramebufferStatus( const TextureName& textureHandle );
} // end namespace spark
#endif
