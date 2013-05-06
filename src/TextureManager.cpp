
#include "TextureManager.hpp"

#include <iostream>


TextureManager
::~TextureManager()
{
    releaseAll();
}

void 
TextureManager
::loadTextureFromImageFile( const char* aTextureFileName, 
                            const TextureName& aHandle )
{
    GLint maxTextureUnits;
    glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits );
    if( m_nextAvailableTextureUnit >= maxTextureUnits )
    {
        LOG_ERROR(g_log) << "Out of texture units!  Cannot load texture \""
                         << aHandle << "\" from file \""
                         << aTextureFileName << "\".";
        //assert( false );
        return;
    }
    GLint textureUnit = m_nextAvailableTextureUnit++;

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
    GLuint textureId = 0;
    GL_CHECK( glGenTextures( 1, &textureId ) );
    if( 0 == textureId )
    {
        LOG_ERROR(g_log) << "Failed to get a texture id!";
        assert(false);
        return;
    }
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + textureUnit ) );
    GL_CHECK( glBindTexture( GL_TEXTURE_2D, textureId ) );
    bool success = loadTextureFromFile( filePath.c_str() );
    if( !success )
    {
        LOG_ERROR(g_log) << "FAILED to load texture from file: \""
                         << filePath << "\".";
        assert( false );
        return;
    }

    GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
    //Set default texture state
    //-- Can be overridden by TextureUnit?
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
    m_registry[aHandle] = textureId;
    m_paths[aHandle] = filePath;
    m_bindingTextureUnitToTextureId.push_back( std::make_pair( textureUnit,
                                                               textureId) );
    LOG_INFO(g_log) << "Texture " << aHandle << " loaded with id=" 
        << textureId << " into texture unit=" << textureUnit << " from path \""
        << filePath << "\"";
}

void
TextureManager
::bindTextureToUnit( const TextureName& aHandle, GLint aTextureUnit )
{
    // Can fail if aHandle hasn't been assigned to a texture yet.
    GLuint textureId = m_registry[aHandle];
    if( getTextureIdBoundToUnit(aTextureUnit) == textureId )
    {
        // Redundant binding request, skip.
        return;
    }
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + aTextureUnit ) );
    GL_CHECK( glBindTexture( GL_TEXTURE_2D, textureId ) );
    m_bindingTextureUnitToTextureId.push_back( std::make_pair(aTextureUnit, textureId) );
    LOG_TRACE(g_log) << "Bound texture \"" << aHandle << "\" (ID="
        << textureId << ") to texture unit " << aTextureUnit << ".\n";
}

GLint 
TextureManager
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
    LOG_WARN(g_log) << "In getTextureIdBoundToUnit, looking up an unbound texture ID.";
    return -1;
}

GLuint 
TextureManager
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
TextureManager
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
TextureManager
::getTextureIdFromHandle( const TextureName& aHandle ) const
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

void 
TextureManager
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
