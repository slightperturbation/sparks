
#include "TextureManager.hpp"
#include "VolumeData.hpp"

#include <boost/thread/locks.hpp>

#include <iostream>

spark::TextureManager
::TextureManager( void )
: m_finder( new FileAssetFinder() )
{
    GL_CHECK( glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTextureUnits ) );
    // set next to a middle value to catch errors using default unit 0
    m_nextAvailableTextureUnit = 0;
}

spark::TextureManager
::~TextureManager()
{
    releaseAll();
}

void
spark::TextureManager
::deleteTexture( const TextureName& aHandle )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    auto iter = m_registry.find( aHandle );
    if( iter == m_registry.end() )
    {
        return;
    }
    GLuint textureId = iter->second;
    if( textureId == -1 )
    {
        return;
    }
    glDeleteTextures( 1, &textureId );
    m_registry.erase( aHandle );
    m_textureType.erase( textureId );
    m_paths.erase( aHandle );

    auto bindIter = m_bindingTextureUnitToTextureId.begin();
    while( bindIter != m_bindingTextureUnitToTextureId.end() )
    {
        if( bindIter->second == textureId )
        {
            bindIter = m_bindingTextureUnitToTextureId.erase( bindIter );
        }
        else
        {
            ++bindIter;
        }
    }
}

void
spark::TextureManager
::createTargetTexture( const TextureName& aHandle,
                      int width, int height )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    GLuint textureId;
    if( exists(aHandle) )
    {
        // Texture already exists-- delete current texture and
        // recreate new texture
          activateTextureUnitForHandle( aHandle );
          textureId = getTextureIdForHandle( aHandle );
    }
    else
    {
        // need to create a new texture
        GL_CHECK( glGenTextures( 1, &textureId ) );
        if( -1 == textureId )
        {
            LOG_ERROR(g_log) << "OpenGL failed to allocate a texture id.";
            assert(false);
            return;
        }
        m_registry[aHandle] = textureId;
        GLint textureUnit = reserveTextureUnit();
        bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );
    }
    //<-- texture unit has been activated and textureId != -1
    assert( textureId != -1 );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); 

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
    
    //    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR ); 
    //    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
 

    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0 );
}

void
spark::TextureManager
::createDepthTargetTexture( const TextureName& aHandle,
                              int width, int height )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    GLuint textureId;
    deleteTexture( aHandle );
    GL_CHECK( glGenTextures( 1, &textureId ) );
    if( -1 == textureId )
    {
        LOG_ERROR(g_log) << "OpenGL failed to allocate a texture id.";
        assert(false);
        return;
    }
    m_registry[aHandle] = textureId;
    GLint textureUnit = reserveTextureUnit();
    bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
    
    // GL_LINEAR for simple shadow maps (make an argument?)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    
    // Remove artifact on the edges of the shadowmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void
spark::TextureManager
::loadTestTexture( const TextureName& aHandle, GLint textureUnit )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    if( textureUnit == -1 )
    {
        textureUnit = reserveTextureUnit();
    }
    GLuint textureId = -1;
    GL_CHECK( glGenTextures( 1, &textureId ) );
    if( -1 == textureId )
    {
        LOG_ERROR(g_log) << "Failed to get a texture id!";
        assert(false);
        return;
    }
    m_registry[aHandle] = textureId;


    bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );

    spark::loadTestTexture();
    GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
    //Set default texture state
    //TODO-- Can be overridden by TextureUnit?
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR ) );

    LOG_INFO(g_log) << "Created Checkered texture \"" << aHandle
    << "\" with id=" << textureId
    << " into texture unit=" << textureUnit;
}

void
spark::TextureManager
::loadCheckerTexture( const TextureName& aHandle, GLint textureUnit )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    if( textureUnit == -1 )
    {
        textureUnit = reserveTextureUnit();
    }
    GLuint textureId = -1;
    GL_CHECK( glGenTextures( 1, &textureId ) );
    if( -1 == textureId )
    {
        LOG_ERROR(g_log) << "Failed to get a texture id!";
        assert(false);
        return;
    }

    m_registry[aHandle] = textureId;
    bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );
    
    GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
    //Set default texture state
    //TODO-- Can be overridden by TextureUnit?
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE ) );
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    spark::loadCheckerTexture();

    LOG_INFO(g_log) << "Created Checkered texture \"" << aHandle
                    << "\" with id=" << textureId
                    << " into texture unit=" << textureUnit;
}

