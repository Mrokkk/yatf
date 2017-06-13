#pragma once

#include <type_traits>
#include "list.hpp"
#include "printer.hpp"

namespace yatf {

namespace detail {

template <typename T>
class mock_handler final {

    void (*scheduled_assert_)(std::size_t, std::size_t) = nullptr;
    unsigned char data_[sizeof(T)];
    T *value_ = reinterpret_cast<T *>(data_);
    std::size_t expected_nr_of_calls_ = 0;
    std::size_t actual_nr_of_calls_ = 0;

public:

    ~mock_handler() {
        if (scheduled_assert_) {
            scheduled_assert_(expected_nr_of_calls_, actual_nr_of_calls_);
        }
    }

    void returns(const T &val) {
        value_ = new(data_) T(val);
    }

    T &get_return_value() const {
        return *value_;
    }

    void set_assertion(void (*l)(std::size_t, std::size_t)) {
        scheduled_assert_ = l;
    }

    bool operator()() {
        ++actual_nr_of_calls_;
        return true;
    }

    mock_handler &expect_call(std::size_t nr = 1) {
        expected_nr_of_calls_ = nr;
        return *this;
    }

    typename list<mock_handler>::node node;

};

template <>
class mock_handler<void> {
public:
    list<mock_handler>::node node;
};

template <typename T>
class mock {};

template <typename R, typename ...Args>
class mock<R(Args...)> final {

    mock_handler<R> default_handler_;
    list<mock_handler<R>> handlers_;

public:

    mock() : handlers_(&mock_handler<R>::node) {
    }

    void register_handler(void *handler) {
        handlers_.push_back(*reinterpret_cast<mock_handler<R> *>(handler));
    }

    mock_handler<R> get_handler() {
        return {};
    }

    mock_handler<R> *cast_handler(void *h) const {
        return reinterpret_cast<mock_handler<R> *>(h);
    }

    template <typename T = R>
    typename std::enable_if<
        std::is_void<T>::value, T
    >::type operator()(Args ...) {
        for (auto it = handlers_.begin(); it != handlers_.end(); ++it) {
            if ((*it)()) {
                return it->get_return_value();
            }
        }
    }

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, T
    >::type operator()(Args ...) {
        for (auto it = handlers_.begin(); it != handlers_.end(); ++it) {
            if ((*it)()) {
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
    name.cast_handler(temp_mock)->set_assertion([](std::size_t expected, std::size_t actual) { \
        if (!yatf::detail::test_session::get().current_test_case().assert_eq(expected, actual)) { \
            yatf::detail::printer_ << "assertion failed: " << __FILE__ << ':' << __LINE__ << " " << #name << ": expected to be called: " << expected << "; actual: " << actual << "\n"; \
        } \
    }); \
    name.cast_handler(temp_mock)->expect_call()

} // namespace yatf

