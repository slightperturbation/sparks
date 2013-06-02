#include "RenderTarget.hpp"

#include "TextureManager.hpp"

spark::FrameBufferRenderTarget
::FrameBufferRenderTarget( int aWidth, int aHeight )
{
    resizeViewport( 0, 0, aWidth, aHeight );
}

spark::FrameBufferRenderTarget
::FrameBufferRenderTarget( int aLeft, int aBottom,
                           int aWidth, int aHeight )
{
    resizeViewport( aLeft, aBottom, aWidth, aHeight );
}

void
spark::FrameBufferRenderTarget
::resizeViewport( int aLeft, int aBottom, int aWidth, int aHeight )
{
    m_left = aLeft; m_bottom = aBottom; m_width = aWidth; m_height = aHeight;
}

void
spark::FrameBufferRenderTarget
::preRender( void ) const
{
    // Bind to zero is a magic number to draw to the display FB
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glViewport( m_left, m_bottom, m_width, m_height );
}

void
spark::FrameBufferRenderTarget
::postRender( void ) const
{
}

////////////////////////////////////////////////////////////
//TextureRenderTarget


spark::TextureRenderTarget
::TextureRenderTarget( const TextureName& aName,
                       int aWidth, int aHeight )
: m_textureHandle( aName ), m_width( aWidth ), m_height( aHeight )
{ }

void
spark::TextureRenderTarget
::initialize( TextureManagerPtr& mgr )
{
    glGenFramebuffers( 1, &m_framebufferId );
    if( m_framebufferId == -1 )
    {
        LOG_ERROR(g_log) << "Unable to allocate framebuffer.";
    }
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    
    mgr->createTargetTexture( m_textureHandle, m_width, m_height );
    
    // Setup render target for depth buffer
    glGenRenderbuffers( 1, &m_depthRenderbufferId );
    glBindRenderbuffer( GL_RENDERBUFFER, m_depthRenderbufferId );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                           m_width, m_height );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER, m_depthRenderbufferId);
    
    // Set target texture as our color attachement #0
    glFramebufferTexture( GL_FRAMEBUFFER,
                          GL_COLOR_ATTACHMENT0,
                          mgr->getTextureIdForHandle( m_textureHandle ),
                          0 );
    
    //// Set the list of draw buffers.
    //GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0 };
    //glDrawBuffers( 1, drawBuffers ); // "1" is the size of DrawBuffers
    //
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
        LOG_ERROR(g_log) << "Couldn't set renderbuffer for TextureRenderTarget \""
        << m_textureHandle << "\".";
    }
    
}

void
spark::TextureRenderTarget
::preRender( void ) const
{
    // Render to our framebuffer
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    glViewport( 0, 0, m_width, m_height );

    glClearColor( 1,1,0, 0 ); // alpha of zero!
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void
spark::TextureRenderTarget
::postRender( void ) const
{
    
}




