#pragma once

#include "../include/yatf.hpp"
#include <string>
#include <memory>

struct dummy_test_case : public yatf::detail::test_session::test_case {

    explicit dummy_test_case(const char *sn, const char *tn) {
        suite_name = sn;
        test_name = tn;
        yatf::detail::test_session::get().register_test(this);
    }

    void test_body() override {
    }

};

extern dummy_test_case dummy_tc;

std::string get_buffer();
void reset_buffer();

struct yatf_fixture {

    yatf::detail::printer printer;

    yatf_fixture() {
        reset_buffer();
        yatf::detail::test_session::instance_.test_cases_.push_back(dummy_tc);
    }

    ~yatf_fixture() {
        reset_buffer();
    }

    static std::size_t get_assertions() {
        return yatf::detail::test_session::instance_.current_test_case_->assertions_;
    }

    static std::size_t get_failed() {
        return yatf::detail::test_session::instance_.current_test_case_->failed_;
    }

    static const char *get_suite_name() {
        return yatf::detail::test_session::instance_.current_test_case_->suite_name;
    }

    static const char *get_test_name() {
        return yatf::detail::test_session::instance_.current_test_case_->test_name;
    }

};

namespace yatf {
namespace detail {

extern printf_t printf_;

} // namespace detail
} // namespace yatf

int print(const char *fmt, ...);

#if (__cplusplus < 201402L)

namespace std {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace std

#endif

