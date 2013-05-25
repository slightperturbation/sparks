
#ifndef SPARKS_TEXTUREMANAGER_HPP
#define SPARKS_TEXTUREMANAGER_HPP

#include "SoftTestDeclarations.hpp"
#include "Utilities.hpp"
#include "FileAssetFinder.hpp"

#include <string>
#include <map>

namespace spark
{

    /// Loads and manages OpenGL textures.
    /// Avoids redundant glBindTexture calls.
    /// Texture units are bound to textures on a first-in-first-out basis.
    /// Typical usage:
    /// In init
    ///   tm->loadTextureFromImageFile( "fire.png", "fire_color" );
    /// During rendering (in Material::use()):
    ///   GLint texUnit = tm->getTextureUnitForHandle( "fire_color" );
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

        /// Load a simple test texture.
        void loadTestTexture( const TextureName& aHandle, GLint textureUnit = -1  );

        /// Load a simple 3x3 checkerboard test texture.
        void loadCheckerTexture( const TextureName& aHandle, GLint textureUnit = -1 );

        /// Load the image file at aTextureFilePath and associate it with 
        /// aHandle.
        void loadTextureFromImageFile( const char* aTextureFilePath, 
                                       const TextureName& aHandle );
        /// Load a 3D texture from the given volume data.
        /// Can be used to reload the texture from changed data.
        void load3DTextureFromVolumeData( const TextureName& aHandle,
                                          VolumeDataPtr aVolume );
        /// Returns true if the given texture handle is loaded and bound to a
        /// texture unit.
        bool isTextureReady( const TextureName& aHandle );

        /// Returns the Texture Unit that the given texture is currently bound to.
        /// If handle is valid and texture is not currently bound to a unit, 
        /// then the next unit will be assigned on a least-recently used basis.
        GLint getTextureUnitForHandle( const TextureName& aHandle );    
    
        /// Returns OpenGL's texture ID (aka texture name) for the given handle.
        /// Returns -1 if handle is not found.
        GLuint getTextureIdForHandle( const TextureName& aHandle ) const;
    
        /// Returns the handle string for the texture ID (aka Texture Name).
        /// Returns "__ERROR__TEXTURE_NOT_REGISTERED" if aTextureId isn't found.
        TextureName getTextureHandleFromTextureId( GLuint aTextureId ) const;

        /// Parameters set per-texture handle
        void setTextureParameteri( const TextureName& aHandle, GLenum target,
            GLenum paramName, GLint param );    
        /// Clients must call releaseAll() explicitly (rather than to rely on
        /// destructor) to ensure order of release.
        void releaseAll( void );
    
        /// Provides the unit and texture id for the texture with the given 
        /// handle.  If handle is loaded but not bound to a texture unit, 
        /// it will be bound by this method and the bound unit returned.
        /// Sets both unit and id to -1 if aHandle is not a valid texture.
        void acquireTextureUnitAndId( const TextureName& aHandle,
                                      GLint& outTextureUnit,
                                      GLuint& outTextureId );
    private:
        /// Calls glActiveTexture and glBindTexture to bind the ID to the unit.
        /// Records binding in m_bindingTextureUnitToTextureId
        void bindTextureIdToUnit( GLint aTextureId, 
                                  GLint aTextureUnit, 
                                  GLenum aTextureType = GL_TEXTURE_2D );
        /// Returns a texture unit that is ready to be bound.
        /// Calling may invalidate old texture bindings, but unbinding
        /// occurs on a least-recently bound basis.
        GLint reserveTextureUnit( void );
        /// Returns (unsigned int)-1 if texture unit has not been bound to a texture id yet.
        GLuint getTextureIdBoundToUnit( GLint aTextureUnit ) const;
        /// Returns (unsigned int)-1 if texture id has not been bound to a texture unit yet.
        /// See ensureTextureUnitBoundToId( GLunit aTextureId )
        GLint getTextureUnitBoundToId( GLuint aTextureId ) const;
        /// Returns the texture unit that aTextureId is bound to.  If aTextureId
        /// isn't bound to a texture unit, allocate next texture unit and bind.
        GLint ensureTextureUnitBoundToId( GLuint aTextureId );
    private:
        FileAssetFinderPtr m_finder;
        /// Registry maps string handles/names to texture "ID"s (sometimes called 
        /// texture names in GL docs).
        /// name -> texture ID
        std::map< const TextureName, GLuint > m_registry;
        /// Stores the type (GL_TEXTURE_2D, GL_TEXTURE_3D, etc) for the texture
        /// with the given texture ID.
        /// texture ID -> texture type
        std::map< GLuint, GLenum > m_textureType;
        /// Store paths used for loading in order to reload or watch
        std::map< const TextureName, std::string > m_paths;
        /// Stores the textureId (second) known to be bound to a given 
        /// texture unit (first).
        std::vector< std::pair<GLint, GLuint> > m_bindingTextureUnitToTextureId;

        GLint m_maxTextureUnits;
        GLint m_nextAvailableTextureUnit;
    };
    typedef std::shared_ptr< TextureManager > TextureManagerPtr;
} // end namespace spark
#endif

