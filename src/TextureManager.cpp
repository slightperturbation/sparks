
#include "TextureManager.hpp"

#include <iostream>


TextureManager
::~TextureManager()
{
    releaseAll();
}

void 
TextureManager
::loadTextureFromImageFile( const char* aTextureFilePath, 
                            const std::string& aHandle )
{
    // Load all textures into the zero-th texture unit
    GLuint textureId = 0;
    GL_CHECK( glGenTextures( 1, &textureId ) );

    GLuint textureUnit = 0; // by default, use the base/zeroth texture unit for the initial load
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + textureUnit ) );
    GL_CHECK( glBindTexture( GL_TEXTURE_2D, textureId ) );

    LOG_INFO(g_log) << "Attempting to load texture from file: \"" << aTextureFilePath << "\"... ";

    bool success = loadTextureFromFile( aTextureFilePath );
    if( !success )
    {
        LOG_INFO(g_log) << "\n------- FAILED to load texture from file: \"" << aTextureFilePath << "\".\n";
        assert( false );
    }
    else
    {
        LOG_INFO(g_log) << "Loaded OK.\n";
        m_registry[aHandle] = textureId;
        m_paths[aHandle] = aTextureFilePath;
        m_currentBinding[textureUnit] = textureId;
    }
    GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ) );
}

void
TextureManager
::bindTextureToUnit( const std::string& aHandle, GLuint aTextureUnit )
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
    m_currentBinding[aTextureUnit] = textureId;
    LOG_INFO(g_log) << "Binding texture \"" << aHandle << "\" to texture unit " << aTextureUnit << ".\n";
}

GLuint 
TextureManager
::getTextureIdBoundToUnit( GLuint aTextureUnit ) const
{
    std::map< GLuint, GLuint >::const_iterator iter = m_currentBinding.find( aTextureUnit );
    if( iter == m_currentBinding.end() )
    {
        return -1;
    }
    return iter->second;
}

void
TextureManager
::setTextureParameteri( const std::string& aHandle, 
                        GLenum target, 
                        GLenum pname, 
                        GLint param )
{
    std::map< std::string, GLuint >::iterator iter = m_registry.find( aHandle );
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

void 
TextureManager
::releaseAll( void )
{
    std::map< std::string, GLuint >::iterator iter = m_registry.begin();
    for( ; iter != m_registry.end(); ++iter )
    {
        GL_CHECK( glDeleteTextures( 1, &(iter->second) ) );
    }
    m_registry.clear();
    m_currentBinding.clear();
}
