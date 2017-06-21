#pragma once

#include <type_traits>

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

using printf_t = int (*)(const char *, ...);

namespace detail {

struct empty_fixture {};

inline int compare_strings(const char *s1, const char *s2) {
    while(*s1 && (*s1 == *s2)) {
        ++s1, ++s2;
    }
    return *s1 - *s2;
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

extern printf_t printf_;

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
            default:
                break;
        }
        return *this;
    }

    printer &operator<<(cursor_movement c) {
        switch (c) {
            case cursor_movement::up:
                printf_("\033[1A");
                break;
            default:
                break;
        }
        return *this;
    }

};

extern printer printer_;

template <typename Type>
struct list final {

    class node {

        node *next_ = this, *prev_ = this;
        std::size_t offset_ = 0;

        Type *this_offset() {
            return reinterpret_cast<Type *>(reinterpret_cast<char *>(this) - offset_);
        }

    public:

        ~node() {
            if (next_ != this) next()->prev() = prev();
            if (prev_ != this) prev()->next() = next();
        }

        node *&next() {
            return next_;
        }

        node *&prev() {
            return prev_;
        }

        const node *prev() const {
            return prev_;
        }

        Type *entry() {
            return this_offset();
        }

        void set_offset(std::size_t offset) {
            offset_ = offset;
        }

    };

    class iterator final {

        node *ptr_ = nullptr;

    public:

        explicit iterator(node *n) : ptr_(n) {
        }

        iterator(const iterator &it) : ptr_(it.ptr_) {
        }

        iterator &operator++() {
            ptr_ = ptr_->next();
            return *this;
        }

        Type &operator*() {
            return *ptr_->entry();
        }

        Type *operator->() {
            return ptr_->entry();
        }

        bool operator!=(const iterator &it) {
            return it.ptr_ != ptr_;
        }

    };

private:

    node head_;
    std::size_t offset_;

    void add_node(node *new_node, node *prev, node *next) {
        new_node->set_offset(offset_);
        next->prev() = prev->next() = new_node;
        new_node->next() = next;
        new_node->prev() = prev;
    }

    template <typename T, typename U>
    std::size_t offset_of(U T::*member) const {
        return reinterpret_cast<char *>(&(static_cast<T *>(nullptr)->*member)) - static_cast<char *>(nullptr);
    }

    node *list_member(Type *obj) {
        return reinterpret_cast<node *>(reinterpret_cast<char *>(obj) + offset_);
    }

public:

    using const_iterator = iterator;

    template <typename U>
    explicit list(U Type::*member) {
        offset_ = offset_of(member);
    }

    list &push_back(Type &new_node) {
        add_node(list_member(&new_node), head_.prev(), &head_);
        return *this;
    }

    bool empty() const {
        return head_.prev() == &head_;
    }

    iterator begin() {
        return iterator(head_.next());
    }

    iterator end() {
        return iterator(&head_);
    }

};

struct test_session final {

    struct messages final {

        enum class msg { start_end, run, pass, fail };

        static const char *get(msg m) {
            static const char *run_messages_[4] = {"[========]",  "[  RUN   ]", "[  PASS  ]", "[  FAIL  ]"};
            return run_messages_[static_cast<int>(m)];
        }

    };

    class test_case {

        list<test_case>::node node_;
        std::size_t assertions_ = 0;
        std::size_t failed_ = 0;

    protected:

        const char *suite_name;
        const char *test_name;

    public:

        void require_true(bool condition, const char *condition_str, const char *file, int line) {
            ++assertions_;
            if (!condition) {
                ++failed_;
                printer_ << "assertion failed: " << file << ':' << line << " \'" << condition_str << "\' is false\n";
            }
        }

        void require_false(bool condition, const char *condition_str, const char *file, int line) {
            ++assertions_;
            if (condition) {
                ++failed_;
                printer_ << "assertion failed: " << file << ':' << line << " \'" << condition_str << "\' is true\n";
            }
        }

