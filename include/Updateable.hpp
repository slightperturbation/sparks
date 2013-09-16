#ifndef SPARK_UPDATEABLE_HPP
#define SPARK_UPDATEABLE_HPP

#include <string>

namespace spark 
{
    /// Interface for objects that should be regularly allowed to update
    /// their state.
    /// Note that these calls should not cause OpenGL changes, as they
    /// may be executed on different threads.
    class Updateable
    {
    public:
        Updateable( const std::string& name = "no_name" ) : m_debugName( name ) {}
        virtual ~Updateable() {}

        virtual const std::string& updateableName( void ) { return m_debugName; }
        
        /// Update this by dt seconds.
        /// dt provides a rough estimate of the wall-clock time
        /// that has elapsed since the previous call to update().
        /// Should not call any OpenGL functions directly!
        /// See TextureManager::queue*() functions for allowed calls.
        virtual void update( double dt ) {}
    private:
        /// Name used for debugging/logging output.
        std::string m_debugName;
    };
}

#endif
