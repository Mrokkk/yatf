#include "../include/yatf.hpp"
#include <boost/test/unit_test.hpp>
#include <cstdarg>
#include <string>
#include "common.hpp"

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

BOOST_FIXTURE_TEST_CASE(dfsdf, yatf_fixture) {
    mock<int(int, int)> dummy_mock;
    auto handler = dummy_mock.get_handler();
    dummy_mock.register_handler(handler);
    handler.for_arguments(::yatf::_, 4).will_return(33);
    auto res = dummy_mock(2, 5);
    BOOST_CHECK_EQUAL(res, 0);
    res = dummy_mock(2, 4);
    BOOST_CHECK_EQUAL(res, 33);
    res = dummy_mock(3, 4);
    BOOST_CHECK_EQUAL(res, 33);
}

BOOST_AUTO_TEST_SUITE_END()