void 
spark::TextureManager
::acquireExternallyAllocatedTexture( const TextureName& aName,
                                     GLuint aTextureId,
                                     GLenum aTextureType,
                                     GLint aTextureUnit )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    GLint textureUnit = aTextureUnit;
    if( textureUnit == -1 )
    {
        textureUnit = reserveTextureUnit();
    }
    m_registry[aName] = aTextureId;
    bindTextureIdToUnit( aTextureId, textureUnit, GL_TEXTURE_2D );
}

void 
spark::TextureManager
::loadTextureFromImageFile( const TextureName& aHandle, 
                            const char* aTextureFileName )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    GLint textureUnit = reserveTextureUnit();

    LOG_DEBUG(g_log) << "Searching for texture file \""
                     << aTextureFileName << "\".";
    std::string filePath;
    if( !m_finder->findFile( aTextureFileName, filePath ) )
    {
        LOG_ERROR(g_log) << "FAILED to find texture file \""
                         << aTextureFileName << "\" in search paths.";
        assert( false );
        return;
    }
    GLuint textureId = -1;
    GL_CHECK( glGenTextures( 1, &textureId ) );
    if( -1 == textureId )
    {
        LOG_ERROR(g_log) << "Failed to get a texture id!";
        assert(false);
        return;
    }
    m_registry[aHandle] = textureId;
    bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );

    bool success = loadTextureFromFile( filePath.c_str() );
    if( !success )
    {
        LOG_ERROR(g_log) << "Failed to load texture from file: \""
                         << filePath << "\".";
        assert( false );
        return;
    }
    GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
    //Set default texture state
    //TODO-- Can be overridden by TextureUnit?
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
                               GL_TEXTURE_WRAP_S,
                               GL_REPEAT ) );
	GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
                               GL_TEXTURE_WRAP_T,
                               GL_REPEAT ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
                               GL_TEXTURE_MIN_FILTER,
                               GL_LINEAR_MIPMAP_LINEAR ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D,
                               GL_TEXTURE_MAG_FILTER,
                               GL_LINEAR ) );
    m_paths[aHandle] = filePath;
    LOG_INFO(g_log) << "Texture \"" << aHandle << "\" loaded with id=" 
        << textureId << " into texture unit=" << textureUnit << " from path \""
        << filePath << "\"";
}

void 
spark::TextureManager
::acquireTextureUnitAndId( const TextureName& aHandle,
                           GLint& outTextureUnit,
                           GLuint& outTextureId )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    outTextureId = getTextureIdForHandle( aHandle );
    if( outTextureId == -1 )
    {
        outTextureUnit = -1;
        LOG_ERROR(g_log) << "Attempt to use unknown texture \"" 
            << aHandle << "\".";
        return;
    }
    outTextureUnit = ensureTextureUnitBoundToId( outTextureId );
}

bool 
spark::TextureManager
::executeSingleQueuedCommand( void )
{
    if( m_commandQueue.empty() )
    {
        return false;
    }
    std::set<TextureManagerCommandPtr, TextureManagerCommandComparator>::iterator oldestIter;
    {
        boost::lock_guard<boost::mutex> lock( m_commandQueueMutex );
        // get the command that has been waiting the longest, ie. smallest time/oldest
        oldestIter = m_commandQueue.begin();
        for( auto iter = m_commandQueue.begin();
             iter != m_commandQueue.end();
             ++iter )
        {
            if( (*iter)->m_creationTime < (*oldestIter)->m_creationTime )
            {
                oldestIter = iter;
            }
        }
        //std::cerr << "\t Loading texture: " << (*oldestIter)->m_handle << "\n";
        (*oldestIter)->operator()( this );
        m_commandQueue.erase( oldestIter );
    }
    return !m_commandQueue.empty();
}

