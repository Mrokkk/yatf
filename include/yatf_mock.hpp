#pragma once

#include <type_traits>

namespace yatf {

namespace detail {

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

struct scoped_function {

    using function = void(*)();

private:

    function function_ = nullptr;

public:

    scoped_function() = default;

    scoped_function(function fn) : function_(fn) {
    }

    ~scoped_function() {
        if (function_) {
            function_();
        }
    }

};

} // namespace yatf

} // namespace detail

