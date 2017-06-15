#pragma once

namespace yatf {

namespace detail {

template <typename ...Types>
struct size_of_pack;

template <>
struct size_of_pack<> {
    static const auto value = 0;
};

template <typename T>
struct size_of_pack<T> {
    static const auto value = sizeof(T);
};

template <typename T, typename ...Rest>
struct size_of_pack<T, Rest...> {
    static const auto value = sizeof(T) + size_of_pack<Rest...>::value;
};

} // namespace detail

} // namespace yatf