        template <typename T1, typename T2>
        void require_eq(const T1 &lhs, const T2 &rhs, const char *lhs_str, const char *rhs_str,
                const char *file, int line) {
            ++assertions_;
            bool cond = (lhs == rhs);
            if (!cond) {
                ++failed_;
                printer_ << "assertion failed: " << file << ':' << line << " \'" << lhs_str
                         << "\' isn't \'" << rhs_str << "\': " << lhs << " != " << rhs << "\n";
            }
        }

        void require_eq(const char *lhs, const char *rhs, const char *, const char *, const char *file, int line) {
            ++assertions_;
            bool cond = compare_strings(lhs, rhs) == 0;
            if (!cond) {
                ++failed_;
                printer_ << "assertion failed: " << file << ':' << line << " \'" << lhs
                         << "\' isn't \'" << rhs << "\n";
            }
        }

        void require_call(const char *mock_name, std::size_t expected_nr_of_calls,
                std::size_t actual_nr_of_calls, const char *file, int line) {
            ++assertions_;
            if (expected_nr_of_calls != actual_nr_of_calls) {
                ++failed_;
                printer_ << "assertion failed: " << file << ':' << line << " " << mock_name
                         << ": expected to be called: " << expected_nr_of_calls << "; actual: "
                         << actual_nr_of_calls << "\n";
            }
        }

        virtual void test_body() = 0;

