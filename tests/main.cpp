#include "../include/yatf.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yatf_tests
#include <boost/test/unit_test.hpp>
#include <cstdarg>

char buffer[1024];

int print(const char *fmt, ...) {
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf(buffer, fmt, args);
    va_end(args);
    return i;
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
    BOOST_CHECK(tc.assertions == 0);
    BOOST_CHECK(tc.failed == 0);
    REQUIRE_EQ(0, 0);
    BOOST_CHECK(tc.assertions == 1);
    BOOST_CHECK(tc.failed == 0);
    REQUIRE(true);
    BOOST_CHECK(tc.assertions == 2);
    BOOST_CHECK(tc.failed == 0);
    REQUIRE_FALSE(false);
    BOOST_CHECK(tc.assertions == 3);
    BOOST_CHECK(tc.failed == 0);
}

BOOST_AUTO_TEST_CASE(test_can_fail) {
    yatf::detail::test_session::test_case tc{"suite", "name", nullptr};
    yatf::detail::test_session::get().current_test_case(&tc);
    BOOST_CHECK(tc.assertions == 0);
    BOOST_CHECK(tc.failed == 0);
    REQUIRE_EQ(0, 1);
    BOOST_CHECK(tc.assertions == 1);
    BOOST_CHECK(tc.failed == 1);
    REQUIRE(false);
    BOOST_CHECK(tc.assertions == 2);
    BOOST_CHECK(tc.failed == 2);
    REQUIRE_FALSE(true);
    BOOST_CHECK(tc.assertions == 3);
    BOOST_CHECK(tc.failed == 3);
}

BOOST_AUTO_TEST_CASE(test_can_fail_and_pass) {
    yatf::detail::test_session::test_case tc{"suite", "name", nullptr};
    yatf::detail::test_session::get().current_test_case(&tc);
    BOOST_CHECK(tc.assertions == 0);
    BOOST_CHECK(tc.failed == 0);
    REQUIRE_EQ(0, 0);
    BOOST_CHECK(tc.assertions == 1);
    BOOST_CHECK(tc.failed == 0);
    REQUIRE(false);
    BOOST_CHECK(tc.assertions == 2);
    BOOST_CHECK(tc.failed == 1);
    REQUIRE_FALSE(false);
    BOOST_CHECK(tc.assertions == 3);
    BOOST_CHECK(tc.failed == 1);
}

