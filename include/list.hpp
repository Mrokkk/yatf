#pragma once

#include <stddef.h>

namespace yatf {

namespace detail {

template <typename Type>
struct list final {

    class node {

        node *next_ = this, *prev_ = this;
        size_t offset_ = 0;

        Type *this_offset(int offset) {
            return reinterpret_cast<Type *>(reinterpret_cast<char *>(this) + offset);
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
            return this_offset(-offset_);
        }

        void set_offset(size_t offset) {
            offset_ = offset;
        }

    };

    using value_type = Type;
    using node_type = node;

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

        node *ptr() {
            return ptr_;
        }

        const node *ptr() const {
            return ptr_;
        }

    };

private:

    node head_;
    size_t offset_;

    void add_node(node *new_node, node *prev, node *next) {
        new_node->set_offset(offset_);
        next->prev() = new_node;
        prev->next() = new_node;
        new_node->next() = next;
        new_node->prev() = prev;
    }

    void remove_node(node *n) {
        n->next()->prev() = n->prev();
        n->prev()->next() = n->next();
        n->prev() = n->next() = n;
    }

    template <typename T, typename U>
    size_t offset_of(U T::*member) const {
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

    list &erase(const iterator &it) {
        remove_node(const_cast<node *>(it.ptr()));
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

    list &clear() {
        while (not empty()) {
            erase(begin());
        }
        return *this;
    }

};

} // namespace detail

} // namespace yatf

