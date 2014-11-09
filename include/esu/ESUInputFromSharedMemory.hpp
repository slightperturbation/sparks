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
    
    const double g_defaultCoagPower = 30.0;
    const double g_defaultCutPower = 30.0;
    
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
            double cut = g_defaultCutPower; // default value
            try
            {
                managed_shared_memory segment( open_only, g_sharedMemorySectionName );
                std::pair< const ESUState*, managed_shared_memory::size_type > res 
                    = segment.find<ESUState>( g_sharedMemoryObjectName );
                if( res.first && res.second )
                {
                    cut = res.first->m_cutWattage;
                }
            }
            catch( ... )
            {
                LOG_ERROR(g_log) << "Failed to acquire Shared ESU State.";
                shared_memory_object::remove( g_sharedMemorySectionName );
            }
            return cut;
        }

        virtual double coagWattage( void ) const
        {
            using namespace boost::interprocess;
            double coag = g_defaultCoagPower;
            try
            {
                managed_shared_memory segment( open_only, g_sharedMemorySectionName );
                std::pair< const ESUState*, managed_shared_memory::size_type > res 
                    = segment.find<ESUState>( g_sharedMemoryObjectName );
                if( res.first && res.second )
                {
                    coag = res.first->m_coagWattage;
                }
            }
            catch( ... )
            {
                LOG_ERROR(g_log) << "Failed to acquire Shared ESU State.";
                shared_memory_object::remove( g_sharedMemorySectionName );
            }
            return coag;
        }
        
        virtual ESUMode mode( void ) const
        {
            using namespace boost::interprocess;
            ESUMode mode = ESUMode::Coag;
            try
            {
                managed_shared_memory segment( open_only, g_sharedMemorySectionName );
                std::pair< const ESUState*, managed_shared_memory::size_type > res 
                    = segment.find<ESUState>( g_sharedMemoryObjectName );
                if( res.first && res.second )
                {
                    mode = res.first->m_mode;
                }
            }
            catch( ... )
            {
                LOG_ERROR(g_log) << "Failed to acquire Shared ESU State.";
                shared_memory_object::remove( g_sharedMemorySectionName );
            }
            return mode;
        }
        
        virtual ElectrodeType electrode( void ) const
        {
            using namespace boost::interprocess;
            ElectrodeType electrode = ElectrodeType::Spatula;
            try
            {
                managed_shared_memory segment( open_only, g_sharedMemorySectionName );
                std::pair< const ESUState*, managed_shared_memory::size_type > res 
                    = segment.find<ESUState>( g_sharedMemoryObjectName );
                if( res.first && res.second )
                {
                    electrode = res.first->m_electrode;
                }
            }
            catch( ... )
            {
                LOG_ERROR(g_log) << "Failed to acquire Shared ESU State.";
                shared_memory_object::remove( g_sharedMemorySectionName );
            }
            return electrode;
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
                                              65536 ); // 65Kb should be plenty

                //Create an object of ESUState initialized to {Blend, 30.0f cut power, 30.0f coag power}
                segment.construct< ESUState >
                    ( g_sharedMemoryObjectName )  //name of the object
                    ( ElectrodeType::Spatula, ESUMode::Blend, g_defaultCutPower, g_defaultCoagPower ); //constructor arguments
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
