#include "../include/yatf.hpp"
#include <boost/test/unit_test.hpp>
#include <ctime>
#include <utility>
#include <vector>
#include <memory>
#include "common.hpp"

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
    REQUIRE_EQ(0, 0);
}

void failing_test_case() {
    REQUIRE_EQ(1, 1);
    REQUIRE(true);
    REQUIRE(false);
    REQUIRE_FALSE(true);
    REQUIRE_EQ(1, 0);
}

int stubbed_printf(const char *, ...) {
    return 0;
}

} // namespace anon

BOOST_FIXTURE_TEST_CASE(can_run_tests, yatf_fixture) {
    srand(time(NULL));
    std::vector<std::unique_ptr<test_session::test_case>> tests;
    std::vector<yatf::config> configs{
            yatf::config{false, false, false},
            yatf::config{true, false, false},
            yatf::config{true, true, false},
            yatf::config{false, true, false},
            yatf::config{false, true, true},
            yatf::config{false, false, true},
            yatf::config{true, false, true},
            yatf::config{true, true, true}
    };
    auto failed = 0u;
    _printf = stubbed_printf;
    for (auto c = configs.begin(); c != configs.end(); ++c) {
        for (auto i = 0u; i < 512; ++i) {
            bool pass = std::rand() % 2 == 1;
            if (pass) tests.push_back(std::make_unique<test_session::test_case>("t", "t", passing_test_case));
            else {
                failed++;
                tests.push_back(std::make_unique<test_session::test_case>("t", "t", failing_test_case));
            }
            BOOST_CHECK_EQUAL(failed, test_session::get().run(*c));
        }
    }
    _printf = print;
}

BOOST_FIXTURE_TEST_CASE(can_run_one_test, yatf_fixture) {
    srand(time(NULL));
    std::vector<std::unique_ptr<test_session::test_case>> tests;
    std::vector<yatf::config> configs{
            yatf::config{false, false, false},
            yatf::config{true, false, false},
            yatf::config{true, true, false},
            yatf::config{false, true, false},
            yatf::config{false, true, true},
            yatf::config{false, false, true},
            yatf::config{true, false, true},
            yatf::config{true, true, true}
    };
    _printf = stubbed_printf;
    for (auto c = configs.begin(); c != ++configs.begin(); ++c) {
        tests.push_back(std::make_unique<test_session::test_case>("t", "t1", passing_test_case));
        BOOST_CHECK_EQUAL(0, test_session::get().run(*c, "t.t1"));
        tests.push_back(std::make_unique<test_session::test_case>("t", "t2", failing_test_case));
        BOOST_CHECK_EQUAL(3, test_session::get().run(*c, "t.t2"));
        BOOST_CHECK(test_session::get().run(*c, "t.t3") < 0);
        BOOST_CHECK(test_session::get().run(*c, "some_bad_name") < 0);
    }
    _printf = print;
}

BOOST_AUTO_TEST_SUITE_END()

