#include "../include/yatf.h"
#include <boost/test/unit_test.hpp>
#include <string>
#include <iomanip>
#include <sstream>

extern std::string get_buffer();

using namespace yatf::detail;

BOOST_AUTO_TEST_CASE(can_print_char) {
    get_buffer();
    for (char i = 1; i < 127; i++) {
        printer::print(i);
        BOOST_CHECK_EQUAL(get_buffer(), std::string(1, i));
    }
}

BOOST_AUTO_TEST_CASE(can_print_unsigned_char) {
    get_buffer();
    for (unsigned char i = 0; i < 255; i++) {
        printer::print(i);
        std::stringstream out;
        out << std::hex << (unsigned int)i;
        BOOST_CHECK_EQUAL(get_buffer(), "0x" + out.str());
    }
}

BOOST_AUTO_TEST_CASE(can_print_short) {
    get_buffer();
    for (short i = -1024; i < 1025; i++) {
        printer::print(i);
        BOOST_CHECK_EQUAL(get_buffer(), std::to_string(i));
    }
}

BOOST_AUTO_TEST_CASE(can_print_unsigned_short) {
    get_buffer();
    for (unsigned short i = 0; i < 1025; i++) {
        printer::print(i);
        std::stringstream out;
        out << std::hex << i;
        BOOST_CHECK_EQUAL(get_buffer(), "0x" + out.str());
    }
}

BOOST_AUTO_TEST_CASE(can_print_int) {
    get_buffer();
    for (int i = -1024; i < 1025; i++) {
        printer::print(i);
        BOOST_CHECK_EQUAL(get_buffer(), std::to_string(i));
    }
}

BOOST_AUTO_TEST_CASE(can_print_unsigned_int) {
    get_buffer();
    for (unsigned int i = 0; i < 1025; i++) {
        printer::print(i);
        std::stringstream out;
        out << std::hex << i;
        BOOST_CHECK_EQUAL(get_buffer(), "0x" + out.str());
    }
}

BOOST_AUTO_TEST_CASE(can_print_address) {
    get_buffer();
    for (unsigned int i = 0; i < 1025; i++) {
        printer::print(reinterpret_cast<void *>(i));
        std::stringstream out;
        out << std::hex << i;
        BOOST_CHECK_EQUAL(get_buffer(), "0x" + out.str());
    }
}

BOOST_AUTO_TEST_CASE(can_print_nullptr) {
    get_buffer();
    printer::print(nullptr);
    BOOST_CHECK_EQUAL(get_buffer(), "NULL");
}

BOOST_AUTO_TEST_CASE(can_print_colors) {
    get_buffer();
    printer::print(printer::color::red);
    BOOST_CHECK_EQUAL(get_buffer(), "\e[31m");
    printer::print(printer::color::green);
    BOOST_CHECK_EQUAL(get_buffer(), "\e[32m");
    printer::print(printer::color::reset);
    BOOST_CHECK_EQUAL(get_buffer(), "\e[0m");
}

BOOST_AUTO_TEST_CASE(can_print_cursor_movement) {
    get_buffer();
    printer::print(printer::cursor_movement::up);
    BOOST_CHECK_EQUAL(get_buffer(), "\e[1A");
}