void 
spark::TextureManager
::executeQueuedCommands( void )
{
    //std::cerr << "------------\n";
    std::set< TextureManagerCommandPtr, TextureManagerCommandComparator > execCopy;
    {
        boost::lock_guard<boost::mutex> lock( m_commandQueueMutex );
        std::copy( m_commandQueue.begin(), m_commandQueue.end(),
            std::inserter( execCopy, execCopy.begin() ) );
    }
    for( auto iter = execCopy.begin();
         iter != execCopy.end();
         ++iter )
    {
        assert( *iter );
        //std::cerr << "\t COMMAND: " << (*iter)->m_handle << "\n";
        (*iter)->operator()( this );
    }
    {
        boost::lock_guard<boost::mutex> lock( m_commandQueueMutex );
        m_commandQueue.clear();
    }
    //std::cerr << "------------\n";
}

void 
spark::TextureManager
::queueLoad3DTextureFromVolumeData( const TextureName& aHandle,
                                    VolumeDataPtr aVolume )
{
    boost::lock_guard<boost::mutex> lock( m_commandQueueMutex );

    m_commandQueue.insert( TextureManagerCommandPtr( 
        new Load3DTextureFromVolumeDataCommand( aHandle, aVolume ) ) );
}

void
spark::TextureManager
::load3DTextureFromVolumeData( const TextureName& aHandle,
                               VolumeDataPtr aVolume )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    // Ok to call many times, if so, reuse texture id
    GLuint textureId;
    if( exists( aHandle ) )
    {
        textureId = getTextureIdForHandle( aHandle );
    }
    else
    {
        GL_CHECK( glGenTextures( 1, &textureId ) );
        if( -1 == textureId )
        {
            LOG_ERROR(g_log) << "OpenGL failed to allocate a texture id.";
            assert(false);
            return;
        }
        m_registry[aHandle] = textureId;
        GLint textureUnit = reserveTextureUnit();
        bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_3D );
    }
    GLint textureUnit = ensureTextureUnitBoundToId( textureId );
    if( textureUnit == -1 )
    {
        LOG_ERROR(g_log) << "Failed to get a texture unit.";
        assert(false);
        return;
    }
    bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_3D );

    // Using 16-bit floats instead of 32-bit floats
    // decreases frame time by about 15ms with a 32x32x32 volume
    // on the K5000 video card.
    GLint textureDataFormat = GL_R16F;//GL_R32F;
    GL_CHECK( glTexImage3D( GL_TEXTURE_3D, 0, textureDataFormat,
                           aVolume->dimX(),
                           aVolume->dimY(),
                           aVolume->dimZ(),
                           0, GL_RED, GL_FLOAT,
                           aVolume->getDensityData() ) );
    
    GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );

    const GLenum edgeParameter = GL_CLAMP_TO_BORDER; //GL_CLAMP_TO_EDGE
    GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, edgeParameter ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, edgeParameter ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, edgeParameter ) );
    float borderColor[] = {0,0,0,0};
    glTexParameterfv( GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor );
    
    GL_CHECK( glGenerateMipmap( GL_TEXTURE_3D ) );

    LOG_TRACE(g_log) << "3DTexture for Volume Data \"" << aHandle
                    << "\" loaded with id=" << textureId
                    << " into texture unit=" << textureUnit ;
}

void
spark::TextureManager
::queueLoad2DByteTextureFromData( const TextureName& aHandle,
                                  const std::vector<unsigned char>& aData,
                                  size_t dimPerSide )
{
    boost::lock_guard<boost::mutex> lock( m_commandQueueMutex );
    m_commandQueue.insert( TextureManagerCommandPtr( 
        new Load2DByteTextureFromDataCommand( aHandle, aData, dimPerSide ) ) );
}

void 
spark::TextureManager
::queueSubsetLoad2DByteTextureFromData( const TextureName& aHandle, 
    const std::vector<unsigned char>& aData, 
    int dimPerSide,
    int minX, int minY, 
    int maxX, int maxY )
{
    boost::lock_guard<boost::mutex> lock( m_commandQueueMutex );
    m_commandQueue.insert( TextureManagerCommandPtr( 
        new SubsetLoad2DByteTextureFromDataCommand( aHandle, aData, dimPerSide, minX, minY, maxX, maxY ) ) );
}

