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

        virtual double wattage( void ) const
        {
            using namespace boost::interprocess;
            managed_shared_memory segment( open_or_create, g_sharedMemorySectionName, 65536 );
            std::pair< const ESUState*, managed_shared_memory::size_type > res = segment.find<ESUState>( g_sharedMemoryObjectName );
            if( res.second && res.first )
            {
                return res.first->m_wattage;
            }
            return 20.0;
        }
        
        virtual ESUMode mode( void ) const
        {
            using namespace boost::interprocess;
            managed_shared_memory segment( open_or_create, g_sharedMemorySectionName, 65536 );
            std::pair< const ESUState*, managed_shared_memory::size_type > res = segment.find<ESUState>( g_sharedMemoryObjectName );
            if( res.second && res.first )
            {
                return res.first->m_mode;
            }
            return ESUMode::Coag;
        }
        
        virtual ElectrodeType electrode( void ) const
        {
            using namespace boost::interprocess;
            managed_shared_memory segment( open_or_create, g_sharedMemorySectionName, 65536 );
            std::pair< const ESUState*, managed_shared_memory::size_type > res = segment.find<ESUState>( g_sharedMemoryObjectName );
            if( res.second && res.first )
            {
                return res.first->m_electrode;
            }
            return ElectrodeType::Hook;
        }
    private:
        ESUInputFromSharedMemory( void )
       {
            using namespace boost::interprocess;
            shared_memory_object::remove( g_sharedMemorySectionName );
            try
            {
                //Create a new segment with given name and size
                managed_shared_memory segment( open_or_create, g_sharedMemorySectionName, 65536 );

                //Create an object of ESUState initialized to {Cut, 0}
                ESUState* sharedState = segment.construct< ESUState >
                    ( g_sharedMemoryObjectName )  //name of the object
                    ( ElectrodeType::Hook, ESUMode::Coag, 30 );            //constructor arguments
                sharedState->m_wattage = 30.0f;
            }
            catch( ... )
            {
                shared_memory_object::remove( g_sharedMemorySectionName );
                LOG_ERROR(g_log) << "Failed to allocate shared memory for ESUInput.";
            }
        }
        ESUInputFromSharedMemory( const ESUInputFromSharedMemory& ); // disabled
        ESUInputFromSharedMemory& operator=( const ESUInputFromSharedMemory& ); // disabled
    };
}

#endif
