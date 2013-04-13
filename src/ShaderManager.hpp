//
//  ShaderManager.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_ShaderManager_hpp
#define sparks_ShaderManager_hpp

class ShaderManager
{
public:
    /// Get the singleton ShaderManager
    /// Note: created on first call, but destructor order is undefined,
    /// so primary thread must call "manually" call releaseAll()
    static ShaderManager& get();
    
    ~ShaderManager();
    
    void loadShaderFromFiles( const std::string& aHandle,
                              const char* aVertexFilePath,
                              const char* aFragmentFilePath );
    
    /// Clients must call releaseAll() explicitly (rather than to rely on
    /// destructor) to ensure order of release.
    void releaseAll( void );
private:
    /// Private CTor to ensure singleton access.
    ShaderManager() {}

    std::map< std::string, GLint > m_registry;
d};

#endif
