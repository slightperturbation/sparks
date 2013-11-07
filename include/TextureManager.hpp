
#ifndef SPARKS_TEXTUREMANAGER_HPP
#define SPARKS_TEXTUREMANAGER_HPP

#include "Spark.hpp"
#include "Utilities.hpp"
#include "FileAssetFinder.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <string>
#include <map>
#include <set>

namespace spark
{
    /// Loads and manages OpenGL textures.
    /// Avoids redundant glBindTexture calls.
    /// Texture units are bound to textures on a first-in-first-out basis.
    /// Typical usage:
    /// In init
    ///   tm->loadTextureFromImageFile( "fire_color", "fire.png" );
    /// During rendering (in Material::use()):
    ///   GLint texUnit = tm->getTextureUnitForHandle( "fire_color" );
    ///   m_shader->setUniform( "s_color", texUnit );
    /// Where "s_color" is the shader's texture sampler.
    class TextureManager
    : public spark::enable_shared_from_this< TextureManager >
    {
        /// Commands can be queue'd up by other threads
        /// to allow the OpenGL-attached thread to 
        /// dispatch.
        struct TextureManagerCommand
        {
            TextureManagerCommand( const TextureName& aHandle )
                : m_handle( aHandle ),
                  m_creationTime( glfwGetTime() )
            {}
            virtual ~TextureManagerCommand() {}
            virtual void operator()( TextureManager* tm ) const = 0;
            const TextureName m_handle;
            const double m_creationTime;
        };
        typedef spark::shared_ptr< TextureManagerCommand > TextureManagerCommandPtr;

        /// Comparisons between TextureManagerCommands are only on the 
        /// handle, *not* on the creation timestamp.
        struct TextureManagerCommandComparator
        {
            bool operator()( const TextureManagerCommandPtr& lhs, 
                             const TextureManagerCommandPtr& rhs )
            {
                return lhs->m_handle < rhs->m_handle;
            }
        };

        struct Load3DTextureFromVolumeDataCommand
            : public TextureManagerCommand
        {
            Load3DTextureFromVolumeDataCommand( const TextureName& aHandle,
                                                VolumeDataPtr& volumeData )
            : TextureManagerCommand( aHandle ), m_volumeData( volumeData ) 
            { }
            virtual void operator()( TextureManager* tm ) const override
            {
                tm->load3DTextureFromVolumeData( m_handle, m_volumeData );
            }
            const VolumeDataPtr m_volumeData; //< TODO -- should the data be copied? 
        };

        struct Load2DByteTextureFromDataCommand
        : public TextureManagerCommand
        {
             Load2DByteTextureFromDataCommand( const TextureName& aHandle,
                                  const std::vector<unsigned char>& aData,
                                  size_t dimPerSide )
            : TextureManagerCommand( aHandle ),
              m_dimPerSide( dimPerSide ),
              m_useBackgroundLoad( false )
            {
                m_data.assign( aData.begin(), aData.end() ); 
            }
            virtual void operator()( TextureManager* tm ) const override
            {
                if( m_useBackgroundLoad )
                {
                    tm->backgroundLoad2DByteTextureFromData( m_handle, m_data, m_dimPerSide );
                }
                else
                {
                    tm->load2DByteTextureFromData( m_handle, m_data, m_dimPerSide );
                }
            }
            std::vector<unsigned char> m_data;
            const size_t m_dimPerSide;
            bool m_useBackgroundLoad;
        };
        // TODO -- template on data type; need to overload load method for data type in support.
        struct Load2DFloatTextureFromDataCommand
        : public TextureManagerCommand
        {
             Load2DFloatTextureFromDataCommand( const TextureName& aHandle,
                                  const std::vector<float>& aData,
                                  size_t dimPerSide )
            : TextureManagerCommand( aHandle ),
              m_dimPerSide( dimPerSide )
            {
                m_data.reserve( aData.size() );
                for( auto iter = aData.begin(); iter != aData.end(); ++iter )
                {
                    m_data.push_back(*iter);
                }
            }
            virtual void operator()( TextureManager* tm ) const override
            {
                tm->load2DFloatTextureFromData( m_handle, m_data, m_dimPerSide );
            }
            std::vector<float> m_data;
            const size_t m_dimPerSide;
        };
        std::set< TextureManagerCommandPtr, TextureManagerCommandComparator > m_commandQueue;

        // Must always acquire commandQueueMutex first, if both are acquired.
        mutable boost::mutex m_commandQueueMutex;
        mutable boost::recursive_mutex m_registryMutex;
    public:
        TextureManager( void );

        // Disable copy constructor & assignment op
        TextureManager( const TextureManager& ); // No impl
        const TextureManager& operator= ( const TextureManager& ); // No impl
        TextureManager( FileAssetFinderPtr finder ) : m_finder( finder ) { }
        virtual ~TextureManager();

        //////////////////////////////////////////////////////////////////
        /// Non-OpenGL-Thread Methods
        /// Some methods may call TextureManager from a thread not 
        /// associated with the OpenGL context.
        /// The following calls can be made from Updateable::fixedUpdate(dt)
        /// or other threads safely, as these calls queue-up requests
        /// that are dispatched in postUpdate() 
        void queueLoad3DTextureFromVolumeData( const TextureName& aHandle,
                                               VolumeDataPtr aVolume );
        void queueLoad2DByteTextureFromData( const TextureName& aHandle, 
                                             const std::vector<unsigned char>& aData, 
                                             size_t dimPerSide );
        void queueLoad2DFloatTextureFromData( const TextureName& aHandle,
                                              const std::vector<float>& aData,
                                              size_t dimPerSide );
        //////////////////////////////////////////////////////////////////
        /// Execute all of the queued commands for this TextureManager.
        /// Can only be called on the OpenGL thread.
        /// See also queueLoad* methods.
        void executeQueuedCommands( void );
        
        /// Execute a single command for this TextureManager.
        /// Returns true if additional commands remain in the queue
        /// after a single one has been executed and removed.
        /// Can only be called on the OpenGL thread.
        bool executeSingleQueuedCommand( void );
        //////////////////////////////////////////////////////////////////

    
        /// Set the asset finder, giving a set of paths for texture files.
        void setAssetFinder( FileAssetFinderPtr finder ) { m_finder = finder; }
    
        FileAssetFinderPtr assetFinder( void ) { return m_finder; }
        
        /// Create a 2D texture with the given handle that serves as a
        /// texture to render to.
        /// Before calling, the FBO must be bound using glBindFramebuffer()
        void createTargetTexture( const TextureName& aHandle,
                                  int width, int height );

        /// Create a depth-only texture-target.  E.g., for shadow mapping
        /// Before calling, the FBO must be bound using glBindFramebuffer()
        void createDepthTargetTexture( const TextureName& aHandle,
                                       int width, int height );
        
        /// Load a simple test texture.
        void loadTestTexture( const TextureName& aHandle,
                              GLint textureUnit = -1  );

        /// Load a simple 3x3 checkerboard test texture.
        void loadCheckerTexture( const TextureName& aHandle,
                                 GLint textureUnit = -1 );

        /// Allows a texture that has already been loaded into OpenGL
        /// to be tracked by TextureManager.
        /// Note that this texture is now owned by TextureManager,
        /// and will be freed on TextureManager's releaseAll()
        void acquireExternallyAllocatedTexture( const TextureName& aName,
                                                GLuint aTextureId,
                                                GLenum aTextureType = GL_TEXTURE_2D,
                                                GLint aTextureUnit = -1 );
        /// Returns a texture unit that is ready to be bound.
        /// Useful for loading externally managed textures.
        /// Calling may invalidate old texture bindings, but unbinding
        /// occurs on a least-recently bound basis.
        GLint reserveTextureUnit( void );

        /// Load the image file at aTextureFilePath and associate it with 
        /// aHandle.
        /// TODO provide aspect ratio (and size?) for loaded texture.
        void loadTextureFromImageFile( const TextureName& aHandle, 
                                       const char* aTextureFileName );
        /// Load a 3D texture from the given volume data.
        /// Can be used to reload the texture from changed data.
        void load3DTextureFromVolumeData( const TextureName& aHandle,
                                          VolumeDataPtr aVolume );
        /// Load 2D texture from given data source 
        /// aData is a vector of unsigned bytes.
        void load2DByteTextureFromData( const TextureName& aHandle,
                                        const std::vector<unsigned char>& aData,
                                        size_t dimPerSide );
        /// Load texture using a "background" texture, then swap when done.
        void backgroundLoad2DByteTextureFromData( const TextureName& aHandle,
            const std::vector<unsigned char>& aData,
            size_t dimPerSide );

        void load2DFloatTextureFromData( const TextureName& aHandle,
                                         const std::vector<float>& aData,
                                         size_t dimPerSide );
        /// Returns true if the given texture handle is loaded and bound to a
        /// texture unit.
        bool isTextureReady( const TextureName& aHandle );
        
        /// Returns true if the given texture handle is currently associated
        /// with a texture.
        bool exists( const TextureName& aHandle ) const;

        /// Returns the Texture Unit that the given texture is currently bound to.
        /// If handle is valid and texture is not currently bound to a unit, 
        /// then the next unit will be assigned on a least-recently used basis.
        /// Note: Changes OpenGL state ActiveTexture unit.
        GLint getTextureUnitForHandle( const TextureName& aHandle );  

        /// Changes OpenGL's state of ActiveTexture to the unit containing
        /// aHandle.  aHandle is assigned a unit if not already.
        /// Primarily useful for working with non-spark code.
        /// See acquireExternallyAllocatedTexture()
        void activateTextureUnitForHandle( const TextureName& aHandle );
    
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
        /// Remove the texture with the given handle.
        void deleteTexture( const TextureName& aHandle );
        
        /// Log all loaded textures
        void logTextures( void ) const;
    private:
        /// Calls glActiveTexture and glBindTexture to bind the ID to the unit.
        /// Records binding in m_bindingTextureUnitToTextureId
        void bindTextureIdToUnit( GLint aTextureId, 
                                  GLint aTextureUnit, 
                                  GLenum aTextureType = GL_TEXTURE_2D );
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
    typedef spark::shared_ptr< TextureManager > TextureManagerPtr;
} // end namespace spark
#endif

