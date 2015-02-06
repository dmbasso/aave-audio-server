//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN test
//#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

#include "test.h"
 
int add(int i, int j)
{
    return i + j;
}
 
BOOST_AUTO_TEST_CASE(AUDIO_INTERFACE_TEST)
{
    //BOOST_CHECK(add(2, 2) == 5);
    
    BOOST_CHECK(set_hrtf(1) == 1);
}
