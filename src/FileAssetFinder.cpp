#include "FileAssetFinder.hpp"
#include "SoftTestDeclarations.hpp"

#include <boost/algorithm/string.hpp>

bool 
spark
::findFileRecursively( const boost::filesystem::path& dirPath, 
                       const std::string& fileName, 
                       boost::filesystem::path& pathFound )
{
    namespace bfs = boost::filesystem;
    try
    {
        for( bfs::recursive_directory_iterator itr( dirPath ), end;
             itr != end; ++itr )
        {
            if( boost::iequals( itr->path().filename().string(), fileName ) )
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
spark::FileAssetFinder
::addSearchPath( const std::string& aPath )
{
    m_paths.push_back( make_pair( aPath, false ) );
}

void 
spark::FileAssetFinder
::addRecursiveSearchPath( const std::string& aPath )
{
    m_paths.push_back( make_pair( aPath, true ) );
}

bool 
spark::FileAssetFinder
::findFile( const std::string& aFilename, std::string& foundPath ) const
{
    namespace bfs = boost::filesystem;
    LOG_DEBUG(g_log) << "Searching for file: \"" << aFilename << "\".";
    for( auto path = m_paths.begin(); path != m_paths.end(); ++path )
    {
        std::string dir = (*path).first;
        // Getting unexpected crashes on OSX -- maybe conflicted boost lib?
//        boost::filesystem::path dirPath( dir );
//        LOG_DEBUG(g_log) << "\tLooking in dir \"" << dir << "\".";
//        bool isFound = false;
//        try
//        {
//            boost::system::error_code ec;
//            if( bfs::is_directory( dirPath ) )
//            {
//                isFound = true;
//            }
//        }
//        catch( ... )
//        {
//            LOG_ERROR(g_log) << "Unable to find path dir \""
//            << dir << "\".";
//            continue;
//        }
//        if( !isFound )
//        {
//            LOG_INFO(g_log) << "Path not found \"" << dir << "\".";
//            continue;
//        }
        bool searchRecursively = (*path).second;
        if( bfs::is_directory(dir) )
        {
            if( searchRecursively )
            {
                bfs::path fpath;
                findFileRecursively( dir, aFilename, fpath );
                foundPath = fpath.string();
                LOG_DEBUG(g_log) << "Found file \"" << aFilename 
                    << "\" at \"" << foundPath << "\".";
                return true;
            }
            else
            {
                bfs::path fpath( dir + aFilename );
                if( bfs::is_regular_file( fpath ) )
                {
                    foundPath = fpath.string();
                    LOG_DEBUG(g_log) << "Found file \"" << aFilename 
                        << "\" at \"" << foundPath << "\".";
                    return true;
                }
            }
        }
    }
    return false;
}