        friend test_session;
        friend yatf_fixture;

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
        if (t.failed_) {
            print_in_color(messages::get(messages::msg::fail), printer::color::red);
            printer_ << " " << t.suite_name << "." << t.test_name << " (" << static_cast<int>(t.assertions_) << " assertions)\n";
        }
        else {
            if (config_.fails_only) return;
            if (config_.oneliners)
                printer_ << printer::cursor_movement::up;
            print_in_color(messages::get(messages::msg::pass), printer::color::green);
            printer_ << " " << t.suite_name << "." << t.test_name << " (" << static_cast<int>(t.assertions_) << " assertions)\n";
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
                auto result = test.failed_;
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
            if (test.failed_) {
                ++failed;
            }
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

namespace helpers {

template <typename T>
inline constexpr T &&forward(typename std::remove_reference<T>::type &v) noexcept {
    return static_cast<T &&>(v);
}

template<typename T>
inline constexpr T &&forward(typename std::remove_reference<T>::type &&v) noexcept {
    static_assert(!std::is_lvalue_reference<T>::value, "template argument"
        " substituting T is an lvalue reference type");
    return static_cast<T &&>(v);
}

template <typename T>
inline constexpr typename std::remove_reference<T>::type&& move(T &&v) noexcept {
    return static_cast<typename std::remove_reference<T>::type &&>(v);
}

} // namespace helpers

template <typename T, std::size_t Size = 0>
class unary_container final {

    unsigned char data_[Size == 0 ? sizeof(T) : Size];
    T *value_ = nullptr;

public:

    constexpr unary_container() : data_() {
    }

    constexpr explicit unary_container(const T &val) : value_(new(data_) T(val)) {
    }

    ~unary_container() {
        if (value_) {
            value_->~T();
        }
    }

    template <typename U>
    void set_by_other_type(const U &v) {
        value_ = new(data_) U(v);
    }

    template <typename ...Args>
    void set(const Args &...v) {
        value_ = new(data_) T(v...);
    }

    T &get() const {
        if (value_ == nullptr) {
            return *reinterpret_cast<T *>(const_cast<unsigned char *>(data_));
        }
        return *value_;
    }

    T *operator->() {
        if (value_ == nullptr) {
            return reinterpret_cast<T *>(const_cast<unsigned char *>(data_));
        }
        return value_;
    }

    operator bool() const {
        return value_ != nullptr;
    }

    bool operator==(const T &rhs) const {
        if (value_) {
            return *value_ == rhs;
        }
        return false;
    }

};

template <>
class unary_container<void> final {
};

struct any_value final {};

template <typename T>
struct matcher {
    virtual ~matcher() {}
    virtual bool match(const T &lhs) = 0;
};

template <std::size_t ...N>
struct expand {
    using type = expand<N...>;
};

template <std::size_t N, typename ...T>
struct choose_nth {
};

template <std::size_t N, typename T, typename ...U>
struct choose_nth<N, T, U...> : choose_nth<N - 1, U...> {
};

template <typename T, typename ...U>
struct choose_nth<0, T, U...> {
    using type = T;
};

template <std::size_t N, typename T>
class argument {

    bool (*matcher_)(const T &) = nullptr;
    unary_container<T> value_;
    unary_container<matcher<T>, 3 * sizeof(matcher<T>) + 2 * sizeof(T)> m_;

public:

    constexpr explicit argument(const T &val) : value_(val) {
    }

    constexpr explicit argument(any_value) : matcher_([](const T &) { return true; }) {
    }

    template <typename Matcher>
    explicit argument(const Matcher &m) {
        m_.set_by_other_type(m);
    }

    bool match(const T &v) {
        if (matcher_) {
            return matcher_(v);
        }
        if (m_) {
            return m_->match(v);
        }
        return value_ == v;
    }

};

template <typename T, typename U>
struct field_matcher : public matcher<T> {

    constexpr explicit field_matcher(U T::*member, const U &value)
            : offset_(offset_of(member)), value_(value) {
    }

    bool match(const T &s) override {
        return *reinterpret_cast<const U *>(reinterpret_cast<const char *>(&s) + offset_) == value_.get();
    }

private:

    constexpr std::size_t offset_of(U T::*member) const {
        return reinterpret_cast<char *>(&(static_cast<T *>(nullptr)->*member))
            - static_cast<char *>(nullptr);
    }

    std::size_t offset_;
    unary_container<U> value_;

};

template <std::size_t L, std::size_t I = 0, typename S = expand<>>
struct range {
};

template <std::size_t L, std::size_t I, std::size_t ...N>
struct range<L, I, expand<N...>> : range<L, I + 1, expand<N..., I>> {
};

template <std::size_t L, std::size_t ...N>
struct range<L, L, expand<N...>> : expand<N...> {
};

template <typename N, typename ...T>
class arguments_impl {
};

template <std::size_t ...N, typename ...T>
struct arguments_impl<expand<N...>, T...> : public argument<N, T>... {

    template <std::size_t M> using value_type = typename choose_nth<M, T...>::type;

    template <typename ...Args>
    explicit arguments_impl(const Args &...values) : argument<N, T>(values)... {
    }

    template <std::size_t M>
    bool compare(const value_type<M> &val) {
        return argument<M, value_type<M>>::match(val);
    }

    template <typename U, typename ...Args, std::size_t M = 0>
    bool compare(const U &first, const Args &...args) {
        return compare<M>(first) && compare<M + 1>(args...);
    }

    template <typename U, std::size_t M = 0>
    bool compare(const U &first) {
        return compare<M>(first);
    }

};

template <typename ...T>
struct arguments final : public arguments_impl<typename range<sizeof...(T)>::type, T...> {

    template <typename ...Args>
    explicit arguments(const Args &...values)
        : arguments_impl<typename range<sizeof...(T)>::type, T...>(values...) {
    }

};

template <>
struct arguments<> final {
    bool compare() const {
        return true;
    }
};

template <typename ...Args>
struct is_empty {
    constexpr static const bool value = sizeof...(Args) == 0;
};

template <typename T>
class mock;

template <typename R, typename ...Args>
class mock_handler final {

    void (*scheduled_assert_)(std::size_t, std::size_t) = nullptr;
    bool (*matcher_)(Args ...) = nullptr;
    std::size_t expected_nr_of_calls_ = 1;
    std::size_t actual_nr_of_calls_ = 0;
    unary_container<R> return_value_;
    unary_container<arguments<Args...>> arguments_;
    typename list<mock_handler>::node node_;

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, T &
    >::type get_return_value() const {
        return return_value_.get();
    }

    template <typename T = bool>
    typename std::enable_if<
        is_empty<Args...>::value, T
    >::type operator()() {
        ++actual_nr_of_calls_;
        return true;
    }

    template <typename T = bool>
    typename std::enable_if<
        !is_empty<Args...>::value, T
    >::type operator()(const Args &...args) {
        if (arguments_) {
            bool is_matched = arguments_->compare(args...);
            if (is_matched) {
                ++actual_nr_of_calls_;
            }
            return is_matched;
        }
        if (matcher_) {
            bool is_matched = matcher_(args...);
            if (is_matched) {
                ++actual_nr_of_calls_;
            }
            return is_matched;
        }
        ++actual_nr_of_calls_;
        return true;
    }

public:

    ~mock_handler() {
        if (scheduled_assert_) {
            scheduled_assert_(expected_nr_of_calls_, actual_nr_of_calls_);
        }
    }

    template <typename T = mock_handler &>
    typename std::enable_if<
        !is_empty<Args...>::value, T
    >::type match_args(bool (*matcher)(Args ...)) {
        matcher_ = matcher;
        return *this;
    }

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, mock_handler &
    >::type will_return(const T &val) {
        return_value_.set(val);
        return *this;
    }

    template <typename U = mock_handler &, typename ...T>
    typename std::enable_if<
        !is_empty<T...>::value, U
    >::type for_arguments(T ...args) {
        arguments_.set(args...);
        return *this;
    }

    void schedule_assertion(void (*l)(std::size_t, std::size_t)) {
        scheduled_assert_ = l;
    }

    mock_handler &times(std::size_t nr = 1) {
        expected_nr_of_calls_ = nr;
        return *this;
    }

    friend mock<R(Args...)>;

};

template <typename T>
class mock final {};

template <typename R, typename ...Args>
class mock<R(Args...)> final {

    unary_container<R> default_return_value_;
    list<mock_handler<R, Args...>> handlers_;

public:

    mock() : handlers_(&mock_handler<R, Args...>::node_) {
    }

    void register_handler(mock_handler<R, Args...> &handler) {
        handlers_.push_back(handler);
    }

    mock_handler<R, Args...> get_handler() const {
        return {};
    }

    template <typename T = R>
    typename std::enable_if<
        std::is_void<T>::value, T
    >::type operator()(Args ...args) {
        for (auto it = handlers_.begin(); it != handlers_.end(); ++it) {
            (*it)(helpers::forward<Args>(args)...);
        }
    }

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, T
    >::type operator()(Args ...args) {
        for (auto it = handlers_.begin(); it != handlers_.end(); ++it) {
            if ((*it)(helpers::forward<Args>(args)...)) {
                return it->get_return_value();
            }
        }
        return default_return_value_.get();
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

#define MOCK(signature, name) \
    yatf::detail::mock<signature> name

#define REQUIRE_CALL(name) \
    auto YATF_UNIQUE_NAME(__mock_handler) = name.get_handler(); \
    name.register_handler(YATF_UNIQUE_NAME(__mock_handler)); \
    YATF_UNIQUE_NAME(__mock_handler).schedule_assertion([](std::size_t expected, std::size_t actual) { \
        yatf::detail::test_session::get().current_test_case().require_call(#name, expected, actual, __FILE__, __LINE__); \
    }); \
    (void)YATF_UNIQUE_NAME(__mock_handler)

#define MATCHER(name, lhs) \
    template <typename T> \
    struct name##_matcher : public yatf::detail::matcher<T> { \
        explicit name##_matcher(const T &val) : arg(val) { \
        } \
        bool match(const T &lhs) override; \
    private: \
        const T arg; \
    }; \
    template <typename T> \
    inline name##_matcher<T> name(const T &v) { \
        return name##_matcher<T>(v); \
    } \
    template <typename T> \
    bool name##_matcher<T>::match(const T &lhs)

MATCHER(eq, n) { return arg == n; }
MATCHER(ne, n) { return arg != n; }
MATCHER(ge, n) { return n >= arg; }
MATCHER(gt, n) { return n > arg; }
MATCHER(le, n) { return n <= arg; }
MATCHER(lt, n) { return n < arg; }

template <typename T, typename U>
inline detail::field_matcher<T, U> field(U T::*member, const U &val) {
    return detail::field_matcher<T, U>(member, val);
}

extern detail::any_value _;

#ifdef YATF_MAIN

namespace detail {

test_session test_session::instance_;
printer printer_;
printf_t printf_;

} // namespace detail

detail::any_value _;

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

