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
        enum ElectrodeType { Spatula, Needle, LapHook, LapSpatula, Ball3mm, Ball5mm };
        virtual ~ESUInput() {}
        
        virtual ElectrodeType electrode( void ) const = 0;
        virtual double cutWattage( void ) const = 0;
        virtual double coagWattage( void ) const = 0;
        virtual ESUMode mode( void ) const = 0;
    };    
    
    /// A single instance of ESUState is put into shared memory.
    struct ESUState
    {
        ESUState( const ESUInput::ElectrodeType electrode,
                  const ESUInput::ESUMode& mode,
                  double cutWattage,
                  double coagWattage )
        : m_electrode( electrode ),
          m_mode( mode ),
          m_cutWattage( cutWattage ),
          m_coagWattage( coagWattage ),
          m_wattageDelta( 10 )
        {}
        
        ESUInput::ElectrodeType m_electrode;
        ESUInput::ESUMode m_mode;
        double m_cutWattage;
        double m_coagWattage;
        double m_wattageDelta;
    };
}


#endif
