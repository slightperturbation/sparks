
#ifndef SPARKS_TEXTUREMANAGER_HPP
#define SPARKS_TEXTUREMANAGER_HPP

#include "SoftTestDeclarations.hpp"
#include "Utilities.hpp"
#include "FileAssetFinder.hpp"

#include <string>
#include <map>

/// Loads and manages OpenGL textures.
/// Avoids redundant glBindTexture calls.
class TextureManager
{
public:
    ~TextureManager();

    /// Set the asset finder, giving a set of paths for texture files.
    void setAssetFinder( FileAssetFinderPtr finder ) { m_finder = finder; }

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
    void setTextureParameteri( const std::string& aHandle, GLenum target, 
                               GLenum  pname, GLint param );
    /// Clients must call releaseAll() explicitly (rather than to rely on 
    /// destructor) to ensure order of release.
    void releaseAll( void );
private:
    /// Registry maps string handles to texture "ID"s (sometimes called 
    /// texture names in GL docs).
    std::map< std::string, GLuint > m_registry;
    /// Store paths used for loading in order to reload or watch
    std::map< std::string, std::string > m_paths;
    /// Stores the textureId (second) known to be bound to a given 
    /// texture unit (first).
    std::map< GLuint, GLuint > m_currentBinding;

    FileAssetFinderPtr m_finder;
};

#endif

