
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
    TextureManager( void )
    : m_finder( new FileAssetFinder() ),
      m_nextAvailableTextureUnit( 0 )
    { }
    TextureManager( FileAssetFinderPtr finder ) : m_finder( finder ) { }
    ~TextureManager();
    
    void setNextAvailableTextureUnit( GLuint unit )
    { m_nextAvailableTextureUnit = unit; }

    /// Set the asset finder, giving a set of paths for texture files.
    void setAssetFinder( FileAssetFinderPtr finder ) { m_finder = finder; }

    /// Load the image file at aTextureFilePath and associate it with 
    /// aHandle.  Later, can call e.g. bindTextureToUnit( aHandle, 0 );
    void loadTextureFromImageFile( const char* aTextureFilePath, 
                                   const TextureName& aHandle );
    /// Request to get an uncontrolled (by manager) texture unit
    GLuint reserveTextureUnit( void )
    { return m_nextAvailableTextureUnit++; }
    
    GLuint getTextureUnitForTexture( const TextureName& aHandle )
    {
        return m_currentBinding[ m_registry[aHandle] ];
    }

    /// Returns (unsigned int)-1 if texture unit has not been bound to a texture yet.
    GLuint getTextureIdBoundToUnit( GLuint aTextureUnit ) const;
    /// Parameters set per-texture handle
    void setTextureParameteri( const TextureName& aHandle, GLenum target,
                              GLenum  pname, GLint param );
    /// Returns OpenGL's texture ID (aka texture name) for the given handle.
    /// Returns -1 if handle is not found.
    GLint getTextureIdFromHandle( const TextureName& aHandle ) const;
    /// Clients must call releaseAll() explicitly (rather than to rely on
    /// destructor) to ensure order of release.
    void releaseAll( void );
private:    
    /// Call at render time to bind the texture with aHandle to the given texture unit.
    //TODO-- When is this useful?  Remove?
    void bindTextureToUnit( const TextureName& aHandle,
                            GLuint aTextureUnit );
private:
    /// Registry maps string handles/names to texture "ID"s (sometimes called 
    /// texture names in GL docs).
    std::map< const TextureName, GLuint > m_registry;
    /// Store paths used for loading in order to reload or watch
    std::map< const TextureName, std::string > m_paths;
    /// Stores the textureId (second) known to be bound to a given 
    /// texture unit (first).
    std::map< GLuint, GLuint > m_currentBinding;

    FileAssetFinderPtr m_finder;
    GLuint m_nextAvailableTextureUnit;
};
typedef std::shared_ptr< TextureManager > TextureManagerPtr;
#endif

