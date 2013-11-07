//
//  ESUInputFromCGI.hpp
//  sparkGui
//
//  Created by Brian Allen on 9/13/13.
//
//

#ifndef sparkGui_ESUInputFromSharedMemory_hpp
#define sparkGui_ESUInputFromSharedMemory_hpp

#include "ESUInput.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <string>
#include <boost/version.hpp>
#include <iostream>

namespace spark
{
    extern const char* g_sharedMemorySectionName;
    extern const char* g_sharedMemoryObjectName;
    
    class ESUInputFromSharedMemory : public ESUInput
    {
    public:
        static ESUInputFromSharedMemory* getPtr( void )
        {
            static ESUInputFromSharedMemory theOne;
            return &theOne;
        }
        static ESUInputFromSharedMemory& get( void )
        {
            return *(getPtr());
        }
        virtual ~ESUInputFromSharedMemory()
        {
            boost::interprocess::shared_memory_object
                ::remove( g_sharedMemorySectionName );
        }

        virtual double cutWattage( void ) const
        {
            using namespace boost::interprocess;
            try
            {
                managed_shared_memory segment( open_only, g_sharedMemorySectionName );
                std::pair< const ESUState*, managed_shared_memory::size_type > res 
                    = segment.find<ESUState>( g_sharedMemoryObjectName );
                if( res.first && res.second )
                {
                    return res.first->m_cutWattage;
                }
                else
                {
                    return 30.0;
                }
            }
            catch( ... )
            {
                LOG_ERROR(g_log) << "Failed to acquire Shared ESU State.";
                shared_memory_object::remove( g_sharedMemorySectionName );
            }
        }

        virtual double coagWattage( void ) const
        {
            using namespace boost::interprocess;
            try
            {
                managed_shared_memory segment( open_only, g_sharedMemorySectionName );
                std::pair< const ESUState*, managed_shared_memory::size_type > res 
                    = segment.find<ESUState>( g_sharedMemoryObjectName );
                if( res.first && res.second )
                {
                    return res.first->m_coagWattage;
                }
                else
                {
                    return 30.0;
                }
            }
            catch( ... )
            {
                LOG_ERROR(g_log) << "Failed to acquire Shared ESU State.";
                shared_memory_object::remove( g_sharedMemorySectionName );
            }
        }
        
        virtual ESUMode mode( void ) const
        {
            using namespace boost::interprocess;
            try
            {
                managed_shared_memory segment( open_only, g_sharedMemorySectionName );
                std::pair< const ESUState*, managed_shared_memory::size_type > res 
                    = segment.find<ESUState>( g_sharedMemoryObjectName );
                if( res.first && res.second )
                {
                    return res.first->m_mode;
                }
                else
                {
                    return ESUMode::Coag;
                }
            }
            catch( ... )
            {
                LOG_ERROR(g_log) << "Failed to acquire Shared ESU State.";
                shared_memory_object::remove( g_sharedMemorySectionName );
            }
        }
        
        virtual ElectrodeType electrode( void ) const
        {
            using namespace boost::interprocess;
            try
            {
                managed_shared_memory segment( open_only, g_sharedMemorySectionName );
                std::pair< const ESUState*, managed_shared_memory::size_type > res 
                    = segment.find<ESUState>( g_sharedMemoryObjectName );
                if( res.first && res.second )
                {
                    return res.first->m_electrode;
                }
                else
                {
                    return ElectrodeType::Spatula;
                }
            }
            catch( ... )
            {
                LOG_ERROR(g_log) << "Failed to acquire Shared ESU State.";
                shared_memory_object::remove( g_sharedMemorySectionName );
            }
        }
    private:
        ESUInputFromSharedMemory( void )
        {
            using namespace boost::interprocess;
            // first, remove any lingering shared object with this name
            shared_memory_object::remove( g_sharedMemorySectionName );
            try
            {
                //Create a new segment with given name and size
                managed_shared_memory segment( open_or_create, 
                    g_sharedMemorySectionName, 
                    65536 );

                //Create an object of ESUState initialized to {Blend, 30.0f cut power, 30.0f coag power}
                ESUState* esu = segment.construct< ESUState >
                    ( g_sharedMemoryObjectName )  //name of the object
                    ( ElectrodeType::Spatula, ESUMode::Blend, 30.0f, 30.0f ); //constructor arguments
            }
            catch( ... )
            {
                shared_memory_object::remove( g_sharedMemorySectionName );
                std::cerr << "Failed to allocate shared memory for ESUInput.\n";
                LOG_ERROR(g_log) << "Failed to allocate shared memory for ESUInput.";
            }
        }

        ESUInputFromSharedMemory( const ESUInputFromSharedMemory& ); // disabled
        ESUInputFromSharedMemory& operator=( const ESUInputFromSharedMemory& ); // disabled
    };
}

#endif
