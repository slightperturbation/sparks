#ifndef sparks_FILEASSETFINDER_HPP
#define sparks_FILEASSETFINDER_HPP

#include <boost/filesystem.hpp>

namespace spark
{
    /// Recursively search for a given file beneath dirPath.
    /// Sets pathFound and returns true iff file is found beneath dirPath.
    bool findFileRecursively( const boost::filesystem::path& dirPath, 
                              const std::string& fileName, 
                              boost::filesystem::path& pathFound );

    /// FileAssetFinder encapsulates searching for files on the filesystem
    /// and manages a set of directories to search.  Directories can 
    /// optionally be searched recursively.
    class FileAssetFinder
    {
    public:
        /// Default constructed FileAssetFinder isn't useful immediately.
        /// Add search paths before attempting to find files.
        FileAssetFinder() {}

        /// Tells the FileAssertFinder to search the given path in future
        /// calls to findFile().  
        void addSearchPath( const std::string& aPath );

        /// Tells the FileAssertFinder to search the given path and
        /// all subdirectories of the given path in future calls to findFile().  
        void addRecursiveSearchPath( const std::string& aPath );

        /// Search for file aFilename within all known path directories.
        /// Return full path in aFoundPath and return true iff found.
        /// Return false if file not found.
        bool findFile( const std::string& aFilename, std::string& aFoundPath ) const;
        
        /// Returns all of the current search paths, expanding
        /// recursive directories.
        std::vector< std::string > getSearchPaths( void ) const;
    private:
        /// String for the path to search in, flag for recursively or not
        std::vector< std::pair< std::string, bool > > m_paths;
    };
} // end namespace spark
#endif