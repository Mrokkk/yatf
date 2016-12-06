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

BOOST_AUTO_TEST_CASE(test_can_pass) {
    yatf::detail::test_session::test_case tc{"suite", "name", nullptr};
    yatf::detail::test_session::get().current_test_case(&tc);
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

BOOST_AUTO_TEST_CASE(test_can_fail) {
    yatf::detail::test_session::test_case tc{"suite", "name", nullptr};
    yatf::detail::test_session::get().current_test_case(&tc);
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

BOOST_AUTO_TEST_CASE(test_can_fail_and_pass) {
    yatf::detail::test_session::test_case tc{"suite", "name", nullptr};
    yatf::detail::test_session::get().current_test_case(&tc);
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

BOOST_AUTO_TEST_CASE(test_can_call) {
    yatf::detail::test_session::test_case tc{"suite", "name", sample_test_case};
    yatf::detail::test_session::get().current_test_case(&tc);
    auto result = tc.call();
    BOOST_CHECK_EQUAL(result, 2);
    BOOST_CHECK_EQUAL(tc.assertions, 3);
    BOOST_CHECK_EQUAL(tc.failed, 2);
}

