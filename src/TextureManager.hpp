
#ifndef SPARKS_TEXTUREMANAGER_HPP
#define SPARKS_TEXTUREMANAGER_HPP

#include "SoftTestDeclarations.hpp"
#include "Utilities.hpp"
#include "FileAssetFinder.hpp"

#include <string>
#include <map>

/// Loads and manages OpenGL textures.
/// Avoids redundant glBindTexture calls.
/// Texture units are bound to textures on a first-in-first-out basis.
/// Typical usage:
/// In init
///   tm->loadTextureFromImageFile( "fire.png", "fire_color" );
/// During rendering (in Material::use()):
///   GLint texUnit = tm->getTextureUnitForTexture( "fire_color" );
///   m_shader->setUniform( "s_color", texUnit );
/// Where "s_color" is the shader's texture sampler.
class TextureManager : public std::enable_shared_from_this< TextureManager >
{
public:
    TextureManager( void );
    TextureManager( FileAssetFinderPtr finder ) : m_finder( finder ) { }
    ~TextureManager();
    
    /// Set the asset finder, giving a set of paths for texture files.
    void setAssetFinder( FileAssetFinderPtr finder ) { m_finder = finder; }
    FileAssetFinderPtr assetFinder( void ) { return m_finder; }
    
    void loadTestTexture( const TextureName& aHandle, GLint textureUnit = -1  );
    void loadCheckerTexture( const TextureName& aHandle, GLint textureUnit = -1 );

    /// Load the image file at aTextureFilePath and associate it with 
    /// aHandle.  Later, can call e.g. bindTextureToUnit( aHandle, 0 );
    void loadTextureFromImageFile( const char* aTextureFilePath, 
                                   const TextureName& aHandle );
    /// Returns true if the given texture handle is loaded and bound to a
    /// texture unit.
    bool isTextureNameReady( const TextureName& aHandle );

    /// Returns the Texture Unit that the given texture is currently bound to.
    /// If texture is not currently bound to a unit, then the next unit will
    /// be assigned on a least-recently used basis.
    GLint getTextureUnitForTexture( const TextureName& aHandle );
    
    /// Parameters set per-texture handle
    void setTextureParameteri( const TextureName& aHandle, GLenum target,
                               GLenum  pname, GLint param );
    /// Returns OpenGL's texture ID (aka texture name) for the given handle.
    /// Returns -1 if handle is not found.
    GLuint getTextureIdFromHandle( const TextureName& aHandle ) const;
    /// Clients must call releaseAll() explicitly (rather than to rely on
    /// destructor) to ensure order of release.
    void releaseAll( void );

    /// Override cacheing and force a rebinding of the texture to
    /// its current unit.
    /// DEBUG ONLY
    void forceTextureBind( const TextureName& aHandle )
    {
        GLint texId = getTextureIdFromHandle( aHandle );
        GLint unit = getTextureUnitBoundToId( texId );
        assert( unit != -1 );
        bindTextureIdToUnit( texId, unit );
    }
private:
    void bindTextureIdToUnit( GLint aTextureId, GLint aTextureUnit );
    /// Request to get an uncontrolled (by manager) texture unit
    /// deprecated
    GLint reserveTextureUnit( void )
    {
        m_nextAvailableTextureUnit = (m_nextAvailableTextureUnit+1) % m_maxTextureUnits;
        return m_nextAvailableTextureUnit;
    }
    /// Returns (unsigned int)-1 if texture unit has not been bound to a texture id yet.
    GLuint getTextureIdBoundToUnit( GLint aTextureUnit ) const;
    /// Returns (unsigned int)-1 if texture id has not been bound to a texture unit yet.
    GLint getTextureUnitBoundToId( GLuint aTextureId ) const;
    /// Call at render time to bind the texture with aHandle to the given texture unit.
    //TODO-- When is this useful?  Remove?
    void bindTextureToUnit( const TextureName& aHandle,
                            GLint aTextureUnit );
private:
    FileAssetFinderPtr m_finder;
    /// Registry maps string handles/names to texture "ID"s (sometimes called 
    /// texture names in GL docs).
    std::map< const TextureName, GLuint > m_registry;
    /// Store paths used for loading in order to reload or watch
    std::map< const TextureName, std::string > m_paths;
    /// Stores the textureId (second) known to be bound to a given 
    /// texture unit (first).
    std::vector< std::pair<GLint, GLuint> > m_bindingTextureUnitToTextureId;

    GLint m_maxTextureUnits;
    GLint m_nextAvailableTextureUnit;
};
typedef std::shared_ptr< TextureManager > TextureManagerPtr;
#endif

