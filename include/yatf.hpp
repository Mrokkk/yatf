#pragma once

#include <type_traits>
#include "list.hpp"
#include "printer.hpp"
#include "yatf_mock.hpp"

struct yatf_fixture;

namespace yatf {

struct config final {
    bool color = true;
    bool oneliners = false;
    bool fails_only = false;
    constexpr config() = default;
    explicit config(bool color, bool oneliners, bool fails_only)
        : color(color), oneliners(oneliners), fails_only(fails_only) {}
};

namespace detail {

struct empty_fixture {};

inline int compare_strings(const char *s1, const char *s2) {
    while(*s1 && (*s1 == *s2))
        ++s1, ++s2;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

inline char *find(char *str, char c) {
    while (*str) {
        if (*str == c) {
            return str;
        }
        ++str;
    }
    return nullptr;
}

inline void copy_string(const char *src, char *dest) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = 0;
}

struct test_session final {

    struct messages final {

        enum class msg { start_end, run, pass, fail };

        static const char *get(msg m) {
            static const char *run_messages_[4] = {"[========]",  "[  RUN   ]", "[  PASS  ]", "[  FAIL  ]"};
            return run_messages_[static_cast<int>(m)];
        }

    };

    class test_case {

        using fn = void(*)();
        list<test_case>::node node_;

    public:
        const char *suite_name;
        const char *test_name;
        std::size_t assertions = 0;
        std::size_t failed = 0;

        void require_true(bool condition, const char *condition_str, const char *file, int line) {
            ++assertions;
            if (!condition) {
                ++failed;
                printer_ << "assertion failed: " << file << ':' << line << " \'" << condition_str << "\' is false\n";
            }
        }

        void require_false(bool condition, const char *condition_str, const char *file, int line) {
            ++assertions;
            if (condition) {
                ++failed;
                printer_ << "assertion failed: " << file << ':' << line << " \'" << condition_str << "\' is true\n";
            }
        }

        template <typename T1, typename T2>
        void require_eq(const T1 &lhs, const T2 &rhs, const char *lhs_str, const char *rhs_str,
                const char *file, int line) {
            ++assertions;
            bool cond = (lhs == rhs);
            if (!cond) {
                ++failed;
                printer_ << "assertion failed: " << file << ':' << line << " \'" << lhs_str
                         << "\' isn't \'" << rhs_str << "\': " << lhs << " != " << rhs << "\n";
            }
        }

        void require_eq(const char *lhs, const char *rhs, const char *, const char *, const char *file, int line) {
            ++assertions;
            bool cond = compare_strings(lhs, rhs) == 0;
            if (!cond) {
                ++failed;
                printer_ << "assertion failed: " << file << ':' << line << " \'" << lhs
                         << "\' isn't \'" << rhs << "\n";
            }
        }

        void require_call(const char *mock_name, std::size_t expected_nr_of_calls,
                std::size_t actual_nr_of_calls, const char *file, int line) {
            ++assertions;
            if (expected_nr_of_calls != actual_nr_of_calls) {
                ++failed;
                printer_ << "assertion failed: " << file << ':' << line << " " << mock_name
                         << ": expected to be called: " << expected_nr_of_calls << "; actual: "
                         << actual_nr_of_calls << "\n";
            }
        }

        virtual void test_body() = 0;

        friend test_session;

    };

private:

    list<test_case> test_cases_;
    test_case *current_test_case_;
    std::size_t tests_number_ = 0;
    config config_;
    static test_session instance_;
    friend yatf_fixture;

    void print_in_color(const char *str, printer::color color) const {
        if (config_.color) printer_ << color;
        printer_ << str;
        if (config_.color) printer_ << printer::color::reset;
    }

    void test_session_start_message() const {
        print_in_color(messages::get(messages::msg::start_end), printer::color::green);
        printer_ << " Running " << static_cast<int>(tests_number_) << " test cases\n";
    }

    void test_session_end_message(int failed) const {
        if (config_.fails_only && config_.oneliners)
            printer_ << printer::cursor_movement::up;
        print_in_color(messages::get(messages::msg::start_end), printer::color::green);
        printer_ << " Passed " << static_cast<int>(tests_number_ - failed) << " test cases\n";
        if (failed) {
            print_in_color(messages::get(messages::msg::start_end), printer::color::red);
            printer_ << " Failed " << static_cast<int>(failed) << " test cases\n";
        }
    }

    void test_start_message(test_case &t) const {
        if (config_.fails_only) return;
        print_in_color(messages::get(messages::msg::run), printer::color::green);
        printer_ << " " << t.suite_name << "." << t.test_name << "\n";
    }

    void test_result(test_case &t) const {
        if (t.failed) {
            print_in_color(messages::get(messages::msg::fail), printer::color::red);
            printer_ << " " << t.suite_name << "." << t.test_name << " (" << static_cast<int>(t.assertions) << " assertions)\n";
        }
        else {
            if (config_.fails_only) return;
            if (config_.oneliners)
                printer_ << printer::cursor_movement::up;
            print_in_color(messages::get(messages::msg::pass), printer::color::green);
            printer_ << " " << t.suite_name << "." << t.test_name << " (" << static_cast<int>(t.assertions) << " assertions)\n";
        }
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
                test.test_body();
                auto result = test.failed;
                test_result(test);
                return result;
            }
        }
        print_in_color(messages::get(messages::msg::fail), printer::color::red);
        printer_ << " error because of bad test name\n";
        return -1;
    }

public:

    test_session() : test_cases_(&test_case::node_) {
    }

    static test_session &get() {
        return instance_;
    }

    void register_test(test_case *t) {
        ++tests_number_;
        test_cases_.push_back(*t);
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
            test.test_body();
            if (test.failed)
                ++failed;
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

} // namespace detail

#define REQUIRE(cond) \
    yatf::detail::test_session::get().current_test_case().require_true(cond, #cond, __FILE__, __LINE__)

#define REQUIRE_FALSE(cond) \
    yatf::detail::test_session::get().current_test_case().require_false(cond, #cond, __FILE__, __LINE__)

#define REQUIRE_EQ(lhs, rhs) \
    yatf::detail::test_session::get().current_test_case().require_eq(lhs, rhs, #lhs, #rhs, __FILE__, __LINE__)

#define YATF_CONCAT_(x,y) x##y
#define YATF_CONCAT(x,y) YATF_CONCAT_(x, y)

#define YATF_UNIQUE_NAME(name) \
    YATF_CONCAT(name, __LINE__)

#define YATF_TEST_FIXTURE(suite, name, f) \
    struct suite##__##name final : public yatf::detail::test_session::test_case, public f { \
        explicit suite##__##name(const char *sn, const char *tn) { \
            suite_name = sn; \
            test_name = tn; \
            yatf::detail::test_session::get().register_test(this); \
        } \
        void test_body() override; \
    } YATF_UNIQUE_NAME(suite##_##name){#suite, #name}; \
    void suite##__##name::test_body()

#define YATF_TEST(suite, name) \
    YATF_TEST_FIXTURE(suite, name, ::yatf::detail::empty_fixture)

#define GET_4TH(_1, _2, _3, NAME, ...) NAME
#define TEST(...) GET_4TH(__VA_ARGS__, YATF_TEST_FIXTURE, YATF_TEST)(__VA_ARGS__)

#ifdef YATF_MAIN

namespace detail {

test_session test_session::instance_;
printer printer_;
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

