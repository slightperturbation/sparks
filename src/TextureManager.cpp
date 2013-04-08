
#include "TextureManager.hpp"

#include <iostream>

TextureManager& 
TextureManager
::get()
{
    static TextureManager singleton;
    return singleton;
}

TextureManager
::~TextureManager()
{
    releaseAllTextures();
}

void 
TextureManager
::loadTextureFromImageFile( const char* aTextureFilePath, 
                            const std::string& aHandle )
{
    // Load all textures into the zero-th texture unit
    GLuint textureId = 0;
    glGenTextures( 1, &textureId ); checkOpenGLErrors();

    GLuint textureUnit = 0; // by default, use the base/zeroth texture unit for the initial load
    glActiveTexture( GL_TEXTURE0 + textureUnit ); checkOpenGLErrors();
    glBindTexture( GL_TEXTURE_2D, textureId ); checkOpenGLErrors();

    std::cerr << "Attempting to load texture from file: \"" << aTextureFilePath << "\"... ";

    bool success = loadTextureFromFile( aTextureFilePath );
    checkOpenGLErrors();
    if( !success )
    {
        std::cerr << "\n------- FAILED to load texture from file: \"" << aTextureFilePath << "\".\n";
        assert( false );
    }
    else
    {
        std::cerr << "OK\n";
        m_registry[aHandle] = textureId;
        m_currentBinding[textureUnit] = textureId;
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    checkOpenGLErrors();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    checkOpenGLErrors();
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
    glActiveTexture( GL_TEXTURE0 + aTextureUnit );
    checkOpenGLErrors();
    glBindTexture( GL_TEXTURE_2D, textureId );
    checkOpenGLErrors();
    m_currentBinding[aTextureUnit] = textureId;
    std::cerr << "Binding texture \"" << aHandle << "\" to texture unit " << aTextureUnit << ".\n";
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
::releaseAllTextures( void )
{
    std::map< std::string, GLuint >::iterator iter = m_registry.begin();
    for( ; iter != m_registry.end(); ++iter )
    {
        glDeleteTextures( 1, &(iter->second) );
        checkOpenGLErrors();
    }
    m_registry.clear();
    m_currentBinding.clear();
}