void
spark::TextureManager
::doubleBufferedLoad2DByteTextureFromData( const TextureName& aHandle, const std::vector<unsigned char>& aData, size_t dimPerSide )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    // Allocate a new textureId for our created background texture
    GLuint backgroundTextureId;
    GL_CHECK( glGenTextures( 1, &backgroundTextureId ) );
    if( -1 == backgroundTextureId )
    {
        LOG_ERROR(g_log) << "OpenGL failed to allocate background texture id.";
        assert(false);
        return;
    }

    // bind the background texture to a texture unit
    GLint textureUnit = reserveTextureUnit();
    bindTextureIdToUnit( backgroundTextureId, textureUnit, GL_TEXTURE_2D );
    GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, GL_R8UI,
        dimPerSide,
        dimPerSide,
        0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
        &(aData[0]) ) );

    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) );

    const GLenum edgeParameter = GL_CLAMP_TO_BORDER; //GL_CLAMP_TO_EDGE
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, edgeParameter ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, edgeParameter ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, edgeParameter ) );
    unsigned int borderColor[] = {0,0,0,0};
    glTexParameterIuiv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

    GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );

    LOG_TRACE(g_log) << "2DTexture for Data \"" << aHandle
        << "\" loaded with id=" << backgroundTextureId
        << " into texture unit=" << textureUnit ;

    // At this point the new texture has been loaded into backgroundTextureId
    // so assign the handle to this new texture Id, 
    // and delete the old texture

    GLuint oldTextureId;
    if( exists( aHandle ) )
    {
        oldTextureId = getTextureIdForHandle( aHandle );
        // delete old texture
        GL_CHECK( glDeleteTextures( 1, &oldTextureId ) );
        // and remove it from the binding map
        for( auto p = m_bindingTextureUnitToTextureId.begin(); 
             p != m_bindingTextureUnitToTextureId.end(); 
             /* no incr */ )
        {
            if( p->first == oldTextureId )
            {
                p = m_bindingTextureUnitToTextureId.erase( p );
            }
            else
            {
                ++p;
            }
        }
    }
    acquireExternallyAllocatedTexture( aHandle, backgroundTextureId, GL_TEXTURE_2D, textureUnit );
}

void
spark::TextureManager
::load2DByteTextureFromData( const TextureName& aHandle,
                             const std::vector<unsigned char>& aData,
                             size_t dimPerSide )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );
    GLuint textureId;
    if( ! exists( aHandle ) )
    {
        // On first call, create new texture
        GL_CHECK( glGenTextures( 1, &textureId ) );
        if( -1 == textureId )
        {
            LOG_ERROR(g_log) << "OpenGL failed to allocate a texture id.";
            assert(false);
            return;
        }
        m_registry[aHandle] = textureId;
        GLint textureUnit = reserveTextureUnit();
        bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );
        textureUnit = ensureTextureUnitBoundToId( textureId );
        if( textureUnit == -1 )
        {
            LOG_ERROR(g_log) << "Failed to get a texture unit.";
            assert(false);
            return;
        }
        GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, GL_R8UI,
            dimPerSide,
            dimPerSide,
            0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
            &(aData[0]) ) );

        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) );

        const GLenum edgeParameter = GL_CLAMP_TO_BORDER; //GL_CLAMP_TO_EDGE
        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, edgeParameter ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, edgeParameter ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, edgeParameter ) );
        unsigned int borderColor[] = {0,0,0,0};
        glTexParameterIuiv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

    }
    else
    {
        // On all subsequent calls, use TexSubImage to write over the existing texture
        textureId = getTextureIdForHandle( aHandle );
        GLint textureUnit = ensureTextureUnitBoundToId( textureId );
        bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );
        if( textureUnit == -1 )
        {
            LOG_ERROR(g_log) << "Failed to get a texture unit.";
            assert(false);
            return;
        }
        bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );
        GL_CHECK( glTexSubImage2D( GL_TEXTURE_2D, 0, 
            0, 0, 
            dimPerSide, dimPerSide,
            GL_RED_INTEGER, // GL_ALPHA? 
            GL_UNSIGNED_BYTE,
            &(aData[0]) ) );
    }
    //GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
}

