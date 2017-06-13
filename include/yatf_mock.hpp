#pragma once

#include <type_traits>
#include "list.hpp"
#include "printer.hpp"

namespace yatf {

namespace detail {

template <typename T>
class return_value final {
    unsigned char data_[sizeof(T)];
    T *value_ = reinterpret_cast<T *>(data_);
public:
    void set(const T &v) {
        value_ = new(data_) T(v);
    }
    T &get() const {
        return *value_;
    }
};

template <>
class return_value<void> final {
};

template <typename R, typename ...Args>
class mock_handler final {

    void (*scheduled_assert_)(std::size_t, std::size_t) = nullptr;
    bool (*matcher_)(Args ...) = nullptr;
    std::size_t expected_nr_of_calls_ = 0;
    std::size_t actual_nr_of_calls_ = 0;
    return_value<R> return_value_;

public:

    ~mock_handler() {
        if (scheduled_assert_) {
            scheduled_assert_(expected_nr_of_calls_, actual_nr_of_calls_);
        }
    }

    mock_handler &match_args(bool (*matcher)(Args ...)) {
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

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, T &
    >::type get_return_value() const {
        return return_value_.get();
    }

    void schedule_assertion(void (*l)(std::size_t, std::size_t)) {
        scheduled_assert_ = l;
    }

    bool operator()(const Args &...args) {
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

    mock_handler &expect_call(std::size_t nr = 1) {
        expected_nr_of_calls_ = nr;
        return *this;
    }

    typename list<mock_handler>::node node;

};

template <typename T>
class mock final {};

template <typename R, typename ...Args>
class mock<R(Args...)> final {

    mock_handler<R, Args...> default_handler_;
    list<mock_handler<R, Args...>> handlers_;

public:

    mock() : handlers_(&mock_handler<R, Args...>::node) {
    }

    void register_handler(void *handler) {
        handlers_.push_back(*reinterpret_cast<mock_handler<R, Args...> *>(handler));
    }

    mock_handler<R, Args...> get_handler() {
        return {};
    }

    mock_handler<R, Args...> *cast_handler(void *h) const {
        return reinterpret_cast<mock_handler<R, Args...> *>(h);
    }

    template <typename T = R>
    typename std::enable_if<
        std::is_void<T>::value, T
    >::type operator()(Args ...args) {
        for (auto it = handlers_.begin(); it != handlers_.end(); ++it) {
            (*it)(args...);
        }
    }

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, T
    >::type operator()(Args ...args) {
        for (auto it = handlers_.begin(); it != handlers_.end(); ++it) {
            if ((*it)(args...)) {
                return it->get_return_value();
            }
        }
        return default_handler_.get_return_value();
    }

};

} // namespace detail

#define MOCK(signature, name) \
    yatf::detail::mock<signature> name;

#define REQUIRE_CALL(name) \
    auto YATF_UNIQUE_NAME(__mock_handler) = name.get_handler(); temp_mock = (void *)&YATF_UNIQUE_NAME(__mock_handler); \
    name.register_handler(temp_mock); \
    name.cast_handler(temp_mock)->schedule_assertion([](std::size_t expected, std::size_t actual) { \
        if (!yatf::detail::test_session::get().current_test_case().assert_eq(expected, actual)) { \
            yatf::detail::printer_ << "assertion failed: " << __FILE__ << ':' << __LINE__ << " " << #name \
                                   << ": expected to be called: " << expected << "; actual: " << actual << "\n"; \
        } \
    }); \
    name.cast_handler(temp_mock)->expect_call()

} // namespace yatf

