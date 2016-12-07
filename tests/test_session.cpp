#include "../include/yatf.h"
#include <boost/test/unit_test.hpp>
#include <ctime>
#include <utility>
#include <vector>
#include <memory>
#include "common.h"

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

namespace {

void passing_test_case() {
    REQUIRE_EQ(1, 1);
    REQUIRE(true);
    REQUIRE_FALSE(false);
}

void failing_test_case() {
    REQUIRE_EQ(1, 1);
    REQUIRE(true);
    REQUIRE_FALSE(true);
}

int stubbed_printf(const char *, ...) {
    return 0;
}

} // namespace anon

BOOST_FIXTURE_TEST_CASE(can_run_tests, yatf_fixture) {
    srand ( time(NULL) );
    std::vector<std::unique_ptr<test_session::test_case>> tests;
    yatf::config c{false, false, false};
    auto failed = 0;
    _printf = stubbed_printf;
    for (int i = 0; i < 1024; ++i) {
        bool pass = std::rand() % 2 == 1;
        if (pass) tests.push_back(std::make_unique<test_session::test_case>("t", "t", passing_test_case));
        else {
            failed++;
            tests.push_back(std::make_unique<test_session::test_case>("t", "t", failing_test_case));
        }
        BOOST_CHECK_EQUAL(failed, test_session::get().run(c));
    }
    _printf = print;
}

BOOST_AUTO_TEST_SUITE_END()