void
spark::TextureManager
::subsetLoad2DByteTextureFromData( const TextureName& aHandle,
    const std::vector<unsigned char>& aData,
    size_t dimPerSide,
    int minX, int minY, 
    int maxX, int maxY )
{
    assert( false ); // NYI 
    if( (maxX < minX) || (maxY < minY) )
    {
        return;
    }
    if( maxX == 0 && minX == 0 )
    {
        return;
    }
    assert( aData.size() == (1+maxX-minX)*(1+maxY-minY) );
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    // Ok to call many times, if so, reuse texture id
    GLuint textureId;
    if( exists( aHandle ) )
    {
        textureId = getTextureIdForHandle( aHandle );
    }
    else
    {
        GL_CHECK( glGenTextures( 1, &textureId ) );
        if( -1 == textureId )
        {
            LOG_ERROR(g_log) << "OpenGL failed to allocate a texture id.";
            assert(false);
            return;
        }
        m_registry[aHandle] = textureId;
        GLint textureUnit = reserveTextureUnit();
        bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );
    }
    GLint textureUnit = ensureTextureUnitBoundToId( textureId );
    if( textureUnit == -1 )
    {
        LOG_ERROR(g_log) << "Failed to get a texture unit.";
        assert(false);
        return;
    }

    bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );
    GL_CHECK( glTexSubImage2D(
        GL_TEXTURE_2D, 
        0,  // mipmap level 
        minX,
        minY,
        1 + maxX - minX, 
        1 + maxY - minY,
        GL_RED, //GL_RED_INTEGER, 
        GL_UNSIGNED_BYTE,
        &(aData[0]) ) );

    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) );

    const GLenum edgeParameter = GL_CLAMP_TO_BORDER; //GL_CLAMP_TO_EDGE
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, edgeParameter ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, edgeParameter ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, edgeParameter ) );
    unsigned int borderColor[] = {0,0,0,0};
    glTexParameterIuiv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

    //GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );

    LOG_TRACE(g_log) << "2DTexture for Volume Data \"" << aHandle
        << "\" loaded with id=" << textureId
        << " into texture unit=" << textureUnit ;
}


void
spark::TextureManager
::queueLoad2DFloatTextureFromData( const TextureName& aHandle,
    const std::vector<float>& aData,
    size_t dimPerSide )
{
    boost::lock_guard<boost::mutex> lock( m_commandQueueMutex );
    m_commandQueue.insert( TextureManagerCommandPtr( 
        new Load2DFloatTextureFromDataCommand( aHandle, aData, dimPerSide ) ) );
};

void
spark::TextureManager
::load2DFloatTextureFromData( const TextureName& aHandle,
                              const std::vector<float>& aData,
                              size_t dimPerSide )
{
    if( aData.empty() || dimPerSide == 0 || aHandle.empty() )
    {
        return;
    }
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );
    // Ok to call many times, if so, reuse texture id
    GLuint textureId;
    if( !exists( aHandle ) )
    {
        // First call, need to create the texture
        GL_CHECK( glGenTextures( 1, &textureId ) );
        if( -1 == textureId )
        {
            LOG_ERROR(g_log) << "OpenGL failed to allocate a texture id.";
            assert(false);
            return;
        }
        m_registry[aHandle] = textureId;
        GLint textureUnit = reserveTextureUnit();
        bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );

        GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F,
            dimPerSide,
            dimPerSide,
            0, GL_RED, GL_FLOAT,
            &(aData[0]) ) );

        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) );

        //GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ) );
        //GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );

        const GLenum edgeParameter = GL_CLAMP_TO_BORDER; //GL_CLAMP_TO_EDGE
        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, edgeParameter ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, edgeParameter ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, edgeParameter ) );
        float borderColor[] = {0,0,0,0};
        glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

        //GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );

        LOG_TRACE(g_log) << "2DTexture for Volume Data \"" << aHandle
            << "\" loaded with id=" << textureId
            << " into texture unit=" << textureUnit ;
    }
    else
    {
        // Texture already created, just reload data
        textureId = getTextureIdForHandle( aHandle );
        GLint textureUnit = ensureTextureUnitBoundToId( textureId );
        bindTextureIdToUnit( textureId, textureUnit, GL_TEXTURE_2D );
        if( textureUnit == -1 )
        {
            LOG_ERROR(g_log) << "Failed to get a texture unit.";
            assert(false);
            return;
        }
        checkOpenGLErrors();
        GL_CHECK( glTexSubImage2D( 
            GL_TEXTURE_2D, 0, 
            0, 0, // xoffset, yoffset
            dimPerSide, // width
            dimPerSide, // height
            GL_RED, GL_FLOAT,
            &(aData[0]) ) );

        //GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F,
        //    dimPerSide,
        //    dimPerSide,
        //    0, GL_RED, GL_FLOAT,
        //    &(aData[0]) ) );

        //GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );

    }
}

