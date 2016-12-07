#include "../include/yatf.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yatf_tests
#include <boost/test/unit_test.hpp>
#include <cstdarg>
#include <string>

char buffer[1024];
int position = 0;

int print(const char *fmt, ...) {
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf((char *)buffer + position, fmt, args);
    position += i;
    va_end(args);
    return i;
}

std::string get_buffer() {
    position = 0;
    return std::string(buffer);
}

namespace yatf {
namespace detail {

test_session test_session::_instance;
printf_t _printf = print;

} // namespace detail
} // namespace yatf

using namespace yatf::detail;

test_session::test_case dummy_tc{"suite", "test", nullptr};

struct yatf_fixture {
    yatf_fixture() { position = 0; test_session::_instance._test_cases = dummy_tc; }
    ~yatf_fixture() { position = 0; }
};

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

BOOST_FIXTURE_TEST_CASE(test_can_pass, yatf_fixture) {
    test_session::test_case tc{"suite", "name", nullptr};
    test_session::get().current_test_case(&tc);
    BOOST_CHECK_EQUAL(tc.assertions, 0);
    BOOST_CHECK_EQUAL(tc.failed, 0);
    BOOST_CHECK_EQUAL(get_buffer(), "");
    REQUIRE_EQ(0, 0);
    BOOST_CHECK_EQUAL(tc.assertions, 1);
    BOOST_CHECK_EQUAL(tc.failed, 0);
    BOOST_CHECK_EQUAL(get_buffer(), "");
    REQUIRE(true);
    BOOST_CHECK_EQUAL(tc.assertions, 2);
    BOOST_CHECK_EQUAL(tc.failed, 0);
    BOOST_CHECK_EQUAL(get_buffer(), "");
    REQUIRE_FALSE(false);
    BOOST_CHECK_EQUAL(tc.assertions, 3);
    BOOST_CHECK_EQUAL(tc.failed, 0);
    BOOST_CHECK_EQUAL(get_buffer(), "");
}

BOOST_FIXTURE_TEST_CASE(test_can_fail, yatf_fixture) {
    test_session::test_case tc{"suite", "name", nullptr};
    test_session::get().current_test_case(&tc);
    BOOST_CHECK_EQUAL(tc.assertions, 0);
    BOOST_CHECK_EQUAL(tc.failed, 0);
    BOOST_CHECK_EQUAL(get_buffer(), "");
    REQUIRE_EQ(0, 1);
    BOOST_CHECK_EQUAL(tc.assertions, 1);
    BOOST_CHECK_EQUAL(tc.failed, 1);
    std::string comp("assertion failed: " + std::string(__FILE__) + ":" + std::to_string(__LINE__ - 3) + " \'0\' isn't \'1\': 0 != 1\n");
    BOOST_CHECK_EQUAL(get_buffer(), comp);
    REQUIRE(false);
    BOOST_CHECK_EQUAL(tc.assertions, 2);
    BOOST_CHECK_EQUAL(tc.failed, 2);
    comp = std::string("assertion failed: " + std::string(__FILE__) + ":" + std::to_string(__LINE__ - 3) + " \'false\' is false\n");
    BOOST_CHECK_EQUAL(get_buffer(), comp);
    REQUIRE_FALSE(true);
    BOOST_CHECK_EQUAL(tc.assertions, 3);
    BOOST_CHECK_EQUAL(tc.failed, 3);
    comp = std::string("assertion failed: " + std::string(__FILE__) + ":" + std::to_string(__LINE__ - 3) + " \'true\' is true\n");
    BOOST_CHECK_EQUAL(get_buffer(), comp);
}

BOOST_FIXTURE_TEST_CASE(test_can_fail_and_pass, yatf_fixture) {
    test_session::test_case tc{"suite", "name", nullptr};
    test_session::get().current_test_case(&tc);
    BOOST_CHECK_EQUAL(tc.assertions, 0);
    BOOST_CHECK_EQUAL(tc.failed, 0);
    REQUIRE_EQ(0, 0);
    BOOST_CHECK_EQUAL(tc.assertions, 1);
    BOOST_CHECK_EQUAL(tc.failed, 0);
    REQUIRE(false);
    BOOST_CHECK_EQUAL(tc.assertions, 2);
    BOOST_CHECK_EQUAL(tc.failed, 1);
    REQUIRE_FALSE(false);
    BOOST_CHECK_EQUAL(tc.assertions, 3);
    BOOST_CHECK_EQUAL(tc.failed, 1);
}

void sample_test_case() {
    REQUIRE_EQ(1, 1);
    REQUIRE(false);
    REQUIRE_FALSE(true);
}

BOOST_FIXTURE_TEST_CASE(test_can_call, yatf_fixture) {
    test_session::test_case tc{"suite", "name", sample_test_case};
    test_session::get().current_test_case(&tc);
    auto result = tc.call();
    BOOST_CHECK_EQUAL(result, 2);
    BOOST_CHECK_EQUAL(tc.assertions, 3);
    BOOST_CHECK_EQUAL(tc.failed, 2);
}

