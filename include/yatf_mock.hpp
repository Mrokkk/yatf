#pragma once

#include <type_traits>
#include "list.hpp"
#include "printer.hpp"

namespace yatf {

namespace detail {

template <typename T>
class mock_handler final {

    unsigned char data_[sizeof(T)];
    T *value_ = reinterpret_cast<T *>(data_);
    std::size_t expected_nr_of_calls_ = 0;
    std::size_t actual_nr_of_calls_ = 0;

public:

    ~mock_handler() {
        if (expected_nr_of_calls_ != actual_nr_of_calls_) {
            // TODO
        }
    }

    void set(const T &val) {
        value_ = new(data_) T(val);
    }

    T &get() const {
        return *value_;
    }

    T &operator()() {
        ++actual_nr_of_calls_;
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
        // TODO
    }

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, T
    >::type operator()(Args ...) {
        // TODO
        return default_handler_.get();
    }

    template <typename T = R>
    typename std::enable_if<
        !std::is_void<T>::value, void
    >::type set_default_return_value(const T &val) {
        default_handler_.set(val);
    }

};

} // namespace detail

#define MOCK(signature, name) \
    yatf::detail::mock<signature> name;

#define REQUIRE_CALL(name) \
    auto YATF_UNIQUE_NAME(__mock_handler) = name.get_handler(); temp_mock = (void *)&YATF_UNIQUE_NAME(__mock_handler); \
    name.register_handler(temp_mock); \
    name.cast_handler(temp_mock)->expect_call()

} // namespace yatf