bool
spark::TextureManager
::isTextureReady( const TextureName& aHandle )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );
    auto iter = m_registry.find( aHandle );
    if( iter == m_registry.end() ) 
    {
        return false;
    }
    GLint id = getTextureUnitBoundToId( iter->second );
    return id != -1 ;
}

GLint
spark::TextureManager
::getTextureUnitForHandle( const TextureName& aHandle )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );
    GLint texId = getTextureIdForHandle( aHandle );
    if( texId == -1 ) 
    {
        LOG_ERROR(g_log) << "Unable to provide texture unit for unknown texture \""
            << aHandle << "\".";
        return -1;
    }
    return ensureTextureUnitBoundToId( texId );
}

void
spark::TextureManager
::activateTextureUnitForHandle( const TextureName& aHandle )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    GLint unit = getTextureUnitForHandle( aHandle );
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + unit ) );
}

void
spark::TextureManager
::bindTextureIdToUnit( GLint aTextureId, GLint aTextureUnit, GLenum aTextureType )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    GL_CHECK( glActiveTexture( GL_TEXTURE0 + aTextureUnit ) );
    m_textureType[ aTextureId ] = aTextureType;
    GL_CHECK( glBindTexture( aTextureType, aTextureId ) );
    m_bindingTextureUnitToTextureId.push_back( std::make_pair( aTextureUnit,
                                                               aTextureId) );
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "Bound texture \""
                         << getTextureHandleFromTextureId( aTextureId )
                         << "\" ID="
                         << aTextureId << " to texture unit "
                         << aTextureUnit << ", type = "
                         << ((aTextureType == GL_TEXTURE_2D) ? "TEXTURE_2D" : "" )
                         << ((aTextureType == GL_TEXTURE_3D) ? "TEXTURE_3D" : "" );
    }
}

GLint
spark::TextureManager
::reserveTextureUnit( void )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    m_nextAvailableTextureUnit = (m_nextAvailableTextureUnit+1) % m_maxTextureUnits;
    auto iter = m_bindingTextureUnitToTextureId.begin();
    while( iter != m_bindingTextureUnitToTextureId.end() )
    {
        bool wasDeleted = false;
        if( iter->first == m_nextAvailableTextureUnit )
        {
            // Unbind current texture
            // Note this could be skipped for performance, but
            // is good for debugging
            GL_CHECK( glActiveTexture( GL_TEXTURE0 + m_nextAvailableTextureUnit ) );
            auto typeIter = m_textureType.find( iter->second );
            if( typeIter != m_textureType.end() )
            {
                GL_CHECK( glBindTexture( typeIter->second, 0 ) );
            }
            iter = m_bindingTextureUnitToTextureId.erase( iter );
            wasDeleted = true;
        }
        if( !wasDeleted )
        {
            ++iter;
        }
    }
    return m_nextAvailableTextureUnit;
}

GLint 
spark::TextureManager
::getTextureUnitBoundToId( GLuint aTextureId ) const
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    auto iter = m_bindingTextureUnitToTextureId.begin();
    for( ; iter != m_bindingTextureUnitToTextureId.end(); ++iter )
    {
        if( iter->second == aTextureId )
        {
            return iter->first;
        }
    }
    LOG_DEBUG(g_log) << "In getTextureIdBoundToUnit, looking up an unbound texture ID.";
    return -1;
}

