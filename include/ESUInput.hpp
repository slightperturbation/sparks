//
//  ESUInput.hpp
//  sparkGui
//
//  Created by Brian Allen on 9/13/13.
//
//

#ifndef spark_ESUInput_hpp
#define spark_ESUInput_hpp

#include "Spark.hpp"

namespace spark
{
    const char* g_sharedMemorySectionName = "SPARK_SharedMemory";
    const char* g_sharedMemoryObjectName = "CurrentESUSettings";
    
    /// Interface for changing Electrosurgical unit (ESU) settings.
    class ESUInput
    {
    public:
        enum ESUMode { Cut, Coag, Blend };
        virtual ~ESUInput() {}
        
        virtual double wattage( void ) const = 0;
        virtual ESUMode mode( void ) const = 0;
    };    
    
    struct ESUState
    {
        ESUState( const ESUInput::ESUMode& mode, double wattage )
        : m_mode( mode ), m_wattage( wattage ) {}
        
        ESUInput::ESUMode m_mode;
        double m_wattage;
    };
}


#endif
