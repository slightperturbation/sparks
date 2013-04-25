
#ifndef sparks_RENDER_TARGET_HPP
#define sparks_RENDER_TARGET_HPP

#include "SoftTestDeclarations.hpp"

//TODO move to implementation file
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

/// Handles sending the GL rendering output to the correct target.
/// Concrete targets can be the usual framebuffer (FrameBufferRenderTarget),
/// shadow maps texture objects, framebuffer object textures for reflections
/// or full-screen effects.
/// see RenderPass
class RenderTarget
{
public:
    RenderTarget( int aWidth, int aHeight )
    {
        resizeViewport( 0, 0, aWidth, aHeight );
    }
    RenderTarget( int aLeft, int aBottom, int aWidth, int aHeight )
    {
        resizeViewport( aLeft, aBottom, aWidth, aHeight );
    }
    virtual ~RenderTarget() {}
    virtual void resizeViewport( int aLeft, int aBottom, int aWidth, int aHeight )
    {
        m_left = aLeft; m_bottom = aBottom; m_width = aWidth; m_height = aHeight;
    }
    virtual void initialize( TextureManagerPtr& mgr ) = 0;
    virtual void preRender( void ) const = 0;
    virtual void postRender( void ) const = 0;
protected:
    int m_left;
    int m_bottom;
    int m_width;
    int m_height;
};

/// Draws to the default display framebuffer.
class FrameBufferRenderTarget : public RenderTarget
{
public:
    FrameBufferRenderTarget( int aWidth, int aHeight ) 
        : RenderTarget( aWidth, aHeight ) { }
    FrameBufferRenderTarget( int aLeft, int aBottom, int aWidth, int aHeight ) 
        : RenderTarget(  aLeft, aBottom, aWidth, aHeight ) { }
    virtual ~FrameBufferRenderTarget() {}
    virtual void initialize( TextureManagerPtr& mgr ) {}
    virtual void preRender( void ) const
    {
        // Bind to zero is a magic number to draw to the display FB
        glBindFramebuffer( GL_FRAMEBUFFER, 0 ); 
        glViewport( m_left, m_bottom, m_width, m_height );

        glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }
    virtual void postRender( void ) const
    {
    }
};

//class TextureTarget : public RenderTarget
//{
//
//};

#endif
