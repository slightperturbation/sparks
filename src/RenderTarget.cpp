#include "RenderTarget.hpp"
#include "TextureManager.hpp"

////////////////////////////////////////////////////////////
// RenderTarget
spark::RenderTarget
::~RenderTarget()
{
    LOG_DEBUG(g_log) << "Dtor -- RenderTarget.";
}

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

spark::FrameBufferRenderTarget
::~FrameBufferRenderTarget()
{
    LOG_DEBUG(g_log) << "Dtor -- FrameBufferRenderTarget \"" << name() << "\".";
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
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "FrameBufferRenderTarget::preRender() to "
            << "framebufferId=0";
    }
    // Bind to zero is a magic number to draw to the display FB
    GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, 0 ) );
    checkFramebufferStatus( "FrameBuffer0" );
    glViewport( m_left, m_bottom, m_width, m_height );
    glEnable( GL_SCISSOR_TEST );
    glScissor( m_left, m_bottom, m_width, m_height );
}

void
spark::FrameBufferRenderTarget
::postRender( void ) const
{
    glDisable( GL_SCISSOR_TEST );
}

void
spark::FrameBufferRenderTarget
::startFrame( void ) const
{
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "TextureRenderTarget::startFrame(): clearing framebufferId = 0";
    }
    // Bind to zero is a magic number to draw to the display FB
    GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, 0 ) );
    checkFramebufferStatus( "FrameBuffer0" );
    glViewport( m_left, m_bottom, m_width, m_height );
    
    // Limit clearing to this viewport
    glEnable( GL_SCISSOR_TEST );
    glScissor( m_left, m_bottom, m_width, m_height );
    
    glClearColor( m_clearColor[0],
                  m_clearColor[1],
                  m_clearColor[2],
                  m_clearColor[3] );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // glClear is done, can disable the scissoring until preRender
    glDisable( GL_SCISSOR_TEST );
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

spark::TextureRenderTarget
::~TextureRenderTarget()
{
    LOG_DEBUG(g_log) << "Dtor -- TextureRenderTarget \"" << name() << "\".";
}

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
    // Need a depth-buffer if you want to render with z-buffer-testing
    glGenRenderbuffers( 1, &m_depthRenderbufferId );
    GL_CHECK( glBindRenderbuffer( GL_RENDERBUFFER, m_depthRenderbufferId ) );
    GL_CHECK( glRenderbufferStorage( GL_RENDERBUFFER,
                                     GL_DEPTH_COMPONENT24,
                                     m_width, m_height ) );
    GL_CHECK( glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                                         GL_DEPTH_ATTACHMENT,
                                         GL_RENDERBUFFER,
                                         m_depthRenderbufferId ) );

    // Set target texture as our color attachment #0
    glFramebufferTexture( GL_FRAMEBUFFER,
                          GL_COLOR_ATTACHMENT0,
                          mgr->getTextureIdForHandle( m_textureHandle ),
                          0 );
    
    //// Set the draw buffer for this framebuffer
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, drawBuffers ); // "1" is the size of DrawBuffers
    
    checkFramebufferStatus( getTextureName() );
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
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "TextureRenderTarget::preRender() to framebufferId="
                     << m_framebufferId << " name=\""
                     << getTextureName() << "\".";
    }
    // Render to our framebuffer
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    checkFramebufferStatus( getTextureName() );
    glViewport( 0, 0, m_width, m_height );
}

void
spark::TextureRenderTarget
::postRender( void ) const
{
    if( !m_textureManager )
    {
        LOG_ERROR(g_log) << "TextureRenderTarget (" 
            << name() << " not initialized (null TextureManager) -- "
            << "no mipmaps will be generated.";
        return;
    }
    GLint texUnit = m_textureManager->getTextureUnitForHandle( getTextureName() );
    LOG_TRACE(g_log) << "TextureRenderTarget activating texture unit GL_TEXTURE0 + " 
        << texUnit;
    // Make the target texture active
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + texUnit ) );
    // generate using the active texture unit
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "Generate Mipmap for TextureRenderTarget \"" 
            << name() << "\".";
    }
    glGenerateMipmap( GL_TEXTURE_2D );
}

void
spark::TextureRenderTarget
::startFrame( void ) const
{
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "TextureRenderTarget::startFrame(): clearing framebufferId = " 
            << m_framebufferId;
    }
    if( m_framebufferId == -1 )
    {
        // TODO Create a structured exception for uninitialized
        throw "TextureRenderTarget not initialized or OpenGL error occurred.";
    }
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    checkFramebufferStatus( m_textureHandle );
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

///////////////////////////////////////////////////////////////////////////
// DepthMapRenderTarget

spark::DepthMapRenderTarget
::DepthMapRenderTarget( const TextureName& aName,
                      int aWidth, int aHeight )
: m_textureManager(), m_textureHandle( aName ),
m_width( aWidth ), m_height( aHeight ),
m_framebufferId( -1 ), m_depthRenderbufferId( -1 )
{ }

spark::DepthMapRenderTarget
::~DepthMapRenderTarget()
{
    LOG_DEBUG(g_log) << "Dtor -- DepthMapRenderTarget \"" << name() << "\".";
}

void
spark::DepthMapRenderTarget
::initialize( TextureManagerPtr& mgr )
{
    LOG_TRACE(g_log) << "Initialize DepthMapRenderTarget \""
    << getTextureName() << "\".";
    m_textureManager = mgr;
    glGenFramebuffers( 1, &m_framebufferId );
    if( m_framebufferId == -1 )
    {
        // how to test failure?
        LOG_ERROR(g_log) << "Unable to allocate framebuffer.";
    }
    GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId ) );
    
    m_textureManager->createDepthTargetTexture( m_textureHandle, m_width, m_height );
    // Attach this texture to a FBO target (depth)
    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D,
                           mgr->getTextureIdForHandle( m_textureHandle ),
                           0 );
    glDrawBuffer( GL_NONE ); // No color buffer is drawn to.

    checkFramebufferStatus( getTextureName() );
}

