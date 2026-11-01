#pragma once
#include <string>

class ParcelQueue {
private:
    struct Node {
        std::string id;
        Node*       next;
        explicit Node(const std::string& v) : id(v), next(nullptr) {}
    };
    Node* front;
    Node* rear;
    int   count;

public:
    ParcelQueue() : front(nullptr), rear(nullptr), count(0) {}
    ~ParcelQueue() { while (!isEmpty()) { std::string tmp; dequeue(tmp); } }
    ParcelQueue(const ParcelQueue&) = delete;
    ParcelQueue& operator=(const ParcelQueue&) = delete;

    bool isEmpty() const { return count == 0; }
    int  size() const { return count; }

    void enqueue(const std::string& id) {
        Node* n = new Node(id);
        if (rear) rear->next = n; else front = n;
        rear = n;
        ++count;
    }

    bool dequeue(std::string& out) {
        if (isEmpty()) return false;
        Node* t = front;
        out = t->id;
        front = front->next;
        if (!front) rear = nullptr;
        delete t;
        --count;
        return true;
    }

    bool peek(std::string& out) const {
        if (isEmpty()) return false;
        out = front->id;
        return true;
    }

    std::string* snapshot(int& n) const {
        n = count;
        if (count == 0) return nullptr;
        std::string* arr = new std::string[count];
        int i = 0;
        for (Node* c = front; c; c = c->next) arr[i++] = c->id;
        return arr;
    }
};
