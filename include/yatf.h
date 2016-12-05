#pragma once

#include <cstddef>

namespace yatf {

using printf_t = int (*)(const char *, ...);

namespace detail {

extern printf_t _printf;

struct printer {

    enum class cursor_movement { up };

    enum class color { red, green, reset };

    static void print(const char *str) {
        _printf(str);
    }

    static void print(char c) {
        _printf("%c", c);
    }

    static void print(unsigned char c) {
        _printf("0x%x", c);
    }

    static void print(short a) {
        _printf("%d", a);
    }

    static void print(unsigned short a) {
        _printf("0x%x", a);
    }

    static void print(int a) {
        _printf("%d", a);
    }

    static void print(unsigned int a) {
        _printf("0x%x", a);
    }

    static void print(void *a) {
        _printf("0x%x", reinterpret_cast<unsigned long>(a));
    }

    static void print(std::nullptr_t) {
        _printf("NULL");
    }

    static void print(color c) {
        switch (c) {
            case color::red:
                _printf("\e[31m");
                break;
            case color::green:
                _printf("\e[32m");
                break;
            case color::reset:
                _printf("\e[0m");
                break;
        }
    }

    static void print(cursor_movement c) {
        switch (c) {
            case cursor_movement::up:
                _printf("\033[1A");
                break;
        }
    }

    template <typename T>
    static void print(const T &a) {
        _printf("0x%x", reinterpret_cast<unsigned long>(&a));
    }

    template<typename First, typename... Rest>
    static void print(const First &first, const Rest &... rest) {
        print(first);
        print(rest...);
    }

};

struct test_session final {

    struct messages final {

        enum class msg { start_end, run, pass, fail };

        static const char *get(msg m) {
            const char *_run_message[4] = {"[========]",  "[  RUN   ]", "[  PASS  ]", "[  FAIL  ]"};
            return _run_message[static_cast<int>(m)];
        }

    };

    struct config final {
        bool color = true;
        bool oneliners = false;
        bool fails_only = false;
    };

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

    class test_case final : public tests_list<test_case> {

        void (*func)();

    public:

        const char *suite_name;
        const char *test_name;
        unsigned assertions = 0;
        unsigned failed = 0;

        test_case(const char *suite, const char *test, void (*func)())
                : func(func), suite_name(suite), test_name(test) {
            test_session::get().register_test(this);
        }

        bool assert(bool cond) {
            ++assertions;
            if (!cond) ++failed;
            return cond;
        }

        template <typename T1, typename T2>
        bool assert_eq(T1 lhs, T2 rhs) {
            (void)lhs; (void)rhs; // for gcc
            ++assertions;
            bool cond = (lhs == rhs);
            if (!cond) ++failed;
            return cond;
        }

        int call() {
            func();
            return failed;
        }

    };

private:

    tests_list<test_case> _test_cases;
    test_case *_current_test_case;
    size_t _tests_number = 0;
    config _config;
    static test_session _instance;

    void print_in_color(const char *str, printer::color color) const {
        if (_config.color) printer::print(color);
        printer::print(str);
        if (_config.color) printer::print(printer::color::reset);
    }

    void test_session_start_message() const {
        print_in_color(messages::get(messages::msg::start_end), printer::color::green);
        printer::print(" Running ", static_cast<int>(_tests_number), " test cases\n");
    }

    void test_session_end_message(int failed) const {
        if (_config.fails_only && _config.oneliners)
            printer::print(printer::cursor_movement::up);
        print_in_color(messages::get(messages::msg::start_end), printer::color::green);
        printer::print(" Passed ", static_cast<int>(_tests_number - failed), " test cases\n");
        if (failed) {
            print_in_color(messages::get(messages::msg::start_end), printer::color::red);
            printer::print(" Failed ", static_cast<int>(failed), " test cases\n");
        }
    }

