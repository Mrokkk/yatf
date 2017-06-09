#pragma once

#include <type_traits>

struct yatf_fixture;

namespace yatf {

using printf_t = int (*)(const char *, ...);

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

class void_function final {

    struct callable {
        virtual void operator()() const = 0;
    };

    template <typename ClosureType>
    class closure: public callable {

        const ClosureType func_;

    public:

        explicit closure(const ClosureType &handler) : func_(handler) {
        }

        void operator()() const override {
            func_();
        }

        constexpr void *operator new(std::size_t, void *address) {
            return address;
        }

    };

    const callable *func_wrapper_ = nullptr;
    char data_[2 * sizeof(void *)];

public:

    void_function() = default;

    void_function(const void_function &fn) {
        func_wrapper_ = fn.func_wrapper_;
    }

    void_function(void_function &&fn) {
        func_wrapper_ = fn.func_wrapper_;
        fn.func_wrapper_ = nullptr;
    }

    template<typename ClosureType>
    void_function(ClosureType &&function)
            : func_wrapper_(new (data_) closure<ClosureType>(function)) {
    }

    template<typename ClosureType>
    void operator=(ClosureType function) {
        func_wrapper_ = new (data_) closure<ClosureType>(function);
    }

    void operator()() const {
        (*func_wrapper_)();
    }

};

extern printf_t printf_;

inline int compare_strings(const char *s1, const char *s2) {
    while(*s1 && (*s1 == *s2))
        s1++, s2++;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

struct printer final {

    enum class cursor_movement { up };
    enum class color { red, green, reset };

    template <typename T>
    typename std::enable_if<
        std::is_signed<T>::value, printer &
    >::type operator<<(T a) {
        printf_("%d", a);
        return *this;
    }

    template <typename T>
    typename std::enable_if<
        std::is_unsigned<T>::value, printer &
    >::type operator<<(T a) {
        printf_("%u", a);
        return *this;
    }

    template <typename T>
    typename std::enable_if<
        std::is_same<T, char *>::value ||
        std::is_same<T, const char *>::value, printer &
    >::type operator<<(T str) {
        printf_(str);
        return *this;
    }

    printer &operator<<(char c) {
        printf_("%c", c);
        return *this;
    }

    template <typename T>
    typename std::enable_if<
        std::is_pointer<T>::value &&
        !std::is_same<T, char *>::value &&
        !std::is_same<T, const char *>::value,
        printer &
    >::type operator<<(T a) {
        printf_("0x%x", reinterpret_cast<unsigned long>(a));
        return *this;
    }

    printer &operator<<(std::nullptr_t) {
        printf_("NULL");
        return *this;
    }

    printer &operator<<(color c) {
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
        return *this;
    }

    printer &operator<<(cursor_movement c) {
        switch (c) {
            case cursor_movement::up:
                printf_("\033[1A");
                break;
        }
        return *this;
    }

};

extern printer printer_;

template <typename Type>
class list final {

    list *next_ = this, *prev_ = this;
    std::size_t offset_;

    void add_element(list *new_element, list *prev, list *next) {
        next->prev_ = new_element;
        prev->next_ = new_element;
        new_element->next_ = next;
        new_element->prev_ = prev;
    }

    template <typename T, typename U>
    constexpr std::size_t offset_of(U T::*member) const {
        return reinterpret_cast<char *>(&(static_cast<T *>(nullptr)->*member)) - static_cast<char *>(nullptr);
    }

    Type *this_offset(int offset) {
        return reinterpret_cast<Type *>(reinterpret_cast<char *>(this) + offset);
    }

    list *list_member(Type *obj) {
        return reinterpret_cast<list *>(reinterpret_cast<char *>(obj) + offset_);
    }

public:

    class iterator {

        list *ptr = nullptr;

    public:

        explicit iterator(list *p) : ptr(p) {
        }

        iterator &operator++() {
            ptr = ptr->next_;
            return *this;
        }

        Type &operator *() {
            return *ptr->entry();
        }

        bool operator!=(const iterator &i) const {
            return i.ptr != ptr;
        }

    };

    template <typename U>
    explicit list(U Type::*member) {
        offset_ = offset_of(member);
    }

    void push_back(Type &new_element) {
        add_element(list_member(&new_element), prev_, this);
    }

    void remove() {
        prev_ = next_ = this;
    }

    bool empty() const {
        return prev_ == this;
    }

    Type *entry() {
        return this_offset(-offset_);
    }

    iterator begin() {
        return iterator(next_);
    }

    iterator end() {
        return iterator(this);
    }

    list *next() const {
        return next_;
    }

};

template <typename T>
class mock_return_value final {

    unsigned char data_[sizeof(T)];
    T *value_ = reinterpret_cast<T *>(data_);

public:

    void set(const T &val) {
        value_ = new(data_) T(val);
    }

    T &get() const {
        return *value_;
    }

};

template <>
class mock_return_value<void> {};

template <typename T>
class mock {};

template <typename R, typename ...Args>
class mock<R(Args...)> final {

    std::size_t nr_of_calls_ = 0;
    mock_return_value<R> default_return_value_;

public:

    template <typename T = R>
    typename std::enable_if<
        std::is_void<T>::value, T
    >::type operator()(Args ...) {
        ++nr_of_calls_;
    }

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, T
    >::type operator()(Args ...) {
        ++nr_of_calls_;
        return default_return_value_.get();
    }

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, void
    >::type set_default_return_value(const T &val) {
        default_return_value_.set(val);
    }

    std::size_t nr_of_calls() const {
        return nr_of_calls_;
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

    class test_case {

        list<test_case> node_;

    public:
        const char *suite_name;
        const char *test_name;
        std::size_t assertions = 0;
        std::size_t failed = 0;

        test_case() : node_(&test_case::node_) {
        }

        explicit test_case(const char *suite, const char *test)
                : node_(&test_case::node_), suite_name(suite), test_name(test) {
            test_session::get().register_test(this);
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

    char *find(char *str, char c) const {
        while (*str) {
            if (*str == c) {
                return str;
            }
            str++;
        }
        return nullptr;
    }

    void copy_string(const char *src, char *dest) const {
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
        tests_number_++;
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
            yatf::detail::printer_ << "assertion failed: " << __FILE__ << ':' << __LINE__ << " \'" << #cond << "\' is false\n"; \
    } while (0)

#define REQUIRE_FALSE(cond) \
    do { \
        if (!yatf::detail::test_session::get().current_test_case().assert_true(!(cond))) \
            yatf::detail::printer_ << "assertion failed: " << __FILE__ << ':' << __LINE__ << " \'" << #cond << "\' is true\n"; \
    } while (0)

#define REQUIRE_EQ(lhs, rhs) \
    do { \
        if (!yatf::detail::test_session::get().current_test_case().assert_eq(lhs, rhs)) { \
            yatf::detail::printer_ << "assertion failed: " << __FILE__ << ':' << __LINE__ << " \'" << #lhs << "\' isn't \'" << #rhs << "\': "; \
            yatf::detail::printer_ << lhs << " != " << rhs << "\n"; \
        } \
    } while (0)

#define YATF_CONCAT_(x,y) x##y
#define YATF_CONCAT(x,y) YATF_CONCAT_(x,y)

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

#define MOCK(signature, name) \
    ::yatf::detail::mock<signature> name;

#define REQUIRE_CALL(name) \
    do { \
    } while (0)

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

