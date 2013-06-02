
#ifndef sparks_RENDER_TARGET_HPP
#define sparks_RENDER_TARGET_HPP

#include "SoftTestDeclarations.hpp"
#include "GuiEventSubscriber.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

namespace spark
{
    /// Handles sending the GL rendering output to the correct target.
    /// Concrete targets can be the usual framebuffer (FrameBufferRenderTarget),
    /// shadow maps texture objects, framebuffer object textures for reflections
    /// or full-screen effects.
    /// see RenderPass
    class RenderTarget
    {
    public:
        virtual ~RenderTarget() {}
        virtual void initialize( TextureManagerPtr& mgr ) = 0;
        virtual void preRender( void ) const = 0;
        virtual void postRender( void ) const = 0;
    };

    /// Draws to the default display framebuffer.
    class FrameBufferRenderTarget
    : public RenderTarget, public GuiEventSubscriber
    {
    public:
        FrameBufferRenderTarget( int aWidth, int aHeight );

        FrameBufferRenderTarget( int aLeft, int aBottom, int aWidth, int aHeight );
        virtual ~FrameBufferRenderTarget() {}

        virtual void resizeViewport( int left, int bottom,
                                     int width, int height ) override;
        virtual void initialize( TextureManagerPtr& mgr ) override {}
        virtual void preRender( void ) const override;
        virtual void postRender( void ) const override;
    protected:
        int m_left;
        int m_bottom;
        int m_width;
        int m_height;
    };
    typedef std::shared_ptr< FrameBufferRenderTarget > FrameBufferRenderTargetPtr;

    /// Draws to the default display framebuffer.
    class TextureRenderTarget : public RenderTarget
    {
    public:
        TextureRenderTarget( const TextureName& aName,
                             int aWidth, int aHeight );
        virtual ~TextureRenderTarget() {}
        const TextureName& getTextureName( void ) const 
        { return m_textureHandle; }
        
        virtual void initialize( TextureManagerPtr& mgr ) override;
        virtual void preRender( void ) const override;
        virtual void postRender( void ) const override;
    private:
        TextureName m_textureHandle;
        int m_width;
        int m_height;
        GLuint m_framebufferId;
        GLuint m_depthRenderbufferId;
    };
    typedef std::shared_ptr< TextureRenderTarget > TextureRenderTargetPtr;
    
    // class TextureTarget ??
} // end namespace spark
#endif
