
#ifndef SPARKS_TEXTUREMANAGER_HPP
#define SPARKS_TEXTUREMANAGER_HPP

#include <string>
#include <map>

#include "Utilities.hpp"

/// Loads and manages OpenGL textures.
/// Avoids redundant glBindTexture calls.
class TextureManager
{
public:
    /// Get the singleton TextureManager
    /// Note: created on first call, but destructor order is undefined,
    /// so primary thread must call "manually" call releaseAllTextures()
    static TextureManager& get();
    ~TextureManager();
    /// Load the image file at aTextureFilePath and associate it with 
    /// aHandle.  Later, can call e.g. bindTextureToUnit( aHandle, 0 );
    void loadTextureFromImageFile( const char* aTextureFilePath, 
                                   const std::string& aHandle );
    /// Call at render time to bind the texture with aHandle to the given texture unit.
    /// Generally aTextureUnit will be matched to a sampler in a texture,
    /// E.g.: m_mesh->setShaderUniformInt( "tex2d", aTextureUnit );
    void bindTextureToUnit( const std::string& aHandle, 
                            GLuint aTextureUnit );
    /// Returns (unsigned int)-1 if texture unit has not been bound to a texture yet.
    GLuint getTextureIdBoundToUnit( GLuint aTextureUnit ) const;
    /// Clients must call releaseAllTextures() explicitly (rather than to rely on 
    /// destructor) to ensure order of release.
    void releaseAllTextures( void );
private:
    /// Make private to ensure only singleton access.
    TextureManager() {}
    /// Registry maps string handles to texture "ID"s (sometimes called 
    /// texture names in GL docs).
    std::map< std::string, GLuint > m_registry;
    /// Stores the textureId (second) known to be bound to a given 
    /// texture unit (first).
    std::map< GLuint, GLuint > m_currentBinding;
};

#endif

