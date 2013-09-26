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
    /// Interface for changing Electrosurgical unit (ESU) settings.
    class ESUInput
    {
    public:
        enum ESUMode { Cut, Coag, Blend };
        enum ElectrodeType { Hook, Ball2mm, Ball4mm };
        virtual ~ESUInput() {}
        
        virtual ElectrodeType electrode( void ) const = 0;
        virtual double wattage( void ) const = 0;
        virtual ESUMode mode( void ) const = 0;
    };    
    
    struct ESUState
    {
        ESUState( const ESUInput::ElectrodeType electrode,
                  const ESUInput::ESUMode& mode,
                  double wattage )
        : m_electrode( electrode ),
          m_mode( mode ),
          m_wattage( wattage ) {}
        
        ESUInput::ElectrodeType m_electrode;
        ESUInput::ESUMode m_mode;
        double m_wattage;
    };
}


#endif
