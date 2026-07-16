#ifndef SJTU_LIST_HPP
#define SJTU_LIST_HPP

#include "exceptions.hpp"
#include "algorithm.hpp"

#include <climits>
#include <cstddef>
#include <utility>

namespace sjtu {

template<typename T>
class list {
protected:
    struct node {
        node *prev;
        node *next;
        bool is_sentinel;
        alignas(T) char data_buf[sizeof(T)];

        node(bool sentinel = false) : prev(nullptr), next(nullptr), is_sentinel(sentinel) {}

        node(const T& val, node* p = nullptr, node* n = nullptr)
            : prev(p), next(n), is_sentinel(false) {
            new (data_buf) T(val);
        }

        ~node() {
            if (!is_sentinel) {
                data_ptr()->~T();
            }
        }

        T* data_ptr() {
            return reinterpret_cast<T*>(data_buf);
        }

        const T* data_ptr() const {
            return reinterpret_cast<const T*>(data_buf);
        }

        T& data() {
            return *reinterpret_cast<T*>(data_buf);
        }

        const T& data() const {
            return *reinterpret_cast<const T*>(data_buf);
        }
    };

protected:
    node *head;
    node *tail;
    size_t _size;

    node *insert_node(node *pos, node *cur) {
        cur->prev = pos->prev;
        cur->next = pos;
        pos->prev->next = cur;
        pos->prev = cur;
        ++_size;
        return cur;
    }

    node *erase_node(node *pos) {
        pos->prev->next = pos->next;
        pos->next->prev = pos->prev;
        --_size;
        return pos;
    }

public:
    class const_iterator;
    class iterator {
    private:
        node *current;
        const list *container;

    public:
        iterator(node *n = nullptr, const list *c = nullptr)
            : current(n), container(c) {}

        iterator operator++(int) {
            if (current == nullptr || container == nullptr || current == container->tail || current == container->head)
                throw invalid_iterator();
            iterator tmp = *this;
            current = current->next;
            return tmp;
        }

        iterator & operator++() {
            if (current == nullptr || container == nullptr || current == container->tail || current == container->head)
                throw invalid_iterator();
            current = current->next;
            return *this;
        }

        iterator operator--(int) {
            if (current == nullptr || container == nullptr || current == container->tail)
                throw invalid_iterator();
            iterator tmp = *this;
            current = current->prev;
            return tmp;
        }

        iterator & operator--() {
            if (current == nullptr || container == nullptr || current == container->head)
                throw invalid_iterator();
            current = current->prev;
            return *this;
        }

        T & operator *() const {
            if (current == nullptr || container == nullptr || current == container->tail || current == container->head)
                throw invalid_iterator();
            return current->data();
        }

        T * operator ->() const {
            if (current == nullptr || container == nullptr || current == container->tail || current == container->head)
                throw invalid_iterator();
            return current->data_ptr();
        }

        bool operator==(const iterator &rhs) const {
            return current == rhs.current;
        }

        bool operator==(const const_iterator &rhs) const;

        bool operator!=(const iterator &rhs) const {
            return current != rhs.current;
        }

        bool operator!=(const const_iterator &rhs) const;

        friend class list;
        friend class const_iterator;
    };

    class const_iterator {
    private:
        const node *current;
        const list *container;

    public:
        const_iterator(const node *n = nullptr, const list *c = nullptr)
            : current(n), container(c) {}

        const_iterator(const iterator &other)
            : current(other.current), container(other.container) {}

        const_iterator operator++(int) {
            if (current == nullptr || container == nullptr || current == container->tail)
                throw invalid_iterator();
            const_iterator tmp = *this;
            current = current->next;
            return tmp;
        }

        const_iterator & operator++() {
            if (current == nullptr || container == nullptr || current == container->tail)
                throw invalid_iterator();
            current = current->next;
            return *this;
        }

        const_iterator operator--(int) {
            if (current == nullptr || container == nullptr || current == container->head)
                throw invalid_iterator();
            const_iterator tmp = *this;
            current = current->prev;
            return tmp;
        }

        const_iterator & operator--() {
            if (current == nullptr || container == nullptr || current == container->head)
                throw invalid_iterator();
            current = current->prev;
            return *this;
        }

        const T & operator *() const {
            if (current == nullptr || container == nullptr || current == container->tail || current == container->head)
                throw invalid_iterator();
            return current->data();
        }

        const T * operator ->() const {
            if (current == nullptr || container == nullptr || current == container->tail || current == container->head)
                throw invalid_iterator();
            return current->data_ptr();
        }

        bool operator==(const const_iterator &rhs) const {
            return current == rhs.current;
        }

        bool operator==(const iterator &rhs) const {
            return current == rhs.current;
        }

        bool operator!=(const const_iterator &rhs) const {
            return current != rhs.current;
        }

        bool operator!=(const iterator &rhs) const {
            return current != rhs.current;
        }

