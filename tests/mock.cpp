#include "../include/yatf.hpp"
#include <boost/test/unit_test.hpp>
#include <cstdarg>
#include <string>
#include "common.hpp"
#include <memory>

using yatf::_;
using namespace yatf::detail;

BOOST_AUTO_TEST_SUITE(mocks_suite)

BOOST_FIXTURE_TEST_CASE(mock_can_expect_number_of_calls, yatf_fixture) {
    mock<void()> dummy_mock;
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.times(23);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 23);
                BOOST_CHECK_EQUAL(actual, 0);
        });
    }
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.times(92483);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 92483);
                BOOST_CHECK_EQUAL(actual, 0);
        });
    }
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.times(10234);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 10234);
                BOOST_CHECK_EQUAL(actual, 2);
        });
        dummy_mock();
        dummy_mock();
    }
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.times(22);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 22);
                BOOST_CHECK_EQUAL(actual, 22);
        });
        for (auto i = 0; i < 22; ++i) {
            dummy_mock();
        }
    }
}

BOOST_FIXTURE_TEST_CASE(mock_without_handler_will_return_def_vals, yatf_fixture) {
    mock<int()> dummy_mock;
    for (auto i = 0; i < 1024; ++i ) {
        auto result = dummy_mock();
        BOOST_CHECK_EQUAL(result, int());
    }
}

BOOST_FIXTURE_TEST_CASE(mock_without_retval_can_be_called, yatf_fixture) {
    mock<void()> dummy_mock;
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 0);
        });
    }
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 1);
        });
        dummy_mock();
    }
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 1024);
        });
        for (auto i = 0; i < 1024; ++i) {
            dummy_mock();
        }
    }
}

BOOST_FIXTURE_TEST_CASE(mock_with_retval_can_be_called, yatf_fixture) {
    mock<int()> dummy_mock;
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 1);
        });
        auto result = dummy_mock();
        BOOST_CHECK_EQUAL(result, int());
    }
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.will_return(934);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 1);
        });
        auto result = dummy_mock();
        BOOST_CHECK_EQUAL(result, 934);
    }
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.will_return(-3198);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 1024);
        });
        for (auto i = 0; i < 1024; ++i) {
            auto result = dummy_mock();
            BOOST_CHECK_EQUAL(result, -3198);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(mock_can_match_arguments, yatf_fixture) {
    mock<int(int, char)> dummy_mock;
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 1);
        });
        handler.match_args([](int a, char c) {
            return a == 9932 && c == 'd';
        }).will_return(9438);
        auto result = dummy_mock(34, 'c');
        BOOST_CHECK_EQUAL(result, int());
        result = dummy_mock(9932, 'd');
        BOOST_CHECK_EQUAL(result, 9438);
    }
    mock<int()> dummy_mock2;
    {
        auto handler = dummy_mock2.get_handler();
        dummy_mock2.register_handler(handler);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 1);
        });
        handler.match_args([]() {
            return true;
        }).will_return(9438);
        auto result = dummy_mock2();
        BOOST_CHECK_EQUAL(result, 9438);
    }
    mock<int()> dummy_mock3;
    {
        auto handler = dummy_mock3.get_handler();
        dummy_mock3.register_handler(handler);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 0);
        });
        handler.match_args([]() {
            return false;
        }).will_return(9438);
        auto result = dummy_mock3();
        BOOST_CHECK_EQUAL(result, int());
    }
    mock<void()> dummy_mock4;
    {
        auto handler = dummy_mock4.get_handler();
        dummy_mock4.register_handler(handler);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 1);
        });
        handler.match_args([]() {
            return true;
        });
        dummy_mock4();
    }
    mock<void()> dummy_mock5;
    {
        auto handler = dummy_mock5.get_handler();
        dummy_mock5.register_handler(handler);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 0);
        });
        handler.match_args([]() {
            return false;
        });
        dummy_mock5();
    }
}

BOOST_FIXTURE_TEST_CASE(can_match_single_argument, yatf_fixture) {
    mock<void(int)> dummy_mock;
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.for_arguments(_);
        dummy_mock(2);
        dummy_mock(0);
        dummy_mock(-932);
        dummy_mock(8);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 4);
        });
    }
    {
        auto handler = dummy_mock.get_handler();
        dummy_mock.register_handler(handler);
        handler.for_arguments(924);
        dummy_mock(924);
        dummy_mock(2);
        handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
                BOOST_CHECK_EQUAL(expected, 1);
                BOOST_CHECK_EQUAL(actual, 1);
        });
    }
}

struct helper {

    int a, b;

    helper(int a, int b) : a(a), b(b) {
    }

};

bool operator==(const helper &lhs, const helper &rhs) {
    return lhs.a == rhs.a && lhs.b == rhs.b;
}

BOOST_FIXTURE_TEST_CASE(can_match_multiple_arguments, yatf_fixture) {
    mock<int(int, helper)> dummy_mock;
    auto handler = dummy_mock.get_handler();
    dummy_mock.register_handler(handler);
    handler.for_arguments(_, helper(2, 1)).will_return(33);
    auto res = dummy_mock(2, helper(2, 1));
    BOOST_CHECK_EQUAL(res, 33);
    res = dummy_mock(2, helper(2, 2));
    BOOST_CHECK_EQUAL(res, 0);
    handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
            BOOST_CHECK_EQUAL(expected, 1);
            BOOST_CHECK_EQUAL(actual, 1);
    });
}

struct my_matcher {

    static bool match(const int &rhs) {
        return rhs == 2;
    }

};

BOOST_FIXTURE_TEST_CASE(can_pass_matcher, yatf_fixture) {
    mock<int(int, helper)> dummy_mock;
    auto handler = dummy_mock.get_handler();
    dummy_mock.register_handler(handler);
    handler.for_arguments(my_matcher(), helper(2, 1)).will_return(33);
    auto res = dummy_mock(2, helper(2, 1));
    BOOST_CHECK_EQUAL(res, 33);
    res = dummy_mock(1, helper(2, 1));
    BOOST_CHECK_EQUAL(res, 0);
    res = dummy_mock(2, helper(2, 2));
    BOOST_CHECK_EQUAL(res, 0);
    handler.schedule_assertion([](std::size_t expected, std::size_t actual) {
            BOOST_CHECK_EQUAL(expected, 1);
            BOOST_CHECK_EQUAL(actual, 1);
    });
}

BOOST_AUTO_TEST_SUITE_END()

