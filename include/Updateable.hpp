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
        
        /// update() is called once per rendering frame.
        /// Failure to return quickly may cause stuttering frames.  Long
        /// running computations should be done in fixedUpdate()
        /// dt provides a rough estimate of the wall-clock time 
        /// that has elapsed since the previous call to update().
        virtual void update( float dt ) {}

        /// Called no faster than a fixed regular interval to handle 
        /// long-running computations.
        /// dt will always have the same value, even if there are longer
        /// or shorter wall-clock time elapses between calls.
        /// Should not call any OpenGL functions directly!
        /// See TextureManager::queue*() functions for allowed calls.
        virtual void fixedUpdate( float dt ) {}
    private:
        /// Name used for debugging/logging output.
        std::string m_debugName;
    };
}

#endif
