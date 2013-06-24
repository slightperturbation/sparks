#include "RenderTarget.hpp"
#include "TextureManager.hpp"

////////////////////////////////////////////////////////////
// RenderTarget
std::ostream&
spark::operator<<( std::ostream& out,
                   RenderTargetPtr target )
{
    return target->debugInfo( out );
}

////////////////////////////////////////////////////////////
// FrameBufferRenderTarget

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

glm::vec2
spark::FrameBufferRenderTarget
::size( void ) const
{
    return glm::vec2( m_width, m_height );
}

void
spark::FrameBufferRenderTarget
::preRender( void ) const
{
    LOG_TRACE(g_log) << "FrameBufferRenderTarget::preRender() to "
        << "framebufferId=0";
    // Bind to zero is a magic number to draw to the display FB
    GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, 0 ) );
    glViewport( m_left, m_bottom, m_width, m_height );
}

void
spark::FrameBufferRenderTarget
::postRender( void ) const
{
}

void
spark::FrameBufferRenderTarget
::startFrame( void ) const
{
    LOG_TRACE(g_log) << "TextureRenderTarget::startFrame(): clearing framebufferId = 0";

    // Bind to zero is a magic number to draw to the display FB
    GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, 0 ) );
    glViewport( m_left, m_bottom, m_width, m_height );
    glClearColor( m_clearColor[0],
                  m_clearColor[1],
                  m_clearColor[2],
                  m_clearColor[3] );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

std::ostream&
spark::FrameBufferRenderTarget
::debugInfo( std::ostream& out ) const
{
    out << "FrameBufferRenderTarget[" << m_left << ", " << m_bottom
        << ", " << m_width << ", " << m_height << "]";
    return out;
}


////////////////////////////////////////////////////////////
// TextureRenderTarget

spark::TextureRenderTarget
::TextureRenderTarget( const TextureName& aName,
                       int aWidth, int aHeight )
: m_textureManager(), m_textureHandle( aName ), 
  m_width( aWidth ), m_height( aHeight ),
  m_framebufferId( -1 ), m_depthRenderbufferId( -1 )
{ }

void
spark::TextureRenderTarget
::initialize( TextureManagerPtr& mgr )
{
    LOG_TRACE(g_log) << "Initialize TextureRenderTarget \""
    << getTextureName() << "\".";
    m_textureManager = mgr;
    glGenFramebuffers( 1, &m_framebufferId );
    if( m_framebufferId == -1 )
    {
        // how to test failure?
        LOG_ERROR(g_log) << "Unable to allocate framebuffer.";
    }
    GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId ) );
    
    m_textureManager->createTargetTexture( m_textureHandle, m_width, m_height );
    
    // Setup render target for depth buffer
    glGenRenderbuffers( 1, &m_depthRenderbufferId );
    GL_CHECK( glBindRenderbuffer( GL_RENDERBUFFER, m_depthRenderbufferId ) );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                           m_width, m_height );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER, m_depthRenderbufferId);
    
    // Set target texture as our color attachement #0
    glFramebufferTexture( GL_FRAMEBUFFER,
                          GL_COLOR_ATTACHMENT0,
                          mgr->getTextureIdForHandle( m_textureHandle ),
                          0 );
    
    //// Set the draw buffer for this framebuffer
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, drawBuffers ); // "1" is the size of DrawBuffers
    
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
        LOG_ERROR(g_log) << "Couldn't set renderbuffer for TextureRenderTarget \""
        << m_textureHandle << "\".";
    }
}

glm::vec2
spark::TextureRenderTarget
::size( void ) const
{
    return glm::vec2( m_width, m_height );
}

void
spark::TextureRenderTarget
::preRender( void ) const
{
    LOG_TRACE(g_log) << "TextureRenderTarget::preRender() to framebufferId="
                     << m_framebufferId << " name=\""
                     << getTextureName() << "\".";
    // Render to our framebuffer
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    glViewport( 0, 0, m_width, m_height );
}

