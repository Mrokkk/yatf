#pragma once

#include <stddef.h>

namespace yatf {

using tests_printer = int (*)(const char *, ...);

namespace detail {

// Minimal version of inherited_list
template <typename Type>
class tests_list {

    Type *_prev, *_next;

    void add_element(Type &new_element, Type &prev, Type &next) {
        next._prev = &new_element;
        prev._next = &new_element;
        new_element._next = &next;
        new_element._prev = &prev;
    }

    operator Type &() {
        return *reinterpret_cast<Type *>(this);
    }

public:

    class iterator {

        Type *_ptr = nullptr;

    public:

        iterator(Type *t)
            : _ptr(t) {}

        iterator &operator++() {
            _ptr = _ptr->next();
            return *this;
        }

        Type &operator*() {
            return *_ptr;
        }

        bool operator!=(const iterator &comp) {
            return _ptr != comp._ptr;
        }

    };

    tests_list() {
        _next = _prev = reinterpret_cast<Type *>(this);
    }

    Type &add(Type *new_element) {
        add_element(*new_element, *_prev, *this);
        return *this;
    }

    Type *next() {
        return _next == this ? nullptr : _next;
    }

    auto begin() {
        return iterator(_next);
    }

    auto end() {
        return iterator(reinterpret_cast<Type *>(this));
    }

};

extern tests_printer _print;

struct test_session final {

    class test_case final : public tests_list<test_case> {

        const char *_suite_name;
        const char *_test_name;
        void (*_func)();
        const char *_run_message = "\e[32m[  RUN   ]\e[0m";
        const char *_pass_message = "\e[32m[  PASS  ]\e[0m";
        const char *_fail_message = "\e[31m[  FAIL  ]\e[0m";

        void print_test_start_message() {
            yatf::detail::_print("%s %s.%s\n", _run_message, _suite_name, _test_name);
        }

        void print_test_result() {
            if (failed)
                yatf::detail::_print("%s ", _fail_message);
            else
                yatf::detail::_print("%s ", _pass_message);
            yatf::detail::_print("%s.%s (%u assertions)\n\n", _suite_name, _test_name, assertions);
        }

    public:

        int assertions = 0;
        int failed = 0;

        test_case(const char *suite, const char *test, void (*func)())
                : _suite_name(suite), _test_name(test), _func(func) {
            test_session::get().register_test(this);
        }

        void assert(bool cond) {
            ++assertions;
            if (!cond) ++failed;
        }

        int call() {
            print_test_start_message();
            _func();
            print_test_result();
            return failed;
        }

    };

private:

    tests_list<test_case> _test_cases;
    test_case *_current_test_case;
    size_t _tests_number = 0;
    static test_session _instance;

public:

    static test_session &get() {
        return _instance;
    }

    void register_test(test_case *t) {
        _tests_number++;
        _test_cases.add(t);
    }

    int run() {
        unsigned failed = 0;
        unsigned test_cases = 0;;
        yatf::detail::_print("\e[32m[========]\e[0m Running %u test cases\n\n", _tests_number);
        for (auto &test : _test_cases) {
            _current_test_case = &test;
            if (test.call()) failed++;
            test_cases++;
        }
        yatf::detail::_print("\e[32m[========]\e[0m Passed %u test cases\n", test_cases - failed);
        if (failed) yatf::detail::_print("\e[31m[========]\e[0m Failed %u test cases\n", failed);
        return failed;
    }

    test_case &current_test_case() {
        return *_current_test_case;
    }

};

} // namespace detail

#define REQUIRE(cond) \
    { \
        yatf::detail::test_session::get().current_test_case().assert(cond); \
        if (!(cond)) \
            yatf::detail::_print("assertion failed: %s:%d: \'%s\' is false\n", __FILE__, __LINE__, #cond); \
    }

#define REQUIRE_FALSE(cond) \
    { \
        yatf::detail::test_session::get().current_test_case().assert(!(cond)); \
        if (cond) \
            yatf::detail::_print("assertion failed: %s:%d: \'%s\' is true\n", __FILE__, __LINE__, #cond); \
    }

#define YATF_UNIQUE_NAME(name) \
    name##__COUNTER__

#define TEST(suite, name) \
    static void suite##_##name(); \
    yatf::detail::test_session::test_case YATF_UNIQUE_NAME(suite##_##name){#suite, #name, suite##_##name}; \
    static void suite##_##name()

#ifdef YATF_MAIN

namespace detail {

test_session test_session::_instance;
tests_printer _print;

} // namespace detail

int main(tests_printer printer) {
    detail::_print = printer;
    return detail::test_session::get().run();
}

#endif

} // namespace yatf

