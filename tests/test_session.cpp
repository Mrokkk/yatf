#include "../include/yatf.h"
#include <boost/test/unit_test.hpp>
#include <ctime>
#include <vector>

using namespace yatf::detail;

BOOST_AUTO_TEST_SUITE(test_session_suite)

BOOST_AUTO_TEST_CASE(can_return_message) {
    std::string result = test_session::messages::get(test_session::messages::msg::start_end);
    BOOST_CHECK_EQUAL(result, "[========]");
    result = test_session::messages::get(test_session::messages::msg::run);
    BOOST_CHECK_EQUAL(result, "[  RUN   ]");
    result = test_session::messages::get(test_session::messages::msg::pass);
    BOOST_CHECK_EQUAL(result, "[  PASS  ]");
    result = test_session::messages::get(test_session::messages::msg::fail);
    BOOST_CHECK_EQUAL(result, "[  FAIL  ]");
}

BOOST_AUTO_TEST_SUITE_END()