glm::vec2
spark::DepthMapRenderTarget
::size( void ) const
{
    return glm::vec2( m_width, m_height );
}

void
spark::DepthMapRenderTarget
::preRender( void ) const
{
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "DepthMapRenderTarget::preRender() to framebufferId="
        << m_framebufferId << " name=\""
        << getTextureName() << "\".";
    }
    // Render to our framebuffer
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    checkFramebufferStatus( getTextureName() );
    glViewport( 0, 0, m_width, m_height );
}

void
spark::DepthMapRenderTarget
::postRender( void ) const
{
    if( !m_textureManager )
    {
        LOG_ERROR(g_log) << "DepthMapRenderTarget ("
        << name() << " not initialized (null TextureManager) -- "
        << "no mipmaps will be generated.";
        return;
    }
    //GLint texUnit = m_textureManager->getTextureUnitForHandle( getTextureName() );
    //LOG_TRACE(g_log) << "DepthMapRenderTarget activating texture unit GL_TEXTURE0 + "
    //<< texUnit;
    //// Make the target texture active
    //GL_CHECK( glActiveTexture( GL_TEXTURE0 + texUnit ) );
    //// generate using the active texture unit
    //if( g_log->isTrace() )
    //{
    //    LOG_TRACE(g_log) << "Generate Mipmap for DepthMapRenderTarget \""
    //    << name() << "\".";
    //}
    //glGenerateMipmap( GL_TEXTURE_2D );
}

void
spark::DepthMapRenderTarget
::startFrame( void ) const
{
    // called at the beginning of the frame, before any rendering

    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "DepthMapRenderTarget::startFrame(): clearing framebufferId = "
        << m_framebufferId;
    }
    if( m_framebufferId == -1 )
    {
        // TODO Create a structured exception for uninitialized
        throw "DepthMapRenderTarget not initialized or OpenGL error occurred.";
    }
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    checkFramebufferStatus( m_textureHandle );
    glViewport( 0, 0, m_width, m_height );
    glClear( GL_DEPTH_BUFFER_BIT );

    glClearColor( 0,0,0,0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


std::ostream&
spark::DepthMapRenderTarget
::debugInfo( std::ostream& out ) const
{
    out << "DepthMapRenderTarget[Texture=\"" << m_textureHandle << "\"@"
    << m_width << ", " << m_height << "]";
    return out;
}

//////////////////////////////////////////////////////////////////////////////
// ScaledTextureRenderTarget

spark::ScaledTextureRenderTarget
::ScaledTextureRenderTarget( const TextureName& aName,
                             int aViewportWidth, int aViewportHeight,
                             float aScaleX, float aScaleY )
: TextureRenderTarget( aName, 
                       aViewportWidth*aScaleX, aViewportHeight*aScaleY ),
  m_scaleX( aScaleX ), m_scaleY( aScaleY )
{
}

spark::ScaledTextureRenderTarget
::~ScaledTextureRenderTarget()
{
    LOG_DEBUG(g_log) << "Dtor - ScaledTextureRenderTarget: \"" << name() << "\".";
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
    
    LOG_TRACE(g_log) << "Resetting Texture for scaled render target: \""
    << getTextureName() << "\".";
    if( m_framebufferId == -1 )
    {
        LOG_TRACE(g_log) << "Allocating new Framebuffer ID \""
            << getTextureName() << "\".";
        glGenFramebuffers( 1, &m_framebufferId );
        if( m_framebufferId == -1 )
        {
            LOG_ERROR(g_log) << "Unable to allocate framebuffer.";
        }
    }
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferId );
    m_textureManager->createTargetTexture( m_textureHandle, m_width, m_height );
    
    
    // TODO Honor BufferType
    
    // Setup render target for depth buffer
    if( m_depthRenderbufferId == -1 )
    {
        glGenRenderbuffers( 1, &m_depthRenderbufferId );
    }
    glBindRenderbuffer( GL_RENDERBUFFER, m_depthRenderbufferId );
    // renderbufferStorage will delete any existing data
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          m_width, m_height );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthRenderbufferId);
    
    // Set target texture as our color attachment #0
    glFramebufferTexture( GL_FRAMEBUFFER,
                         GL_COLOR_ATTACHMENT0,
                         m_textureManager->getTextureIdForHandle( m_textureHandle ),
                         0 );
    
    // Set the list of draw buffers.
    // TODO -- depth?
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, drawBuffers );
    
    checkFramebufferStatus( m_textureHandle );
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

//////////////////////////////////////////////////////////////////////////////
// Functions
void
spark
::checkFramebufferStatus( const TextureName& textureHandle )
{
    GLenum fbStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( fbStatus != GL_FRAMEBUFFER_COMPLETE )
    {
        std::stringstream msg;
        switch( fbStatus )
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                msg << "target is the default framebuffer, but the default framebuffer does not exist.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                msg << "any of the framebuffer attachment points are framebuffer incomplete.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                msg << "the framebuffer does not have at least one image attached to it.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                msg << "the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                msg << "GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                msg << "the combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                msg << "the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                msg << "any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target.";
                break;
        }
        
        LOG_ERROR(g_log) << "Couldn't set renderbuffer for TextureRenderTarget \""
        << textureHandle << "\".";
        LOG_ERROR(g_log) << "GL Error: " << msg.str();
    }
}
