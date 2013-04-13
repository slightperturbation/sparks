//#define CPPLOG_THREADING
//#define CPPLOG_FILTER_LEVEL               LL_WARN
//#define CPPLOG_FILTER_LEVEL               LL_ERROR
#include "cpplog.hpp"

// Define the global Logger
cpplog::FileLogger g_log( "sparks.log" );

///////////////////////////////////////////////////////////////////////////
#define BOOST_TEST_MODULE SparksUnitTestSuite
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>


#include "Renderable.hpp"
#include "RenderCommand.hpp"
#include "Shader.hpp"

BOOST_AUTO_TEST_SUITE( RenderingSuite )

BOOST_AUTO_TEST_CASE( DummyTest )
{
    BOOST_CHECK(1 == 1);
}

BOOST_AUTO_TEST_SUITE_END()