void
spark::TextureRenderTarget
::postRender( void ) const
{
    LOG_TRACE(g_log) << "Generate Mipmap for TextureRenderTarget \"" 
                     << name() << "\".";
    if( !m_textureManager )
    {
        LOG_ERROR(g_log) << "TextureRenderTarget (" 
            << name() << " not initialized (null TextureManager) -- "
            << "no mipmaps will be generated.";
        return;
    }
    GLint texUnit = m_textureManager->getTextureUnitForHandle( getTextureName() );
    // Make the target texture active
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + texUnit ) );
    // generate using the active texture unit
    glGenerateMipmap( GL_TEXTURE_2D );
}

void
spark::TextureRenderTarget
::startFrame( void ) const
{
    LOG_TRACE(g_log) << "TextureRenderTarget::startFrame(): clearing framebufferId = " 
        << m_framebufferId;
    if( m_framebufferId == -1 )
    {
        // TODO Create a structured exception for uninitialized
        throw "TextureRenderTarget not initialized or OpenGL error occurred.";
    }
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    glViewport( 0, 0, m_width, m_height );

    //TODO setup rendering to depth buffer!

    glClearColor( m_clearColor[0],
                  m_clearColor[1],
                  m_clearColor[2],
                  m_clearColor[3] );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


std::ostream&
spark::TextureRenderTarget
::debugInfo( std::ostream& out ) const
{
    out << "TextureRenderTarget[Texture=\"" << m_textureHandle << "\"@"
    << m_width << ", " << m_height << "]";
    return out;
}

//////////////////////////////////////////////////////////////////////////////
// ScaledTextureRenderTarget

spark::ScaledTextureRenderTarget
::ScaledTextureRenderTarget( const TextureName& aName,
                             int aViewportWidth, int aViewportHeight,
                             float aScaleX, float aScaleY )
: TextureRenderTarget( aName, aViewportWidth*aScaleX, aViewportHeight*aScaleY ),
  m_scaleX( aScaleX ), m_scaleY( aScaleY )
{
}

void
spark::ScaledTextureRenderTarget
::initialize( TextureManagerPtr& mgr )
{
    m_textureManager = mgr;
    resetTexture();
}

void
spark::ScaledTextureRenderTarget
::resizeViewport( int left, int bottom,
                            int width, int height )
{
    m_width = width * m_scaleX;
    m_height = height * m_scaleY;
    resetTexture();
}

void
spark::ScaledTextureRenderTarget
::scaleFactor( float x, float y )
{
    m_scaleX = x;
    m_scaleY = y;
    resetTexture();
}

void
spark::ScaledTextureRenderTarget
::resetTexture( void )
{
    if( !m_textureManager )
    {
        LOG_ERROR(g_log) << "ScaledTextureRenderTarget[\""
            << name()
            << "\"]::resetTexture called before initializing.";
        m_framebufferId = -1;
        m_depthRenderbufferId = -1;
        return;
    }
    
    LOG_TRACE(g_log) << "Initialize TextureRenderTarget \""
    << getTextureName() << "\".";
    glGenFramebuffers( 1, &m_framebufferId );
    if( m_framebufferId == -1 )
    {
        // how to test failure?
        LOG_ERROR(g_log) << "Unable to allocate framebuffer.";
    }
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    
    m_textureManager->createTargetTexture( m_textureHandle, m_width, m_height );
    
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
                         m_textureManager->getTextureIdForHandle( m_textureHandle ),
                         0 );
    
    // Set the list of draw buffers.
    // TODO -- depth?
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, drawBuffers );
    
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
        LOG_ERROR(g_log) << "Couldn't set renderbuffer for TextureRenderTarget \""
        << m_textureHandle << "\".";
    }
}

std::ostream&
spark::ScaledTextureRenderTarget
::debugInfo( std::ostream& out ) const
{
    out << "ScaledTextureRenderTarget[Texture=\"" << m_textureHandle << "\"@"
    << m_width << ", " << m_height << ", scale="
    << m_scaleX << "," << m_scaleY << "]";
    return out;    
}