    void test_start_message(test_case &t) const {
        if (_config.fails_only) return;
        print_in_color(messages::get(messages::msg::run), printer::color::green);
        printer::print(" ",  t.suite_name, ".", t.test_name, "\n");
    }

    void test_result(test_case &t) const {
        if (t.failed) {
            print_in_color(messages::get(messages::msg::fail), printer::color::red);
            printer::print(" ", t.suite_name, ".", t.test_name, " (", static_cast<int>(t.assertions), " assertions)\n");
        }
        else {
            if (_config.fails_only) return;
            if (_config.oneliners)
                printer::print(printer::cursor_movement::up);
            print_in_color(messages::get(messages::msg::pass), printer::color::green);
            printer::print(" ", t.suite_name, ".", t.test_name, " (", static_cast<int>(t.assertions), " assertions)\n");
        }
    }

public:

    static test_session &get() {
        return _instance;
    }

    void register_test(test_case *t) {
        _tests_number++;
        _test_cases.add(t);
    }

    int run(config c) {
        unsigned failed = 0;
        _config = c;
        test_session_start_message();
        for (auto &test : _test_cases) {
            test_start_message(test);
            _current_test_case = &test;
            if (test.call())
                failed++;
            test_result(test);
        }
        test_session_end_message(failed);
        return failed;
    }

    test_case &current_test_case() {
        return *_current_test_case;
    }

};

} // namespace detail

#define REQUIRE(cond) \
    { \
        if (!yatf::detail::test_session::get().current_test_case().assert(cond)) \
            yatf::detail::printer::print("assertion failed: ", __FILE__, ':', __LINE__, " \'", #cond, "\' is false\n"); \
    }

#define REQUIRE_FALSE(cond) \
    { \
        if (!yatf::detail::test_session::get().current_test_case().assert(!(cond))) \
            yatf::detail::printer::print("assertion failed: ", __FILE__, ':', __LINE__, " \'", #cond, "\' is true\n"); \
    }

#define REQUIRE_EQ(lhs, rhs) \
    { \
        if (!yatf::detail::test_session::get().current_test_case().assert_eq(lhs, rhs)) { \
            yatf::detail::printer::print("assertion failed: ", __FILE__, ':', __LINE__, " \'", #lhs, "\' isn't \'", #rhs, "\': "); \
            yatf::detail::printer::print(lhs, " != ", rhs, "\n"); \
        } \
    }

// gtest compatible
#define EXPECT_EQ(lhs, rhs) REQUIRE_EQ(lhs, rhs)
#define EXPECT_TRUE(cond) REQUIRE(cond)

#define YATF_UNIQUE_NAME(name) \
    name##__LINE__

#define TEST(suite, name) \
    static void suite##_##name(); \
    yatf::detail::test_session::test_case YATF_UNIQUE_NAME(suite##_##name){#suite, #name, suite##_##name}; \
    static void suite##_##name()

#ifdef YATF_MAIN

namespace detail {

test_session test_session::_instance;
printf_t _printf;

} // namespace detail

inline int strcmp(const char *string1, const char *string2) {
    if (string1 == 0 || string2 == 0) return 1;
    while (1) {
        if (*string1++ != *string2++)
            return 1;
        if (*string1 == '\0' && *string2 == '\0')
            return 0;
        if (*string1 == '\0' || *string2 == '\0')
            return 1;
    }
    return 0;
}

inline detail::test_session::config config(unsigned argc, const char **argv) {
    detail::test_session::config c;
    for (unsigned i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--no-color")) c.color = false;
        else if (!strcmp(argv[i], "--oneliners")) c.oneliners = true;
        else if (!strcmp(argv[i], "--fails-only")) c.fails_only = true;
    }
    return c;
}

inline int main(printf_t print_func, unsigned argc = 0, const char **argv = nullptr) {
    detail::_printf = print_func;
    return detail::test_session::get().run(config(argc, argv));
}

#endif

} // namespace yatf

