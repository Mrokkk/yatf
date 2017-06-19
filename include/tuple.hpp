#pragma once

#include <type_traits>

namespace yatf {

struct any_value {};

extern any_value _;

namespace detail {

template <typename T>
struct matcher {
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

    T value_;
    bool (*matcher_)(const T &) = nullptr;
    char data_[3 * sizeof(matcher<T>) + 2 * sizeof(T)];
    matcher<T> *m_ = nullptr;

public:

    explicit argument(const T &val) : value_(val) {
    }

    explicit argument(any_value) : matcher_([](const T &) {
            return true;
        }) {
    }

    template <typename Matcher>
    explicit argument(const Matcher &m) : m_(new(data_) Matcher(m)) {
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

    explicit field_matcher(U T::*member, const U &value)
        : offset_(offset_of(member)), value_(new(data_) U(value)) {
    }

    bool match(const T &s) override {
        return *reinterpret_cast<const U *>(reinterpret_cast<const char *>(&s) + offset_) == *value_;
    }

private:

    constexpr std::size_t offset_of(U T::*member) const {
        return reinterpret_cast<char *>(&(static_cast<T *>(nullptr)->*member))
            - static_cast<char *>(nullptr);
    }

    std::size_t offset_;
    char data_[sizeof(U)];
    U *value_ = nullptr;
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

} // namespace detail

template <typename ...T>
struct arguments final : public detail::arguments_impl<typename detail::range<sizeof...(T)>::type, T...> {

    template <typename ...Args>
    explicit arguments(const Args &...values)
        : detail::arguments_impl<typename detail::range<sizeof...(T)>::type, T...>(values...) {
    }

    static constexpr std::size_t size() {
        return sizeof...(T);
    }

};

template <>
struct arguments<> final {
    bool compare() const {
        return true;
    }
};

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

MATCHER(eq, n) {
    return arg == n;
}

MATCHER(ne, n) {
    return arg != n;
}

MATCHER(ge, n) {
    return n >= arg;
}

MATCHER(gt, n) {
    return n > arg;
}

MATCHER(le, n) {
    return n <= arg;
}

MATCHER(lt, n) {
    return n < arg;
}

template <typename T, typename U>
inline detail::field_matcher<T, U> field(U T::*member, const U &val) {
    return detail::field_matcher<T, U>(member, val);
}

} // namespace yatf

