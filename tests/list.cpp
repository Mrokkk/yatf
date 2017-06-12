#include "../include/yatf.hpp"
#include <boost/test/unit_test.hpp>
#include <ctime>
#include <vector>
#include <utility>

using namespace yatf::detail;

namespace {

struct helper {
    int a = 0;
    list<helper>::node node_;
    helper(int x) : a(x) {}
};

void init_vectors(std::vector<int> &int_vec, std::vector<helper> &helper_vec, int size) {
    std::srand(std::time(0));
    for (auto i = 0; i < size; i++) {
        int_vec.push_back(std::rand());
    }
    for (auto e : int_vec) {
        helper_vec.push_back(helper{e});
    }
}

void test_adding(list<helper> &head, int s) {
    std::vector<int> v;
    std::vector<helper> helper_vec;
    init_vectors(v, helper_vec, s);
    auto expected_size = 1;
    for (auto &h : helper_vec) {
        auto size = 0;
        head.push_back(h);
        for (const auto &h : head) {
            BOOST_CHECK_EQUAL(v[size], h.a);
            size++;
        };
        BOOST_CHECK_EQUAL(size, expected_size);
        expected_size++;
    }
}

} // namespace

BOOST_AUTO_TEST_SUITE(tests_list_suite)

BOOST_AUTO_TEST_CASE(can_create_empty) {
    helper h(2);
    BOOST_CHECK_EQUAL(h.node_.next(), &h.node_);
    BOOST_CHECK_EQUAL(h.a, 2);
}

BOOST_AUTO_TEST_CASE(can_add_elements) {
    list<helper> head(&helper::node_);
    test_adding(head, 1024);
}

BOOST_AUTO_TEST_CASE(can_use_iterator) {
    list<helper> head(&helper::node_);
    std::vector<helper> helper_vec{0, 2, 4, 9, 30, 109, 938, -231, 3, -29};
    for (auto &v : helper_vec) {
        head.push_back(v);
    }
    for (auto it = std::make_pair(helper_vec.begin(), head.begin()); it.first != helper_vec.end(); ++it.first, ++it.second) {
        BOOST_CHECK_EQUAL(it.first->a, (*it.second).a);
    }
}

BOOST_AUTO_TEST_CASE(can_remove_elements) {
    list<helper> head(&helper::node_);
    helper e1(21), e2(32), e3(-92), e4(1398), e5(0), e6(12438), e7(-2);
    head.push_back(e1);
    head.push_back(e2);
    head.push_back(e3);
    head.push_back(e4);
    head.push_back(e5);
    head.push_back(e6);
    head.push_back(e7);
    head.clear();
    BOOST_CHECK(head.empty());
}

BOOST_AUTO_TEST_SUITE_END()

