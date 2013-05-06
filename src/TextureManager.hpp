
#ifndef SPARKS_TEXTUREMANAGER_HPP
#define SPARKS_TEXTUREMANAGER_HPP

#include "SoftTestDeclarations.hpp"
#include "Utilities.hpp"
#include "FileAssetFinder.hpp"

#include <string>
#include <map>


/// Loads and manages OpenGL textures.
/// Avoids redundant glBindTexture calls.
/// Note texture units are signed and texture ids are unsigned.
class TextureManager
{
public:
    TextureManager( void )
    : m_finder( new FileAssetFinder() ),
      m_nextAvailableTextureUnit( 0 )
    { }
    TextureManager( FileAssetFinderPtr finder ) : m_finder( finder ) { }
    ~TextureManager();
    
    void setNextAvailableTextureUnit( GLint unit )
    { m_nextAvailableTextureUnit = unit; }

    /// Set the asset finder, giving a set of paths for texture files.
    void setAssetFinder( FileAssetFinderPtr finder ) { m_finder = finder; }

    /// Load the image file at aTextureFilePath and associate it with 
    /// aHandle.  Later, can call e.g. bindTextureToUnit( aHandle, 0 );
    void loadTextureFromImageFile( const char* aTextureFilePath, 
                                   const TextureName& aHandle );
    bool isTextureNameReady( const TextureName& aHandle )
    {
        auto iter = m_registry.find( aHandle );
        if( iter == m_registry.end() ) return false;
        return ( getTextureUnitBoundToId( iter->second ) != (unsigned int)-1 );
    }
    /// Request to get an uncontrolled (by manager) texture unit
    GLint reserveTextureUnit( void )
    { 
        return m_nextAvailableTextureUnit++; 
    }
    /// Returns the Texture Unit that the given texture is currently bound to.
    /// Returns (unsigned int)-1 and logs error if texture is not bound to a unit.
    GLint getTextureUnitForTexture( const TextureName& aHandle )
    {
        return getTextureUnitBoundToId( getTextureIdFromHandle( aHandle ) );
    }
    /// Parameters set per-texture handle
    void setTextureParameteri( const TextureName& aHandle, GLenum target,
                               GLenum  pname, GLint param );
    /// Returns OpenGL's texture ID (aka texture name) for the given handle.
    /// Returns -1 if handle is not found.
    GLuint getTextureIdFromHandle( const TextureName& aHandle ) const;
    /// Clients must call releaseAll() explicitly (rather than to rely on
    /// destructor) to ensure order of release.
    void releaseAll( void );
private:    
    /// Returns (unsigned int)-1 if texture unit has not been bound to a texture id yet.
    GLuint getTextureIdBoundToUnit( GLint aTextureUnit ) const;
    /// Returns (unsigned int)-1 if texture id has not been bound to a texture unit yet.
    GLint getTextureUnitBoundToId( GLuint aTextureId ) const;
    /// Call at render time to bind the texture with aHandle to the given texture unit.
    //TODO-- When is this useful?  Remove?
    void bindTextureToUnit( const TextureName& aHandle,
                            GLint aTextureUnit );
private:
    /// Registry maps string handles/names to texture "ID"s (sometimes called 
    /// texture names in GL docs).
    std::map< const TextureName, GLuint > m_registry;
    /// Store paths used for loading in order to reload or watch
    std::map< const TextureName, std::string > m_paths;
    /// Stores the textureId (second) known to be bound to a given 
    /// texture unit (first).
    std::vector< std::pair<GLint, GLuint> > m_bindingTextureUnitToTextureId;

    FileAssetFinderPtr m_finder;
    GLint m_nextAvailableTextureUnit;
};
typedef std::shared_ptr< TextureManager > TextureManagerPtr;
#endif

