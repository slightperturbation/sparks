#ifndef sparks_FILEASSETFINDER_HPP
#define sparks_FILEASSETFINDER_HPP

#include <boost/filesystem.hpp>

/// Recursively search for a given file beneath dirPath.
/// Sets pathFound and returns true iff file is found beneath dirPath.
bool findFileRecursively( const boost::filesystem::path& dirPath, 
                          const std::string& fileName, 
                          boost::filesystem::path& pathFound );

/// Keeps track of the search paths and hides boost::filesystem calls.
class FileAssetFinder
{
public:
    void addSearchPath( const std::string& aPath );
    /// Search for file aFilename within all known path directories.
    /// Return full path in aFoundPath and return true iff found.
    /// Return false if file not found.
    bool findFile( const std::string& aFilename, std::string& aFoundPath ) const;
private:
    /// String for the path to search in, flag for recursively or not
    std::vector< std::pair< std::string, bool > > m_paths;
};


#endif