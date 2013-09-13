
#include "ESUInput.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <string>
#include <cstdlib>
#include <boost/version.hpp>
#include <iostream>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/lexical_cast.hpp>

#include <map>

using namespace boost::interprocess;
using namespace spark;

namespace qi = boost::spirit::qi;

/// Parser for processing the query string
namespace queryProcessing
{
    typedef std::map<std::string, std::string> pairs_type;

    template <typename Iterator>
    struct key_value_sequence 
        : qi::grammar<Iterator, pairs_type()>
    {
        key_value_sequence()
            : key_value_sequence::base_type(query)
        {
            query =  pair >> *((qi::lit(';') | '&') >> pair);
            pair  =  key >> -('=' >> value);
            key   =  qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9\\.\\-\\+");
            value = +qi::char_("a-zA-Z_0-9\\.\\-\\+");
        }

        qi::rule<Iterator, pairs_type()> query;
        qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
        qi::rule<Iterator, std::string()> key, value;
    };
}

/// Main CGI function-- expect args from 
int main(int argc, char *argv[])
{
    std::cout << "Content-type: text/html\n\n";
    
    char* queryString =  getenv( "QUERY_STRING" );
    if( !queryString )
    {
        std::cout << "Error: null query string.";
        return 0;
    }
    std::string s( queryString );

    std::cout << "Debug: QUERY_STRING=\"" << s << "\"<br>";

    queryProcessing::key_value_sequence<std::string::iterator> p;
    queryProcessing::pairs_type dataMap;

    if( !qi::parse(s.begin(), s.end(), p, dataMap) )
    {
        std::cout << "Error: unable to parse arguments.<br>";
        return 0;
    }

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
                std::cerr << "Error: Unable to find shared memory ESUInput object: \"" << g_sharedMemoryObjectName << "\"<br>";
                return 0;
            }
            ESUState& state = *(res.first);
            {
                // If we got a new wattage, apply it
                queryProcessing::pairs_type::iterator iter = dataMap.find("Wattage");
                if( iter != dataMap.end() )
                {
                    state.m_wattage = boost::lexical_cast< double >( (*iter).second );
                }
            }
            {
                // If we got a new mode, apply it
                queryProcessing::pairs_type::iterator iter = dataMap.find("Mode");
                if( iter != dataMap.end() )
                {
                    const std::string& mode = (*iter).second;
                    if( mode == "Cut" )
                    {
                        state.m_mode = ESUInput::ESUMode::Cut;
                    }
                    if( mode == "Coag" )
                    {
                        state.m_mode = ESUInput::ESUMode::Coag;
                    }
                    if( mode == "Blend" )
                    {
                        state.m_mode = ESUInput::ESUMode::Blend;
                    }
                }
            }
        }
    }
    catch( ... )
    {
        std::cout << "Error: unable to open shared memory segment.";
    }
    
    return 0;
}

