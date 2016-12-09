#include "../include/yatf.h"
#include <boost/test/unit_test.hpp>
#include <cstdarg>
#include <string>
#include "common.h"

using namespace yatf::detail;

BOOST_AUTO_TEST_SUITE(test_case_suite)

BOOST_FIXTURE_TEST_CASE(assert_works, yatf_fixture) {
    test_session::test_case tc{"suite", "name", nullptr};
    BOOST_CHECK(tc.assert_eq(1, 1));
    BOOST_CHECK(tc.assert_true(true));
    BOOST_CHECK(!tc.assert_eq(1, 0));
    BOOST_CHECK(!tc.assert_true(false));
}

BOOST_FIXTURE_TEST_CASE(test_can_pass, yatf_fixture) {
    test_session::test_case tc{"suite_name", "test_name", nullptr};
    test_session::get().current_test_case(&tc);
    BOOST_CHECK_EQUAL(std::string{tc.suite_name}, "suite_name");
    BOOST_CHECK_EQUAL(std::string{tc.test_name}, "test_name");
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

BOOST_AUTO_TEST_SUITE_END()

