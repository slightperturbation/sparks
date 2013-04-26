//
//  ShaderManager.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_ShaderManager_hpp
#define sparks_ShaderManager_hpp

#include "SoftTestDeclarations.hpp"
#include "FileAssetFinder.hpp"

#include <map>

/// ShaderManager keeps track of the OpenGL shader resources
/// and the original files used to load those shaders.
/// Note:  automatically binds output fragment buffer 0 to "outColor"
///TODO $$$ Need to own or allow callbacks to Shader objects, because
/// when ShaderManager detects a new file and reloads, the Shader Object
/// needs to call lookupUniformLocations().  Note this breaks the current
/// attempt to isolate the Manager from the actual shader class...
class ShaderManager
{
    struct ShaderFilePaths
    {
        std::string vertexFilePath;
        std::string fragmentFilePath;
    };
public:
    ShaderManager( void ) {}
    ShaderManager( FileAssetFinderPtr finder ) : m_finder( finder ) {}
    ~ShaderManager();

    unsigned int getProgramIndexForShaderName( const ShaderName& name )
    {
        auto iter = m_registry.find( name );
        if( iter == m_registry.end() )
        {
            LOG_ERROR(g_log) << "Failed to find shader by name \"" << name
            << "\".  Not loaded?";
            assert( false );
        }
        return iter->second;
    }
    /// Set the asset finder, giving a set of paths for texture files.
    void setAssetFinder( FileAssetFinderPtr finder ) { m_finder = finder; }
    /// Create a new shader from the vertex and fragment files given.
    void loadShaderFromFiles( const ShaderName& aHandle,
                              const char* aVertexFilePath,
                              const char* aFragmentFilePath );
    /// Reload the shader from original files.
    void reloadShader( const ShaderName& aHandle );
    void reloadAllShaders( void );
    /// Clients must call releaseAll() explicitly (rather than to rely on
    /// destructor) to ensure order of release.
    void releaseAll( void );
private:
    FileAssetFinderPtr m_finder;
    std::map< const ShaderName, unsigned int > m_registry;
    std::map< const ShaderName, ShaderFilePaths > m_files;
};

#endif
