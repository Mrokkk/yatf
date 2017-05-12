#pragma once

#include <type_traits>

struct yatf_fixture;

namespace yatf {

using printf_t = int (*)(const char *, ...);

struct config final {
    bool color = true;
    bool oneliners = false;
    bool fails_only = false;
    constexpr config() {}
    explicit config(bool color, bool oneliners, bool fails_only)
        : color(color), oneliners(oneliners), fails_only(fails_only) {}
};

namespace detail {

extern printf_t printf_;

inline int compare_strings(const char *s1, const char *s2) {
    while(*s1 && (*s1 == *s2))
        s1++, s2++;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

struct printer {

    enum class cursor_movement { up };

    enum class color { red, green, reset };

    template <typename T>
    static typename std::enable_if<std::is_signed<T>::value>::type print(T a) {
        printf_("%d", a);
    }

    template <typename T>
    static typename std::enable_if<std::is_unsigned<T>::value>::type print(T a) {
        printf_("%u", a);
    }

    template <typename T>
    static typename std::enable_if<
        std::is_same<T, char *>::value ||
        std::is_same<T, const char *>::value
    >::type
    print(T str) {
        printf_(str);
    }

    static void print(char c) {
        printf_("%c", c);
    }

    template <typename T>
    static typename std::enable_if<
        std::is_pointer<T>::value &&
        !std::is_same<T, char *>::value &&
        !std::is_same<T, const char *>::value
    >::type print(T a) {
        printf_("0x%x", reinterpret_cast<unsigned long>(a));
    }

    static void print(std::nullptr_t) {
        printf_("NULL");
    }

    static void print(color c) {
        switch (c) {
            case color::red:
                printf_("\e[31m");
                break;
            case color::green:
                printf_("\e[32m");
                break;
            case color::reset:
                printf_("\e[0m");
                break;
        }
    }

    static void print(cursor_movement c) {
        switch (c) {
            case cursor_movement::up:
                printf_("\033[1A");
                break;
        }
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
            const char *run_messages_[4] = {"[========]",  "[  RUN   ]", "[  PASS  ]", "[  FAIL  ]"};
            return run_messages_[static_cast<int>(m)];
        }

    };

    // Minimal version of inherited_list
    template <typename Type>
    class tests_list {

        Type *prev_, *next_;

        void add_element(Type &new_element, Type &prev, Type &next) {
            next.prev_ = prev.next_ = &new_element;
            new_element.next_ = &next;
            new_element.prev_ = &prev;
        }

        operator Type &() {
            return *reinterpret_cast<Type *>(this);
        }

    public:

        class iterator {

            Type *ptr_ = nullptr;

        public:

            iterator(Type *t)
                : ptr_(t) {}

            iterator &operator++() {
                ptr_ = ptr_->next();
                return *this;
            }

            Type &operator*() {
                return *ptr_;
            }

            bool operator!=(const iterator &comp) {
                return ptr_ != comp.ptr_;
            }

        };

        tests_list() {
            next_ = prev_ = reinterpret_cast<Type *>(this);
        }

        Type &add(Type *new_element) {
            add_element(*new_element, *prev_, *this);
            return *this;
        }

        Type *next() {
            return next_ == this ? nullptr : next_;
        }

        iterator begin() {
            return iterator(next_);
        }

        iterator end() {
            return iterator(reinterpret_cast<Type *>(this));
        }

    };

    class test_case final : public tests_list<test_case> {

        void (*test_case_function_)();
        void (*runner_)(void (*)());

    public:

        const char *suite_name;
        const char *test_name;
        unsigned assertions = 0;
        unsigned failed = 0;

        explicit test_case(const char *suite, const char *test, void (*func)())
                : test_case_function_(func), suite_name(suite), test_name(test) {
            test_session::get().register_test(this);
            runner_ = [](void (*f)()){ f(); };
        }

        bool assert_true(bool cond) {
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

        unsigned call() {
            runner_(test_case_function_);
            return failed;
        }

    };

private:

    tests_list<test_case> test_cases_;
    test_case *current_test_case_;
    unsigned tests_number_ = 0;
    config config_;
    static test_session instance_;
    friend yatf_fixture;

    void print_in_color(const char *str, printer::color color) const {
        if (config_.color) printer::print(color);
        printer::print(str);
        if (config_.color) printer::print(printer::color::reset);
    }

    void test_session_start_message() const {
        print_in_color(messages::get(messages::msg::start_end), printer::color::green);
        printer::print(" Running ", static_cast<int>(tests_number_), " test cases\n");
    }

    void test_session_end_message(int failed) const {
        if (config_.fails_only && config_.oneliners)
            printer::print(printer::cursor_movement::up);
        print_in_color(messages::get(messages::msg::start_end), printer::color::green);
        printer::print(" Passed ", static_cast<int>(tests_number_ - failed), " test cases\n");
        if (failed) {
            print_in_color(messages::get(messages::msg::start_end), printer::color::red);
            printer::print(" Failed ", static_cast<int>(failed), " test cases\n");
        }
    }

    void test_start_message(test_case &t) const {
        if (config_.fails_only) return;
        print_in_color(messages::get(messages::msg::run), printer::color::green);
        printer::print(" ",  t.suite_name, ".", t.test_name, "\n");
    }

    void test_result(test_case &t) const {
        if (t.failed) {
            print_in_color(messages::get(messages::msg::fail), printer::color::red);
            printer::print(" ", t.suite_name, ".", t.test_name, " (", static_cast<int>(t.assertions), " assertions)\n");
        }
        else {
            if (config_.fails_only) return;
            if (config_.oneliners)
                printer::print(printer::cursor_movement::up);
            print_in_color(messages::get(messages::msg::pass), printer::color::green);
            printer::print(" ", t.suite_name, ".", t.test_name, " (", static_cast<int>(t.assertions), " assertions)\n");
        }
    }

    char *find(char *str, char c) const {
        while (*str) {
            if (*str == c) {
                return str;
            }
            str++;
        }
        return nullptr;
    }

    void copy_string(const char *src, char *dest) {
        while (*src) {
            *dest++ = *src++;
        }
        *dest = 0;
    }

    int call_one_test(const char *test_name) {
        char suite_name[512]; // FIXME
        copy_string(test_name, suite_name);
        auto dot_position = find(suite_name, '.');
        if (dot_position == nullptr) {
            return -1;
        }
        *dot_position = 0;
        auto case_name = dot_position + 1;
        for (auto &test : test_cases_) {
            if (compare_strings(test.test_name, case_name) == 0 &&
                compare_strings(test.suite_name, suite_name) == 0) {
                test_start_message(test);
                current_test_case_ = &test;
                auto result = test.call();
                test_result(test);
                return result;
            }
        }
        print_in_color(messages::get(messages::msg::fail), printer::color::red);
        printer::print(" error because of bad test name\n");
        return -1;
    }

public:

    static test_session &get() {
        return instance_;
    }

    void register_test(test_case *t) {
        tests_number_++;
        test_cases_.add(t);
    }

    int run(config c, const char *test_name = nullptr) {
        config_ = c;
        if (test_name) {
            return call_one_test(test_name);
        }
        auto failed = 0u;
        test_session_start_message();
        for (auto &test : test_cases_) {
            test_start_message(test);
            current_test_case_ = &test;
            if (test.call())
                failed++;
            test_result(test);
        }
        test_session_end_message(failed);
        return failed;
    }

    test_case &current_test_case() {
        return *current_test_case_;
    }

    void current_test_case(test_case *tc) {
        current_test_case_ = tc; // for tests only
    }

};

template <>
inline bool test_session::test_case::assert_eq(const char *lhs, const char *rhs) {
    ++assertions;
    auto cond = compare_strings(lhs, rhs) == 0;
    if (!cond) failed++;
    return cond;
}

} // namespace detail

#define REQUIRE(cond) \
    do { \
        if (!yatf::detail::test_session::get().current_test_case().assert_true(cond)) \
            yatf::detail::printer::print("assertion failed: ", __FILE__, ':', __LINE__, " \'", #cond, "\' is false\n"); \
    } while (0)

#define REQUIRE_FALSE(cond) \
    do { \
        if (!yatf::detail::test_session::get().current_test_case().assert_true(!(cond))) \
            yatf::detail::printer::print("assertion failed: ", __FILE__, ':', __LINE__, " \'", #cond, "\' is true\n"); \
    } while (0)

#define REQUIRE_EQ(lhs, rhs) \
    do { \
        if (!yatf::detail::test_session::get().current_test_case().assert_eq(lhs, rhs)) { \
            yatf::detail::printer::print("assertion failed: ", __FILE__, ':', __LINE__, " \'", #lhs, "\' isn't \'", #rhs, "\': "); \
            yatf::detail::printer::print(lhs, " != ", rhs, "\n"); \
        } \
    } while (0)

#define YATF_CONCAT_(x,y) x##y
#define YATF_CONCAT(x,y) YATF_CONCAT_(x,y)

#define YATF_UNIQUE_NAME(name) \
    YATF_CONCAT(name, __LINE__)

#define YATF_TEST(suite, name) \
    static void suite##__##name(); \
    yatf::detail::test_session::test_case YATF_UNIQUE_NAME(suite##_##name){#suite, #name, suite##__##name}; \
    static void suite##__##name()

#define GET_4TH(_1, _2, _3, NAME, ...) NAME
#define TEST(...) GET_4TH(__VA_ARGS__, UNUSED, YATF_TEST)(__VA_ARGS__)

#ifdef YATF_MAIN

namespace detail {

test_session test_session::instance_;
printf_t printf_;

} // namespace detail

inline config read_config(unsigned argc, const char **argv) {
    config c;
    for (unsigned i = 1; i < argc; ++i) {
        if (!detail::compare_strings(argv[i], "--no-color")) c.color = false;
        else if (!detail::compare_strings(argv[i], "--oneliners")) c.oneliners = true;
        else if (!detail::compare_strings(argv[i], "--fails-only")) c.fails_only = true;
    }
    return c;
}

inline int main(printf_t print_func, unsigned argc = 0, const char **argv = nullptr) {
    detail::printf_ = print_func;
    return detail::test_session::get().run(read_config(argc, argv));
}

inline int main(printf_t print_func, config &c) {
    detail::printf_ = print_func;
    return detail::test_session::get().run(c);
}

inline int run_one(printf_t print_func, const char *test_name, config &c) {
    if (test_name == nullptr) {
        return -1;
    }
    detail::printf_ = print_func;
    return detail::test_session::get().run(c, test_name);
}

#endif

} // namespace yatf

