#pragma once

#include <type_traits>

namespace yatf {

using printf_t = int (*)(const char *, ...);

namespace detail {

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

} // namespace detail

} // namespace yatf

