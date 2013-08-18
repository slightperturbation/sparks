#include "State.hpp"

spark::State
::State( const StateName& name )
: m_name( name )
{
    
}

spark::State
::~State()
{
    
}

spark::StateName
spark::State
::name( void ) const
{
    return m_name;
}

