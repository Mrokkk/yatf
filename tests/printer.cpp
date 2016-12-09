#include "../include/yatf.h"
#include <boost/test/unit_test.hpp>
#include <string>
#include <iomanip>
#include <sstream>
#include "common.h"

using namespace yatf::detail;

BOOST_AUTO_TEST_SUITE(printer_suite)

BOOST_FIXTURE_TEST_CASE(can_print_char, yatf_fixture) {
    for (char i = 1; i < 127; i++) {
        printer::print(i);
        BOOST_CHECK_EQUAL(get_buffer(), std::string(1, i));
    }
}

BOOST_FIXTURE_TEST_CASE(can_print_unsigned_char, yatf_fixture) {
    for (unsigned char i = 0; i < 255; i++) {
        printer::print(i);
        BOOST_CHECK_EQUAL(get_buffer(), std::to_string(i));
    }
}

BOOST_FIXTURE_TEST_CASE(can_print_short, yatf_fixture) {
    for (short i = -1024; i < 1025; i++) {
        printer::print(i);
        BOOST_CHECK_EQUAL(get_buffer(), std::to_string(i));
    }
}

BOOST_FIXTURE_TEST_CASE(can_print_unsigned_short, yatf_fixture) {
    for (unsigned short i = 0; i < 1025; i++) {
        printer::print(i);
        BOOST_CHECK_EQUAL(get_buffer(), std::to_string(i));
    }
}

BOOST_FIXTURE_TEST_CASE(can_print_int, yatf_fixture) {
    for (int i = -1024; i < 1025; i++) {
        printer::print(i);
        BOOST_CHECK_EQUAL(get_buffer(), std::to_string(i));
    }
}

BOOST_FIXTURE_TEST_CASE(can_print_unsigned_int, yatf_fixture) {
    for (unsigned int i = 0; i < 1025; i++) {
        printer::print(i);
        BOOST_CHECK_EQUAL(get_buffer(), std::to_string(i));
    }
}

BOOST_FIXTURE_TEST_CASE(can_print_address, yatf_fixture) {
    for (unsigned int i = 0; i < 1025; i++) {
        printer::print(reinterpret_cast<void *>(i));
        std::stringstream out;
        out << std::hex << i;
        BOOST_CHECK_EQUAL(get_buffer(), "0x" + out.str());
    }
}

BOOST_FIXTURE_TEST_CASE(can_print_nullptr, yatf_fixture) {
    printer::print(nullptr);
    BOOST_CHECK_EQUAL(get_buffer(), "NULL");
}

BOOST_FIXTURE_TEST_CASE(can_print_colors, yatf_fixture) {
    printer::print(printer::color::red);
    BOOST_CHECK_EQUAL(get_buffer(), "\e[31m");
    printer::print(printer::color::green);
    BOOST_CHECK_EQUAL(get_buffer(), "\e[32m");
    printer::print(printer::color::reset);
    BOOST_CHECK_EQUAL(get_buffer(), "\e[0m");
}

BOOST_FIXTURE_TEST_CASE(can_print_cursor_movement, yatf_fixture) {
    printer::print(printer::cursor_movement::up);
    BOOST_CHECK_EQUAL(get_buffer(), "\e[1A");
}

BOOST_FIXTURE_TEST_CASE(can_print_const_string, yatf_fixture) {
    const char *str = "test string";
    printer::print(str);
    BOOST_CHECK_EQUAL(get_buffer(), str);
}

BOOST_FIXTURE_TEST_CASE(can_print_string, yatf_fixture) {
    char str[] = "test string";
    printer::print(str);
    BOOST_CHECK_EQUAL(get_buffer(), str);
}

BOOST_AUTO_TEST_SUITE_END()

