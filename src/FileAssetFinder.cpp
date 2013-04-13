#include "FileAssetFinder.hpp"


bool findFileRecursively( const boost::filesystem::path& dirPath, 
                          const std::string& fileName, 
                          boost::filesystem::path& pathFound )
{
    using namespace boost::filesystem;
    bool successFlag = false;
    if( exists(dirPath) && is_directory(dirPath) )
    {
        directory_iterator iter(dirPath), end_iter;
        for(; iter!= end_iter; ++iter)
        {
            if( is_directory(*iter) )
            {
                if( find_file(*iter, fileName, pathFound) )
                    successFlag = true;
                break;
            }
            else if( iter->leaf() == fileName )
            {
                pathFound = *iter;
                successFlag = true;
                break;
            }
        }
        return successFlag;
    }
}

bool 
FileAssetFinder
::findFile( const std::string& aFilename, std::string& foundPath ) const
{
    using namespace boost::filesystem;
    for( auto path = m_paths.begin(); path != m_paths.end(); ++path )
    {
        std::string& dir = (*path).first;
        if( !exists(dir) ) continue;
        bool searchRecursively = (*path).second;
        if( is_directory(dir) && searchRecursively )
        {
            boost::filesystem::path path;
            findFileRecursively( dir, aFilename, path );
            foundPath = path.string();
        }
    }
}