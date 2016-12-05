#pragma once

#include <cstddef>

namespace yatf {

using tests_printer = int (*)(const char *, ...);

namespace detail {

extern tests_printer _print;

struct printer {

    enum class cursor_movement {
        up
    };

    enum class color {
        red,
        green,
        reset
    };

    static void print(const char *str) {
        _print(str);
    }

    static void print(char c) {
        _print("%c", c);
    }

    static void print(unsigned char c) {
        _print("0x%x", c);
    }

    static void print(short a) {
        _print("%d", a);
    }

    static void print(unsigned short a) {
        _print("0x%x", a);
    }

    static void print(int a) {
        _print("%d", a);
    }

    static void print(unsigned int a) {
        _print("0x%x", a);
    }

    static void print(void *a) {
        _print("0x%x", reinterpret_cast<unsigned long>(a));
    }

    static void print(std::nullptr_t) {
        _print("NULL");
    }

    static void print(color c) {
        switch (c) {
            case color::red:
                _print("\e[31m");
                break;
            case color::green:
                _print("\e[32m");
                break;
            case color::reset:
                _print("\e[0m");
                break;
        }
    }

    static void print(cursor_movement c) {
        switch (c) {
            case cursor_movement::up:
                _print("\033[1A");
                break;
        }
    }

    template <typename T>
    static void print(const T &a) {
        _print("0x%x", reinterpret_cast<unsigned long>(&a));
    }

    template<typename First, typename... Rest>
    static void print(const First &first, const Rest &... rest) {
        print(first);
        print(rest...);
    }

};

struct test_session final {

    struct messages final {

        enum class msg {
            start_end, run, pass, fail
        };

        static const char *get(msg m) {
            const char *_run_message[4] = {"[========]",  "[  RUN   ]", "[  PASS  ]", "[  FAIL  ]"};
            return _run_message[static_cast<int>(m)];
        }

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

    struct test_case final : public tests_list<test_case> {

        const char *suite_name;
        const char *test_name;
        void (*func)();
        unsigned assertions = 0;
        unsigned failed = 0;

        test_case(const char *suite, const char *test, void (*func)())
                : suite_name(suite), test_name(test), func(func) {
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
    static test_session _instance;


    void print_test_start_message(test_case &t) const {
        printer::print(printer::color::green, messages::get(messages::msg::run), printer::color::reset, " ",  t.suite_name, ".", t.test_name, "\n");
    }

    void print_test_result(test_case &t) const {
        if (t.failed) {
            printer::print(printer::color::red, messages::get(messages::msg::fail), printer::color::reset, " ");
            printer::print(t.suite_name, ".", t.test_name, " (", static_cast<int>(t.assertions), " assertions)\n");
        }
        else {
#if ONELINERS
            printer::print(printer::cursor_movement::up);
#endif
            printer::print(printer::color::green, messages::get(messages::msg::pass), printer::color::reset, " ", t.suite_name, ".", t.test_name, " (", static_cast<int>(t.assertions), " assertions)\n");
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

    int run() {
        unsigned failed = 0;
        unsigned test_cases = 0;;
        printer::print(printer::color::green, messages::get(messages::msg::start_end), printer::color::reset, " Running ", static_cast<int>(_tests_number), " test cases\n");
        for (auto &test : _test_cases) {
            print_test_start_message(test);
            _current_test_case = &test;
            if (test.call()) failed++;
            print_test_result(test);
            test_cases++;
        }
        printer::print(printer::color::green, messages::get(messages::msg::start_end), printer::color::reset, " Passed ", static_cast<int>(_tests_number - failed), " test cases\n");
        if (failed) printer::print(printer::color::red, messages::get(messages::msg::start_end), printer::color::reset, " Failed ", static_cast<int>(failed), " test cases\n");
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
tests_printer _print;

} // namespace detail

inline int main(tests_printer print_func) {
    detail::_print = print_func;
    return detail::test_session::get().run();
}

#endif

} // namespace yatf

