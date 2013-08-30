
#include "TextureManager.hpp"
#include "VolumeData.hpp"

#include <iostream>


spark::TextureManager
::TextureManager( void )
: m_finder( new FileAssetFinder() )
{
    GL_CHECK( glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTextureUnits ) );
    // set next to a middle value to catch errors using default unit 0
    m_nextAvailableTextureUnit = 0;//m_maxTextureUnits / 2;
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
    auto iter = m_registry.find( aHandle );
    if( iter != m_registry.end() )
    {
        LOG_WARN(g_log) << "Attempt to createTargetTexture(\""
            << aHandle 
            << "\") but texture with that name already exists.  "
            << "Ignoring attempt and preserving existing texture.";
        return;
    }
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
    
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); 

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
    
    //    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR ); 
    //    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
 
    
    ////////////
    /// TODO $$$$$ MUST BIND the FBO before this next call????
    
    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0 );
}

void
spark::TextureManager
::createDepthTargetTexture( const TextureName& aHandle,
                              int width, int height )
{
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
    
    // GL_LINEAR for simple shadow maps (make an argument?)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    
    // Remove artifact on the edges of the shadowmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL );
    
    ////////////
    /// TODO $$$$$ MUST BIND the FBO before this next call????
    

    // Attach this texture to a FBO target (depth)
    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER,
                            GL_DEPTH_ATTACHMENT,
                            GL_TEXTURE_2D,
                            textureId,
                            0 );
}

void
spark::TextureManager
::loadTestTexture( const TextureName& aHandle, GLint textureUnit )
{
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
    GLint textureUnit = aTextureUnit;
    if( textureUnit == -1 )
    {
        textureUnit = reserveTextureUnit();
    }
    m_registry[aName] = aTextureId;
    bindTextureIdToUnit( aTextureId, textureUnit, GL_TEXTURE_2D );
}

void spark::TextureManager::loadTextureFromImageFile( const TextureName& aHandle, const char* aTextureFileName )
{
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

void
spark::TextureManager
::load3DTextureFromVolumeData( const TextureName& aHandle,
                               VolumeDataPtr aVolume )
{
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
    GL_CHECK( glTexImage3D( GL_TEXTURE_3D, 0, GL_R32F,
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
::load2DByteTextureFromData( const TextureName& aHandle,
                             const std::vector<unsigned char>& aData,
                             size_t dimPerSide )
{
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
    
    LOG_TRACE(g_log) << "2DTexture for Volume Data \"" << aHandle
    << "\" loaded with id=" << textureId
    << " into texture unit=" << textureUnit ;
}


void
spark::TextureManager
::load2DFloatTextureFromData( const TextureName& aHandle,
                              const std::vector<float>& aData,
                              size_t dimPerSide )
{
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
    GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F,
                            dimPerSide,
                            dimPerSide,
                            0, GL_RED, GL_FLOAT,
                            &(aData[0]) ) );

    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );

    const GLenum edgeParameter = GL_CLAMP_TO_BORDER; //GL_CLAMP_TO_EDGE
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, edgeParameter ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, edgeParameter ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, edgeParameter ) );
    float borderColor[] = {0,0,0,0};
    glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

    GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );

    LOG_TRACE(g_log) << "2DTexture for Volume Data \"" << aHandle
        << "\" loaded with id=" << textureId
        << " into texture unit=" << textureUnit ;
}

bool
spark::TextureManager
::isTextureReady( const TextureName& aHandle )
{
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
    GLint texId = getTextureIdForHandle( aHandle );
    if( texId == -1 ) 
    {
        LOG_ERROR(g_log) << "Unable to provide texture unit for unknown texture \""
            << aHandle << "\".";
        throw "getTextureUnitForHandle called for unknown texture";
        return -1;
    }
    return ensureTextureUnitBoundToId( texId );
}

void
spark::TextureManager
::activateTextureUnitForHandle( const TextureName& aHandle )
{
    GLint unit = getTextureUnitForHandle( aHandle );
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + unit ) );
}

void
spark::TextureManager
::bindTextureIdToUnit( GLint aTextureId, GLint aTextureUnit, GLenum aTextureType )
{
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + aTextureUnit ) );
    m_textureType[ aTextureId ] = aTextureType;
    GL_CHECK( glBindTexture( aTextureType, aTextureId ) );
    m_bindingTextureUnitToTextureId.push_back( std::make_pair( aTextureUnit,
                                                               aTextureId) );
    LOG_TRACE(g_log) << "Bound texture \""
                     << getTextureHandleFromTextureId( aTextureId )
                     << "\" ID="
                     << aTextureId << " to texture unit "
                     << aTextureUnit << ", type = "
                     << ((aTextureType == GL_TEXTURE_2D) ? "TEXTURE_2D" : "" )
                     << ((aTextureType == GL_TEXTURE_3D) ? "TEXTURE_3D" : "" );
}

GLint
spark::TextureManager
::reserveTextureUnit( void )
{
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
    auto iter = m_registry.find( aHandle );
    if( iter == m_registry.end() )
    {
        LOG_ERROR(g_log) << "Failed to find Texture by handle \"" 
                         << aHandle << "\".";
        return -1;
    }
    return iter->second;
}

bool
spark::TextureManager
::exists( const TextureName& aHandle ) const
{
    return m_registry.end() != m_registry.find( aHandle );
}

spark::TextureName
spark::TextureManager
::getTextureHandleFromTextureId( GLuint aTextureId ) const
{
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
