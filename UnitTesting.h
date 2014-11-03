/*
 * File:   UnitTesting.h
 * Version 0.0 Author: Bernhard Humm
 * Version 0.1 Author: Ronald Moore
 *
 * Version 0.0 created on 15. März 2010, 18:22
 * Version 0.1 created on 28 March 2014
 */

#include <iostream> // for std::cout
#include <sstream>  // for std::stringstream
#include <string>   // for std:.string

#pragma once

namespace testing {

// Semi-global variable (inside namespace testing)
// starts out true and stay true until some test fails...
bool AllTestsSuccessful = true;


/**
 * Tests a condition. Basic Test Function.
 *
 * @param condition test evaluation - any boolean expression can be used here!
 * @param description (optional) testing context
 * @return void - the test result is printed to cout
**/
void assertTrue(bool condition, std::string description = "")  {
    std::cout << "Test "
              << ( condition ? "Successful: "
                             : "FAILURE!!!: " )
              << description << std::endl;
    // Note whether any test has failed...
    AllTestsSuccessful = AllTestsSuccessful && condition;
}

/**
 * Tests whether actual value is equal to expected value.
 *
 * @param expected correct value - must support operator== and operator<<
 * @param actual computed value  - must support operator== and operator<<
 * @param description (optional) testing context
 * @return void - the test result is printed to cout
**/
template < class SomeType >
void assertEquals(SomeType expected, SomeType actual,
                  std::string description = "") {
    std::stringstream explanation;
    explanation << description
                << "; Expected " << expected
                << ", computed " << actual;
    assertTrue( (expected == actual), explanation.str() );
}

/**
 * Tests whether actual value is NOT equal to forbidden value.
 *
 * @param forbidden incorrect value - must support operator== and operator<<
 * @param actual computed value  - must support operator== and operator<<
 * @param description (optional) testing context
 * @return void - the test result is printed to cout
**/
template < class SomeType >
void assertNotEquals(SomeType forbidden, SomeType actual,
                  std::string description = "") {
    std::stringstream explanation;
    explanation << description
                << "; Computed " << actual
                << ", which should not be " << forbidden;
    assertTrue( !(actual == forbidden), explanation.str() );
}

/**
 * Tests whether |actual value - expected value| < tolerance
 *
 * @param expected correct value - must support operators ==, << and -
 * @param actual computed value  - must support operators ==, << and -
 * @param tolerance tolerance  - must support operators ==, << and -
 * @param description (optional) testing context
 * @return void - the test result is printed to cout
**/
template < class SomeType >
void assertCloseEnough(SomeType expected, SomeType actual, SomeType tolerance,
                       std::string description = "") {
    std::stringstream explanation( description );
    explanation << "; Expected " << expected << ", computed instead " << actual
                << ", which is outside tolerance " << tolerance;
    SomeType difference = (expected - actual);
    assertTrue( ((difference < tolerance) || (-tolerance < -difference)) ,
                explanation.str() );
}


} // end namespace testing

/*********************************** Black Magic Section ************/

    // Sometimes it just has to be a preprocessor macro <sigh>
    // Or in this case, several macros <roll eyes and sigh>
#define STRINGIFY(x) #x
#define LOCATION_STRING(filename,linenumber)  filename ":" STRINGIFY(linenumber)
#define TEST_LABEL( desc ) ( LOCATION_STRING( __FILE__, __LINE__) ": " desc )

    // The next three macros call the (template) functions above,
    // but automatically insert the file name and line numbers of the test
#define ASSERTION_TEST( cond, desc ) \
                               (testing::assertTrue( cond, TEST_LABEL( desc ) ))

#define EQUALITY_TEST( expected, actual, desc )  \
                 (testing::assertEquals( expected, actual, TEST_LABEL( desc ) ))

#define NOT_EQUALS_TEST( expected, wrong, desc )  \
               (testing::assertNotEquals( expected, wrong, TEST_LABEL( desc ) ))

#define CLOSE_ENOUGH_TEST( expected, actual, tolerance, desc )  \
 (testing::assertCloseEnough( expected, actual, tolerance, TEST_LABEL( desc ) ))
