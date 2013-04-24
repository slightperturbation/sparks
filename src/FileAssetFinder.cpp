#include "FileAssetFinder.hpp"
#include "SoftTestDeclarations.hpp"


bool findFileRecursively( const boost::filesystem::path& dirPath, 
                          const std::string& fileName, 
                          boost::filesystem::path& pathFound )
{
    namespace bfs = boost::filesystem;
    try
    {
        for( bfs::recursive_directory_iterator itr( dirPath ), end;
             itr != end; ++itr )
        {
            if( itr->path().filename() == fileName )
            {
                pathFound = itr->path();
                return true;
            }
        }
    }
    catch( bfs::filesystem_error& err )
    {
        LOG_ERROR(g_log) << "Unable to find file \""
                         << fileName << "\": " << err.what();
    }
    return false;
}

void 
FileAssetFinder
::addSearchPath( const std::string& aPath )
{
    m_paths.push_back( make_pair( aPath, false ) );
}

void 
FileAssetFinder
::addRecursiveSearchPath( const std::string& aPath )
{
    m_paths.push_back( make_pair( aPath, true ) );
}

bool 
FileAssetFinder
::findFile( const std::string& aFilename, std::string& foundPath ) const
{
    namespace bfs = boost::filesystem;
    for( auto path = m_paths.begin(); path != m_paths.end(); ++path )
    {
        std::string dir = (*path).first;
        try
        {
            if( !bfs::exists(dir) ) continue;
        }
        catch( bfs::filesystem_error err )
        {
            LOG_ERROR(g_log) << "Unable to find path dir \""
                             << dir << "\": " << err.what();
        }
        bool searchRecursively = (*path).second;
        if( bfs::is_directory(dir) )
        {
            if( searchRecursively )
            {
                bfs::path path;
                findFileRecursively( dir, aFilename, path );
                foundPath = path.string();
                return true;
            }
            else
            {
                bfs::path path( dir + aFilename );
                if( bfs::is_regular_file( path ) )
                {
                    foundPath = path.string();
                    return true;
                }
            }
        }
    }
    return false;
}