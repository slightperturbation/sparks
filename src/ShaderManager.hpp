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
class ShaderManager : public std::enable_shared_from_this< ShaderManager >
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

    /// Create a new ShaderInstance for the given shader.
    ShaderInstancePtr createShaderInstance( const ShaderName& name );

    unsigned int getProgramIndexForShaderName( const ShaderName& name );

    /// Set the asset finder, giving a set of paths for texture files.
    void setAssetFinder( FileAssetFinderPtr finder ) { m_finder = finder; }
    /// Create a new shader from the vertex and fragment files given.
    void loadShaderFromFiles( const ShaderName& aHandle,
                              const char* aVertexFilePath,
                              const char* aFragmentFilePath );
    /// Reload the shader from original files.
    void reloadShader( const ShaderName& aHandle );
    void reloadAllShaders( void );
    void refreshUniformLocations( void );
    /// Clients must call releaseAll() explicitly (rather than to rely on
    /// destructor) to ensure order of release.
    void releaseAll( void );
private:
    FileAssetFinderPtr m_finder;
    std::map< const ShaderName, unsigned int > m_registry;
    std::map< const ShaderName, ShaderFilePaths > m_files;
    std::vector< std::weak_ptr< ShaderInstance > > m_shaderInstances;
};

#endif
