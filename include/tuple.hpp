#pragma once

#include <type_traits>

namespace detail {

struct any_value {};

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
class tuple_element {
    const T value_;
    bool match_all_ = false;
public:
    explicit tuple_element(const T &val) : value_(val) {
    }
    explicit tuple_element(any_value) : match_all_(true) {
    }
    const T &get() const {
        return value_;
    }
    bool operator==(const T &v) {
        if (match_all_) {
            return true;
        }
        return value_ == v;
    }
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
class tuple_impl {
};

template <std::size_t ...N, typename ...T>
struct tuple_impl<expand<N...>, T...> : public tuple_element<N, T>... {

    template <std::size_t M> using value_type = typename choose_nth<M, T...>::type;

    explicit tuple_impl(const T &...values) : tuple_element<N, T>(values)... {
    }

    template <std::size_t M>
    const value_type<M> &get() const {
        return tuple_element<M, value_type<M>>::get();
    }

    template <std::size_t M>
    bool compare(value_type<M> val) {
        return get<M>() == val;
    }

    template <typename U, typename ...Args, std::size_t M = 0>
    bool compare(U first, Args ...args) {
        return compare<M>(first) && compare<M + 1>(args...);
    }

};

} // namespace detail

template <typename ...T>
struct tuple : public detail::tuple_impl<typename detail::range<sizeof...(T)>::type, T...> {

    explicit tuple(const T &...values)
        : detail::tuple_impl<typename detail::range<sizeof...(T)>::type, T...>(values...) {
    }

    static constexpr std::size_t size() {
        return sizeof...(T);
    }

};

template <>
struct tuple<> {
    bool compare() {
        return true;
    }
};

