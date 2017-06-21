#include "../include/yatf.hpp"
#include <boost/test/unit_test.hpp>
#include <cstdarg>
#include <string>
#include "common.hpp"

using namespace yatf::detail;

BOOST_AUTO_TEST_SUITE(unary_container_suite)

struct helper {
    int a;
    bool operator==(const helper &v) const {
        return a == v.a;
    }
};

BOOST_FIXTURE_TEST_CASE(can_be_created_empty, yatf_fixture) {
    unary_container<int> container;
    BOOST_CHECK(!container);
    BOOST_CHECK(!(container == 0));
}

BOOST_FIXTURE_TEST_CASE(can_be_initialized, yatf_fixture) {
    unary_container<int> container(2);
    BOOST_CHECK(container);
    BOOST_CHECK(container == 2);
    BOOST_CHECK(!(container == 4));
}

BOOST_FIXTURE_TEST_CASE(can_be_set, yatf_fixture) {
    unary_container<int> container;
    container.set(43);
    BOOST_CHECK(container);
    BOOST_CHECK(container == 43);
    BOOST_CHECK(!(container == 4));
    BOOST_CHECK_EQUAL(container.get(), 43);
    container.set(-842);
    BOOST_CHECK(container);
    BOOST_CHECK(container == -842);
    BOOST_CHECK(!(container == 43));
    BOOST_CHECK_EQUAL(container.get(), -842);
}

BOOST_FIXTURE_TEST_CASE(works_with_structs, yatf_fixture) {
    do {
        unary_container<helper> container;
        BOOST_CHECK(!container);
        BOOST_CHECK(!(container == helper{43}));
        BOOST_CHECK(!(container == helper()));
        BOOST_CHECK_EQUAL(container->a, 0);
        container.set(helper{-999});
        BOOST_CHECK(container);
        BOOST_CHECK(!(container == helper{43}));
        BOOST_CHECK(container == helper{-999});
        BOOST_CHECK_EQUAL(container->a, -999);
    } while (0);
    do {
        unary_container<helper> container(helper{});
        BOOST_CHECK(container);
        BOOST_CHECK(!(container == helper{43}));
        BOOST_CHECK(container == helper());
        BOOST_CHECK_EQUAL(container->a, 0);
        container.set(helper{-999});
        BOOST_CHECK(container);
        BOOST_CHECK(!(container == helper{43}));
        BOOST_CHECK(container == helper{-999});
        BOOST_CHECK_EQUAL(container->a, -999);
    } while (0);
}

BOOST_AUTO_TEST_SUITE_END()

