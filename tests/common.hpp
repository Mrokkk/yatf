#pragma once

#include "../include/yatf.hpp"
#include <string>
#include <memory>

extern yatf::detail::test_session::test_case dummy_tc;

std::string get_buffer();
void reset_buffer();

struct yatf_fixture {
    yatf_fixture() { reset_buffer(); yatf::detail::test_session::instance_.test_cases_ = dummy_tc; }
    ~yatf_fixture() { reset_buffer(); }
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

