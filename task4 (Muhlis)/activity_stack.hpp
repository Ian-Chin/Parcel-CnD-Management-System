#pragma once

// Linked-list stack (LIFO). Written for Task 4 instead of std::stack.
// The top node is always the most recently pushed item.
// push / pop / peek / isEmpty / size are all O(1).
template <typename T>
class Stack {
private:
    struct Node {
        T     data;
        Node* next;
        Node(const T& d, Node* n) : data(d), next(n) {}
    };
    Node* top;
    int   count;

public:
    Stack() : top(nullptr), count(0) {}
    ~Stack() { clear(); }
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    bool isEmpty() const { return top == nullptr; }
    int  size() const { return count; }

    void push(const T& item) {
        top = new Node(item, top);   // new node points to the old top
        ++count;
    }

    bool pop(T& out) {
        if (isEmpty()) return false;
        Node* old = top;
        out = old->data;
        top = old->next;
        delete old;
        --count;
        return true;
    }

    bool peek(T& out) const {
        if (isEmpty()) return false;
        out = top->data;
        return true;
    }

    void clear() {
        T dummy;
        while (pop(dummy)) {}
    }
};
