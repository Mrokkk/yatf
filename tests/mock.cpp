#include <ctime>
#include <limits>
#include <cstdlib>
#include <boost/test/unit_test.hpp>

#include "common.hpp"
#include "../include/yatf.hpp"

using yatf::_;
using yatf::field;
using namespace yatf::detail;

#define GET_HANDLER(mock, handler_name) \
    auto handler_name = mock.get_handler(); \
    dummy_mock.register_handler(handler_name);

struct helper {

    int a = 0;

    helper() = default;

    helper(int a) : a(a) {
    }

    bool operator==(const helper &h) {
        return h.a == a;
    }

};

BOOST_AUTO_TEST_SUITE(mocks_suite)

template <typename Signature>
void test_handler_creating() {
    mock<Signature> dummy_mock;
    GET_HANDLER(dummy_mock, handler);
}

BOOST_FIXTURE_TEST_CASE(can_create_handler, yatf_fixture) {
    test_handler_creating<void()>();
    test_handler_creating<void(int)>();
    test_handler_creating<void(int, int)>();
    test_handler_creating<void(helper)>();
    test_handler_creating<int()>();
    test_handler_creating<int(int)>();
    test_handler_creating<int(int, int)>();
    test_handler_creating<int(helper)>();
}

template <typename Signature>
void test_expecting_calls() {
    mock<Signature> dummy_mock;
    do {
        GET_HANDLER(dummy_mock, handler);
        handler.times(0);
        handler.schedule_assertion([](std::size_t expected, std::size_t) {
            BOOST_CHECK_EQUAL(expected, 0);
        });
    } while (0);
    do {
        GET_HANDLER(dummy_mock, handler);
        handler.times(1);
        handler.schedule_assertion([](std::size_t expected, std::size_t) {
            BOOST_CHECK_EQUAL(expected, 1);
        });
    } while (0);
    do {
        GET_HANDLER(dummy_mock, handler);
        handler.times(924);
        handler.schedule_assertion([](std::size_t expected, std::size_t) {
            BOOST_CHECK_EQUAL(expected, 924);
        });
    } while (0);
}

BOOST_FIXTURE_TEST_CASE(can_expect_number_of_calls, yatf_fixture) {
    test_expecting_calls<void()>();
    test_expecting_calls<void(int)>();
    test_expecting_calls<void(int, int)>();
    test_expecting_calls<void(helper)>();
    test_expecting_calls<int()>();
    test_expecting_calls<int(int)>();
    test_expecting_calls<int(int, int)>();
    test_expecting_calls<int(helper)>();
}

template <typename R, typename ...Args>
void test_calling() {
    mock<R(Args...)> dummy_mock;
    do {
        GET_HANDLER(dummy_mock, handler);
        handler.schedule_assertion([](std::size_t, std::size_t actual) {
            BOOST_CHECK_EQUAL(actual, 0);
        });
    } while (0);
    do {
        GET_HANDLER(dummy_mock, handler);
        handler.schedule_assertion([](std::size_t, std::size_t actual) {
            BOOST_CHECK_EQUAL(actual, 1);
        });
        dummy_mock(Args()...);
    } while (0);
    do {
        GET_HANDLER(dummy_mock, handler);
        handler.schedule_assertion([](std::size_t, std::size_t actual) {
            BOOST_CHECK_EQUAL(actual, 256);
        });
        for (int i = 0; i < 256; ++i) {
            dummy_mock(Args()...);
        }
    } while (0);
}

BOOST_FIXTURE_TEST_CASE(can_count_actual_nr_of_calls, yatf_fixture) {
    test_calling<void>();
    test_calling<void, int>();
    test_calling<void, int, int>();
    test_calling<void, helper>();
    test_calling<int>();
    test_calling<int, int>();
    test_calling<int, int, int>();
    test_calling<int, helper>();
}

template <typename R, typename ...Args>
void test_return_value() {
    mock<R(Args...)> dummy_mock;
    for (auto i = 0; i < 256; ++i ) {
        GET_HANDLER(dummy_mock, handler);
        auto return_value = std::rand() % std::numeric_limits<R>::max();
        handler.will_return(return_value);
        auto result = dummy_mock(Args()...);
        BOOST_CHECK_EQUAL(result, return_value);
    }
}

BOOST_FIXTURE_TEST_CASE(can_specify_return_value, yatf_fixture) {
    std::srand(std::time(0));
    test_return_value<int>();
    test_return_value<int, int>();
    test_return_value<int, int, int>();
    test_return_value<int, helper>();
}

template <typename R, typename ...Args>
void test_default_return_value() {
    mock<R(Args...)> dummy_mock;
    for (auto i = 0; i < 256; ++i ) {
        auto result = dummy_mock(Args()...);
        BOOST_CHECK_EQUAL(result, R());
    }
}

