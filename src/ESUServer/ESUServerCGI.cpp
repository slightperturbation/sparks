
#include "ESUInput.hpp"
#include "ESUInputFromSharedMemory.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <string>
#include <cstdlib>
#include <boost/version.hpp>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include <map>
#undef main

using namespace boost::interprocess;
using namespace spark;


/// Main CGI function
/// See 
int main(int argc, char *argv[])
{
    std::cout << "Content-type: text/html\n\n";  //"mode=blend&cut=12&coag=34";

    char* queryString =  getenv( "QUERY_STRING" );
    if( !queryString )
    {
        std::cout << "Error: null query string.";
        std::cout.flush();
        return 0;
    }
    std::string s( queryString );

    // Open the managed segment to share our results
    try
    {
        managed_shared_memory segment( open_only, g_sharedMemorySectionName );
        std::pair< ESUState*, managed_shared_memory::size_type > res;
        offset_t memSize = segment.get_size();
        if( memSize )
        {
            res = segment.find< ESUState >( g_sharedMemoryObjectName );
            // Length should be 1
            if( res.second != 1 )
            {
                // Failure!
                std::cout << "Error: Unable to find shared memory ESUInput object: \"" << g_sharedMemoryObjectName << "\".";
                std::cout.flush();
                return 0;
            }
            ESUState& state = *(res.first);
            // Update state based on input
            if( s == "coagUp" )   state.m_coagWattage += state.m_wattageDelta;
            if( s == "coagDown" ) state.m_coagWattage -= state.m_wattageDelta;

            if( s == "cutUp" )    state.m_cutWattage  += state.m_wattageDelta;
            if( s == "cutDown" )  state.m_cutWattage  -= state.m_wattageDelta;

            if( s == "modeBlend" ) state.m_mode = ESUInput::ESUMode::Blend;
            if( s == "modeCut" )   state.m_mode = ESUInput::ESUMode::Cut;
            if( s == "modeCoag" )  state.m_mode = ESUInput::ESUMode::Coag;

            if( s == "electrodeSpatula" )    state.m_electrode = ESUInput::ElectrodeType::Spatula;
            if( s == "electrodeNeedle" )     state.m_electrode = ESUInput::ElectrodeType::Needle;
            if( s == "electrodeBall3mm" )    state.m_electrode = ESUInput::ElectrodeType::Ball3mm;
            if( s == "electrodeBall5mm" )    state.m_electrode = ESUInput::ElectrodeType::Ball5mm;
            if( s == "electrodeLapHook" )    state.m_electrode = ESUInput::ElectrodeType::LapHook;
            if( s == "electrodeLapSpatula" ) state.m_electrode = ESUInput::ElectrodeType::LapSpatula;
            //////////////////////////////////////////////////////////////
            // Report new state
            std::cout << "cut=" << state.m_cutWattage 
                << "&coag=" << state.m_coagWattage;
            switch( state.m_mode )
            {
            case ESUInput::ESUMode::Blend: 
                std::cout << "&mode=blend"; 
                break;
            case ESUInput::ESUMode::Cut: 
                std::cout << "&mode=cut"; 
                break;
            case ESUInput::ESUMode::Coag: 
                std::cout << "&mode=coag"; 
                break;
            }
            //switch( state.m_electrode )
            //{
            //case ESUInput::ElectrodeType::Spatula:
            //    std::cout << "&electrode=spatula";
            //    break;
            //case ESUInput::ElectrodeType::Needle:
            //    std::cout << "&electrode=needle";
            //    break;
            //case ESUInput::ElectrodeType::Ball3mm:
            //    std::cout << "&electrode=ball3mm";
            //    break;
            //case ESUInput::ElectrodeType::Ball5mm:
            //    std::cout << "&electrode=ball5mm";
            //    break;
            //case ESUInput::ElectrodeType::LapHook:
            //    std::cout << "&electrode=lapHook";
            //    break;
            //case ESUInput::ElectrodeType::LapSpatula:
            //    std::cout << "&electrode=lapSpatula";
            //    break;
            //}
        }
    }
    catch( ... )
    {
        std::cout << "Error: unable to open shared memory segment.";
    }
    // Must flush to avoid webserver hangs
    std::cout.flush();
    return 0;
}