        friend class list;
        friend class iterator;
    };

    list() : _size(0) {
        head = new node(true);
        tail = new node(true);
        head->next = tail;
        tail->prev = head;
    }

    list(const list &other) : _size(0) {
        head = new node(true);
        tail = new node(true);
        head->next = tail;
        tail->prev = head;

        for (node *p = other.head->next; p != other.tail; p = p->next) {
            push_back(p->data());
        }
    }

    virtual ~list() {
        clear();
        delete head;
        delete tail;
    }

    list &operator=(const list &other) {
        if (this == &other)
            return *this;

        clear();
        for (node *p = other.head->next; p != other.tail; p = p->next) {
            push_back(p->data());
        }
        return *this;
    }

    const T & front() const {
        if (empty())
            throw container_is_empty();
        return head->next->data();
    }

    const T & back() const {
        if (empty())
            throw container_is_empty();
        return tail->prev->data();
    }

    iterator begin() {
        return iterator(head->next, this);
    }

    const_iterator cbegin() const {
        return const_iterator(head->next, this);
    }

    iterator end() {
        return iterator(tail, this);
    }

    const_iterator cend() const {
        return const_iterator(tail, this);
    }

    virtual bool empty() const {
        return _size == 0;
    }

    virtual size_t size() const {
        return _size;
    }

    virtual void clear() {
        while (!empty()) {
            pop_front();
        }
    }

    virtual iterator insert(iterator pos, const T &value) {
        if (pos.container != this)
            throw invalid_iterator();
        node *newNode = new node(value);
        insert_node(pos.current, newNode);
        return iterator(newNode, this);
    }

    virtual iterator erase(iterator pos) {
        if (pos.container != this || pos.current == nullptr || pos.current->is_sentinel)
            throw invalid_iterator();
        node *nextNode = pos.current->next;
        node *erased = erase_node(pos.current);
        delete erased;
        return iterator(nextNode, this);
    }

    void push_back(const T &value) {
        insert(end(), value);
    }

    void pop_back() {
        if (empty())
            throw container_is_empty();
        erase(iterator(tail->prev, this));
    }

    void push_front(const T &value) {
        insert(begin(), value);
    }

    void pop_front() {
        if (empty())
            throw container_is_empty();
        erase(begin());
    }

    void sort() {
        if (_size <= 1)
            return;

        char *buf = new char[_size * sizeof(T)];
        T *arr = reinterpret_cast<T*>(buf);
        size_t idx = 0;
        for (node *p = head->next; p != tail; p = p->next) {
            new (&arr[idx++]) T(p->data());
        }

        sjtu::sort<T>(arr, arr + _size, std::function<bool(const T&, const T&)>([](const T &a, const T &b) { return a < b; }));

        idx = 0;
        for (node *p = head->next; p != tail; p = p->next) {
            p->data() = arr[idx++];
        }

        for (size_t i = 0; i < _size; ++i) {
            arr[i].~T();
        }
        delete[] buf;
    }

    void merge(list &other) {
        if (this == &other)
            return;

        node *p1 = head->next;
        node *p2 = other.head->next;

        while (p1 != tail && p2 != other.tail) {
            if (p2->data() < p1->data()) {
                node *next2 = p2->next;
                p2->prev->next = p2->next;
                p2->next->prev = p2->prev;
                other._size--;

                p2->prev = p1->prev;
                p2->next = p1;
                p1->prev->next = p2;
                p1->prev = p2;
                _size++;

                p2 = next2;
            } else {
                p1 = p1->next;
            }
        }

        while (p2 != other.tail) {
            node *next2 = p2->next;
            p2->prev->next = p2->next;
            p2->next->prev = p2->prev;
            other._size--;

            p2->prev = tail->prev;
            p2->next = tail;
            tail->prev->next = p2;
            tail->prev = p2;
            _size++;

            p2 = next2;
        }

        other.head->next = other.tail;
        other.tail->prev = other.head;
    }

    void reverse() {
        if (_size <= 1)
            return;

        node *p = head->next;
        while (p != tail) {
            node *next = p->next;
            std::swap(p->prev, p->next);
            p = next;
        }

        std::swap(head->next, tail->prev);
        head->next->prev = head;
        tail->prev->next = tail;
    }

    void unique() {
        if (_size <= 1)
            return;

        node *p = head->next;
        while (p != tail && p->next != tail) {
            if (p->data() == p->next->data()) {
                node *toDelete = p->next;
                toDelete->prev->next = toDelete->next;
                toDelete->next->prev = toDelete->prev;
                --_size;
                delete toDelete;
            } else {
                p = p->next;
            }
        }
    }
};

template<typename T>
bool list<T>::iterator::operator==(const const_iterator &rhs) const {
    return current == rhs.current;
}

template<typename T>
bool list<T>::iterator::operator!=(const const_iterator &rhs) const {
    return current != rhs.current;
}

}

#endif