BOOST_FIXTURE_TEST_CASE(mock_will_return_default_value_when_no_handler, yatf_fixture) {
    test_default_return_value<int>();
    test_default_return_value<int, int>();
    test_default_return_value<int, int, int>();
    test_default_return_value<int, helper>();
}

BOOST_FIXTURE_TEST_CASE(dummy_test_for_arguments, yatf_fixture) {
    ::yatf::detail::arguments<> a;
    BOOST_CHECK(a.compare());
}

template <typename R>
void test_matching_by_lambda() {
    do {
        mock<R()> dummy_mock;
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 2);
            });
            handler.match_args([]() {
                return true;
            });
            dummy_mock();
            dummy_mock();
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 0);
            });
            handler.match_args([]() {
                return false;
            });
            dummy_mock();
            dummy_mock();
        } while (0);
    } while (0);
    do {
        mock<R(int)> dummy_mock;
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 1);
            });
            handler.match_args([](int a) {
                return a == 32;
            });
            dummy_mock(2);
            dummy_mock(32);
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 32);
            });
            handler.match_args([](int a) {
                return a == 19432;
            });
            for (auto i = 0; i < 32; ++i) {
                dummy_mock(19432);
            }
        } while (0);
    } while (0);
    do {
        mock<R(int, int)> dummy_mock;
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 1);
            });
            handler.match_args([](int a, int b) {
                return a == 32 && b == 3;
            });
            dummy_mock(2, 3);
            dummy_mock(32, 3);
            dummy_mock(32, 0);
        } while (0);
    } while (0);
    do {
        mock<R(helper)> dummy_mock;
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 2);
            });
            handler.match_args([](helper h) {
                return h.a == 32;
            });
            dummy_mock(helper(23));
            dummy_mock(helper(32));
            dummy_mock(helper(32));
        } while (0);
    } while (0);
}

BOOST_FIXTURE_TEST_CASE(can_match_arguments_by_lambda, yatf_fixture) {
    test_matching_by_lambda<void>();
    test_matching_by_lambda<int>();
}

template <typename R>
void test_matching_directly() {
    do {
        mock<R()> dummy_mock;
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 2);
            });
            handler.for_arguments();
            dummy_mock();
            dummy_mock();
        } while (0);
    } while (0);
    do {
        mock<R(int)> dummy_mock;
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 1);
            });
            handler.for_arguments(32);
            dummy_mock(2);
            dummy_mock(32);
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 32);
            });
            handler.for_arguments(19432);
            for (auto i = 0; i < 32; ++i) {
                dummy_mock(19432);
            }
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 32);
            });
            handler.for_arguments(_);
            for (auto i = 0; i < 32; ++i) {
                dummy_mock(i);
            }
        } while (0);
    } while (0);
    do {
        mock<R(int, int)> dummy_mock;
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 1);
            });
            handler.for_arguments(32, 3);
            dummy_mock(2, 3);
            dummy_mock(32, 3);
            dummy_mock(32, 0);
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 32);
            });
            handler.for_arguments(_, _);
            for (auto i = 0; i < 32; ++i) {
                dummy_mock(i, i + 1024);
            }
        } while (0);
    } while (0);
    do {
        mock<R(helper)> dummy_mock;
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 1);
            });
            handler.for_arguments(helper(32));
            dummy_mock(helper(2));
            dummy_mock(helper(31));
            dummy_mock(helper(32));
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            handler.schedule_assertion([](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 32);
            });
            handler.for_arguments(_);
            for (auto i = 0; i < 32; ++i) {
                dummy_mock(helper(i));
            }
        } while (0);
    } while (0);
}

BOOST_FIXTURE_TEST_CASE(can_match_arguments_directly, yatf_fixture) {
    test_matching_directly<void>();
    test_matching_directly<int>();
}

BOOST_FIXTURE_TEST_CASE(can_match_arguments_and_set_return_values, yatf_fixture) {
    mock<int(int)> dummy_mock;
    do {
        GET_HANDLER(dummy_mock, handler1);
        GET_HANDLER(dummy_mock, handler2);
        GET_HANDLER(dummy_mock, handler3);
        auto assertion = [](std::size_t, std::size_t actual) {
            BOOST_CHECK_EQUAL(actual, 1);
        };
        handler1.schedule_assertion(assertion);
        handler2.schedule_assertion(assertion);
        handler3.schedule_assertion(assertion);
        handler1.for_arguments(932).will_return(99);
        handler2.for_arguments(1).will_return(-9423);
        handler3.for_arguments(-44).will_return(0);
        BOOST_CHECK_EQUAL(dummy_mock(1), -9423);
        BOOST_CHECK_EQUAL(dummy_mock(-44), 0);
        BOOST_CHECK_EQUAL(dummy_mock(932), 99);
    } while (0);
}

