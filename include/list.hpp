#pragma once

#include <type_traits>

namespace yatf {

namespace detail {

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

} // namespace detail

} // namespace yatf

