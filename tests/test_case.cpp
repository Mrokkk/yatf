#include "../include/yatf.hpp"
#include <boost/test/unit_test.hpp>
#include <cstdarg>
#include <string>
#include "common.hpp"

using namespace yatf::detail;

BOOST_AUTO_TEST_SUITE(test_case_suite)

BOOST_FIXTURE_TEST_CASE(assert_works, yatf_fixture) {
    dummy_test_case tc{"suite", "name"};
    test_session::get().current_test_case(&tc);
    REQUIRE_EQ(1, 1);
    BOOST_CHECK_EQUAL(tc.failed, 0);
    BOOST_CHECK_EQUAL(tc.assertions, 1);
    REQUIRE_EQ(1, 0);
    BOOST_CHECK_EQUAL(tc.failed, 1);
    BOOST_CHECK_EQUAL(tc.assertions, 2);
    REQUIRE(true);
    BOOST_CHECK_EQUAL(tc.failed, 1);
    BOOST_CHECK_EQUAL(tc.assertions, 3);
    REQUIRE(false);
    BOOST_CHECK_EQUAL(tc.failed, 2);
    BOOST_CHECK_EQUAL(tc.assertions, 4);
    REQUIRE_FALSE(true);
    BOOST_CHECK_EQUAL(tc.failed, 3);
    BOOST_CHECK_EQUAL(tc.assertions, 5);
    REQUIRE_FALSE(false);
    BOOST_CHECK_EQUAL(tc.failed, 3);
    BOOST_CHECK_EQUAL(tc.assertions, 6);
    REQUIRE_EQ("hello", "world");
    BOOST_CHECK_EQUAL(tc.failed, 4);
    BOOST_CHECK_EQUAL(tc.assertions, 7);
    REQUIRE_EQ("hello", "hello");
    BOOST_CHECK_EQUAL(tc.failed, 4);
    BOOST_CHECK_EQUAL(tc.assertions, 8);
}

BOOST_FIXTURE_TEST_CASE(test_can_pass, yatf_fixture) {
    dummy_test_case tc{"suite_name", "test_name"};
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
    dummy_test_case tc{"suite", "name"};
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
    dummy_test_case tc{"suite", "name"};
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

TEST(suite, name) {
    REQUIRE_EQ(1, 1);
    REQUIRE(false);
    REQUIRE_FALSE(true);
}

BOOST_FIXTURE_TEST_CASE(test_can_call, yatf_fixture) {
    auto &tc = suite_name101;
    test_session::get().current_test_case(&tc);
    tc.test_body();
    auto result = tc.failed;
    BOOST_CHECK_EQUAL(result, 2);
    BOOST_CHECK_EQUAL(tc.assertions, 3);
    BOOST_CHECK_EQUAL(tc.failed, 2);
}

BOOST_AUTO_TEST_SUITE_END()