BOOST_FIXTURE_TEST_CASE(can_match_arguments_by_matchers, yatf_fixture) {
    mock<int(int)> dummy_mock;
    do {
        do {
            GET_HANDLER(dummy_mock, handler);
            auto assertion = [](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 1);
            };
            handler.schedule_assertion(assertion);
            handler.for_arguments(::yatf::eq(3)).will_return(2);
            BOOST_CHECK_EQUAL(dummy_mock(194), int());
            BOOST_CHECK_EQUAL(dummy_mock(3), 2);
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            auto assertion = [](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 4);
            };
            handler.schedule_assertion(assertion);
            handler.for_arguments(::yatf::ne(3)).will_return(2);
            BOOST_CHECK_EQUAL(dummy_mock(0), 2);
            BOOST_CHECK_EQUAL(dummy_mock(1), 2);
            BOOST_CHECK_EQUAL(dummy_mock(2), 2);
            BOOST_CHECK_EQUAL(dummy_mock(3), int());
            BOOST_CHECK_EQUAL(dummy_mock(4), 2);
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            auto assertion = [](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 223);
            };
            handler.schedule_assertion(assertion);
            handler.for_arguments(::yatf::gt(32)).will_return(2);
            for (auto i = 0; i <= 32; ++i) {
                BOOST_CHECK_EQUAL(dummy_mock(i), int());
            }
            for (auto i = 33; i < 256; ++i) {
                BOOST_CHECK_EQUAL(dummy_mock(i), 2);
            }
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            auto assertion = [](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 224);
            };
            handler.schedule_assertion(assertion);
            handler.for_arguments(::yatf::ge(32)).will_return(2);
            for (auto i = 0; i < 32; ++i) {
                BOOST_CHECK_EQUAL(dummy_mock(i), int());
            }
            for (auto i = 32; i < 256; ++i) {
                BOOST_CHECK_EQUAL(dummy_mock(i), 2);
            }
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            auto assertion = [](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 32);
            };
            handler.schedule_assertion(assertion);
            handler.for_arguments(::yatf::lt(32)).will_return(2);
            for (auto i = 0; i < 32; ++i) {
                BOOST_CHECK_EQUAL(dummy_mock(i), 2);
            }
            for (auto i = 32; i < 256; ++i) {
                BOOST_CHECK_EQUAL(dummy_mock(i), int());
            }
        } while (0);
        do {
            GET_HANDLER(dummy_mock, handler);
            auto assertion = [](std::size_t, std::size_t actual) {
                BOOST_CHECK_EQUAL(actual, 33);
            };
            handler.schedule_assertion(assertion);
            handler.for_arguments(::yatf::le(32)).will_return(2);
            for (auto i = 0; i <= 32; ++i) {
                BOOST_CHECK_EQUAL(dummy_mock(i), 2);
            }
            for (auto i = 33; i < 256; ++i) {
                BOOST_CHECK_EQUAL(dummy_mock(i), int());
            }
        } while (0);
    } while (0);
}

BOOST_FIXTURE_TEST_CASE(can_match_arguments_by_field, yatf_fixture) {
    do {
        mock<void(helper)> dummy_mock;
        GET_HANDLER(dummy_mock, handler);
        auto assertion = [](std::size_t, std::size_t actual) {
            BOOST_CHECK_EQUAL(actual, 2);
        };
        handler.schedule_assertion(assertion);
        handler.for_arguments(field(&helper::a, 439));
        dummy_mock(helper(3));
        dummy_mock(helper(0));
        dummy_mock(helper(440));
        dummy_mock(helper(438));
        dummy_mock(helper(439));
        dummy_mock(helper(439));
    } while (0);
    do {
        mock<int(helper)> dummy_mock;
        GET_HANDLER(dummy_mock, handler);
        auto assertion = [](std::size_t, std::size_t actual) {
            BOOST_CHECK_EQUAL(actual, 1);
        };
        handler.schedule_assertion(assertion);
        handler.for_arguments(field(&helper::a, 439)).will_return(999);
        BOOST_CHECK_EQUAL(dummy_mock(helper(3)), int());
        BOOST_CHECK_EQUAL(dummy_mock(helper(0)), int());
        BOOST_CHECK_EQUAL(dummy_mock(helper(440)), int());
        BOOST_CHECK_EQUAL(dummy_mock(helper(438)), int());
        BOOST_CHECK_EQUAL(dummy_mock(helper(439)), 999);
    } while (0);
}

BOOST_AUTO_TEST_SUITE_END()

