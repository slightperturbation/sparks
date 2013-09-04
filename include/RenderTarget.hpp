
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
        virtual std::string name( void ) const = 0;
        virtual glm::vec2 size( void ) const = 0;
        virtual void initialize( TextureManagerPtr& mgr ) = 0;
        virtual void preRender( void ) const = 0;
        virtual void postRender( void ) const = 0;
        virtual void startFrame( void ) const = 0;
        virtual void resizeViewport( int left, int bottom,
            int width, int height ) override {}
        void setClearColor( const glm::vec4& c ) { m_clearColor = c; }
        virtual std::ostream& debugInfo( std::ostream& out ) const { return out; }
        friend std::ostream& operator<<( std::ostream& out,
                                         RenderTargetPtr target );
    protected:
        glm::vec4 m_clearColor;
    };

    /// Draws to the default display framebuffer.
    class FrameBufferRenderTarget 
        : public RenderTarget, 
          public GuiEventSubscriber
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
        enum BufferType { ColorOnly, ColorAndDepth, DepthOnly };

        TextureRenderTarget( const TextureName& aName,
                             int aWidth, int aHeight,
                             BufferType bufferType = ColorAndDepth );
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
        BufferType m_bufferType;
    };
    typedef spark::shared_ptr< TextureRenderTarget > TextureRenderTargetPtr;

    ///////////////////////////////////////////////////////////////////////////
    // ScaledTextureRenderTarget

    /// A ScaledTexureRenderTarget that maintains a size that is a scale factor
    /// of the viewport's size.  Listens to GUI callbacks to keep size linked.
    class ScaledTextureRenderTarget : public TextureRenderTarget,
                                      public GuiEventSubscriber
    {
    public:
        ScaledTextureRenderTarget( const TextureName& aName,
            int aViewportWidth, int aViewportHeight, 
            float aScaleX, float aScaleY,
            BufferType bufferType = ColorAndDepth );
        virtual ~ScaledTextureRenderTarget();

        virtual void initialize( TextureManagerPtr& mgr ) override;

        virtual void resizeViewport( int left, int bottom,
                                    int width, int height ) override;

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
