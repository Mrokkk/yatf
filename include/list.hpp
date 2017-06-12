#pragma once

#include <type_traits>

namespace yatf {

namespace detail {

template <typename Type>
struct list final {

    class node {

        node *next_ = this, *prev_ = this;
        std::size_t offset_ = 0;

        Type *this_offset(int offset) {
            return reinterpret_cast<Type *>(reinterpret_cast<char *>(this) + offset);
        }

        const Type *this_offset(int offset) const {
            return reinterpret_cast<const Type *>(reinterpret_cast<const char *>(this) + offset);
        }

    public:

        node *&next() {
            return next_;
        }

        node *&prev() {
            return prev_;
        }

        const node *next() const {
            return next_;
        }

        const node *prev() const {
            return prev_;
        }

        Type *entry() {
            return this_offset(-offset_);
        }

        const Type *entry() const {
            return this_offset(-offset_);
        }

        void set_offset(std::size_t offset) {
            offset_ = offset;
        }

        void insert(node *new_node) {
            new_node->set_offset(offset_);
            next_->prev() = new_node;
            prev_->next() = new_node;
            new_node->next() = next_;
            new_node->prev() = prev_;
        }

    };

    using value_type = Type;
    using node_type = node;

    class iterator {

        node *ptr_ = nullptr;

    public:

        explicit iterator(node *p) : ptr_(p) {
        }

        iterator &operator++() {
            ptr_ = ptr_->next();
            return *this;
        }

        Type &operator *() {
            return *ptr_->entry();
        }

        bool operator!=(const iterator &i) const {
            return i.ptr_ != ptr_;
        }

        node *ptr() {
            return ptr_;
        }

    };
private:

    node head_;
    std::size_t offset_;

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
    constexpr std::size_t offset_of(U T::*member) const {
        return reinterpret_cast<char *>(&(static_cast<T *>(nullptr)->*member)) - static_cast<char *>(nullptr);
    }

    node *list_member(Type *obj) {
        return reinterpret_cast<node *>(reinterpret_cast<char *>(obj) + offset_);
    }

public:

    template <typename U>
    constexpr explicit list(U Type::*member) {
        offset_ = offset_of(member);
    }

    list &push_back(Type &new_node) {
        add_node(list_member(&new_node), head_.prev(), &head_);
        return *this;
    }

    list &insert(const iterator &pos, Type &new_node) {
        add_node(list_member(&new_node), pos.node()->prev(), pos.node());
        return *this;
    }

    list &insert(Type &pos, Type &new_node) {
        auto node_member = list_member(&pos);
        add_node(list_member(&new_node), node_member->prev(), node_member);
        return *this;
    }

    list &erase(const iterator &it) {
        remove_node(it.ptr());
        return *this;
    }

    list &erase(Type &n) {
        remove_node(list_member(&n));
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

    void clear() {
        while (not empty()) {
            remove_node(head_.next());
        }
    }

};

} // namespace detail

} // namespace yatf

