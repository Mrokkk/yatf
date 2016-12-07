#pragma once

#include "../include/yatf.h"
#include <string>

extern yatf::detail::test_session::test_case dummy_tc;

std::string get_buffer();
void reset_buffer();

struct yatf_fixture {
    yatf_fixture() { reset_buffer(); yatf::detail::test_session::_instance._test_cases = dummy_tc; }
    ~yatf_fixture() { reset_buffer(); }
};

namespace yatf {
namespace detail {

extern printf_t _printf;

} // namespace detail
} // namespace yatf

int print(const char *fmt, ...);

