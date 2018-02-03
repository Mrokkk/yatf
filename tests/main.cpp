#include "../include/yatf.hpp"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yatf_tests
#include <boost/test/unit_test.hpp>
#include <cstdarg>
#include <string>
#include "common.hpp"

char buffer[4096];
int position = 0;

int print(const char *fmt, ...) {
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf((char *)buffer + position, fmt, args);
    position += i;
    va_end(args);
    return i;
}

std::string get_buffer() {
    position = 0;
    return std::string(buffer);
}

void reset_buffer() {
    buffer[0] = 0;
    position = 0;
}

dummy_test_case dummy_tc{"suite", "test"};

