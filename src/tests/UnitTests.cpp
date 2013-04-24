
#include "SoftTestDeclarations.hpp"

// Define the global Logger
cpplog::BaseLogger* g_log = new cpplog::FileLogger( "unit_test.log" );

#define BOOST_TEST_MODULE SparksUnitTestSuite

///////////////////////////////////////////////////////////////////////////
// Template for new test files:
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "SoftTestDeclarations.hpp"

BOOST_AUTO_TEST_SUITE( DummySuite )

bool func( int a, int b, int c ) { return a < b && b < c; }

BOOST_AUTO_TEST_CASE( DummyTest )
{
    // Some examples of using BOOST unit tests
    // 
    // BOOST_[WARN|CHECK|REQUIRE]_[|EQUAL|CLOSE|GE|GT|LE|LT|NE|SMALL|PREDICATE]
    // 
    // WARN:  error not counted, continues
    // CHECK: error counted, continues
    // REQUIRE: error counted, aborts
    // 
    BOOST_TEST_MESSAGE( "Dummy tests." );
    BOOST_CHECK( 1 == 1 );
    BOOST_CHECK_MESSAGE( 1 == 1, "Check that " << 1 << " equals " << 1 );
    BOOST_CHECK_EQUAL( 1, 1 );
    BOOST_TEST_CHECKPOINT( "Testing addition to " << 4 ); 
    BOOST_REQUIRE( 2+2 == 4 );

    BOOST_CHECK_CLOSE( 1.00000001f, 1.0f, 0.1 /* percentage units */ );
    BOOST_CHECK_PREDICATE( func, (1)(2)(3) );
    //BOOST_CHECK_EQUAL_COLLECTIONS( )
}

BOOST_AUTO_TEST_SUITE_END()
///////////////////////////////////////////////////////////////////////////