GLint 
spark::TextureManager
::ensureTextureUnitBoundToId( GLuint aTextureId ) 
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    if( aTextureId == -1 )
    {
        LOG_ERROR(g_log) << "Attempt to bind to unloaded texture (-1).";
        return -1;
    }
    GLint unit = getTextureUnitBoundToId( aTextureId );
    if( unit == -1 )
    {
        // Not currently bound, need to reserve next texture unit
        // reserve returns the least-recently used unit
        unit = reserveTextureUnit();
        bindTextureIdToUnit( aTextureId, unit );
    }
    return unit;
}

GLuint 
spark::TextureManager
::getTextureIdBoundToUnit( GLint aTextureUnit ) const
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    auto iter = m_bindingTextureUnitToTextureId.begin();
    for( ; iter != m_bindingTextureUnitToTextureId.end(); ++iter )
    {
        if( iter->first == aTextureUnit )
        {
            return iter->second;
        }
    }
    LOG_WARN(g_log) << "In getTextureIdBoundToUnit, looking up an unbound texture unit.";
    return (unsigned int)-1;
}

void
spark::TextureManager
::setTextureParameteri( const TextureName& aHandle, 
                        GLenum target, 
                        GLenum pname, 
                        GLint param )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    auto iter = m_registry.find( aHandle );
    if( iter != m_registry.end() )
    {
        GLuint texture = iter->second;
        if( glewIsSupported( "GL_EXT_direct_state_access" ) )
        {
            GL_CHECK( glTextureParameteriEXT( texture, target, pname, param ) );
        }
        else
        {
            GLint curTex;
            glGetIntegerv( GL_TEXTURE_BINDING_2D, &curTex );
            glBindTexture( GL_TEXTURE_2D, texture );
            glTexParameteri( GL_TEXTURE_2D, pname, param );
            glBindTexture( GL_TEXTURE_2D, curTex );
        }
    }
    else
    {
        LOG_ERROR( g_log ) << "Cannot find texture by name \"" << aHandle << "\".";
    }
}

GLuint 
spark::TextureManager
::getTextureIdForHandle( const TextureName& aHandle ) const
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    auto iter = m_registry.find( aHandle );
    if( iter == m_registry.end() )
    {
        LOG_ERROR(g_log) << "Failed to find Texture by handle \"" 
                         << aHandle << "\". (OK during initialization)";
        return -1;
    }
    return iter->second;
}

bool
spark::TextureManager
::exists( const TextureName& aHandle ) const
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    return m_registry.end() != m_registry.find( aHandle );
}

spark::TextureName
spark::TextureManager
::getTextureHandleFromTextureId( GLuint aTextureId ) const
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    for( auto iter = m_registry.begin(); iter != m_registry.end(); ++iter )
    {
        if( iter->second == aTextureId )
        {
            return iter->first;
        }
    }
    return "__ERROR__TEXTURE_NOT_REGISTERED";
}

void
spark::TextureManager
::releaseAll( void )
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    auto iter = m_registry.begin();
    for( ; iter != m_registry.end(); ++iter )
    {
        GL_CHECK( glDeleteTextures( 1, &(iter->second) ) );
    }
    m_registry.clear();
    m_bindingTextureUnitToTextureId.clear();
}

void
spark::TextureManager
::logTextures( void ) const
{
    boost::unique_lock<boost::recursive_mutex> lock( m_registryMutex );

    LOG_INFO(g_log) << "TextureManager resources:";
    for( auto iter = m_registry.begin(); iter != m_registry.end(); ++iter )
    {
        TextureName name = iter->first;
        GLuint id = iter->second;
        
        std::string path( "NOT_FILE_BASED" );
        auto pathIter = m_paths.find( name );
        if( pathIter != m_paths.end() )
        {
            path = pathIter->second;
        }
        GLuint unit = getTextureUnitBoundToId( id );
        LOG_INFO(g_log) << "\tName=\"" << name << "\"\tID=" << id
                        << "\tUnit=" << unit << "\tPath=\"" << path << "\"";
    }
}
