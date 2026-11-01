#pragma once
#include <string>
using namespace std;

const int MAX_QUEUE = 20;

// Array-based circular queue of resource IDs.
// Used for round-robin allocation: the resource at the front is tried first,
// then moved to the back, so work is shared evenly between resources.
class CircularQueue {
private:
    string items[MAX_QUEUE];
    int front;   // index of first item
    int rear;    // index of last item
    int count;   // number of items

public:
    CircularQueue() {
        front = 0;
        rear = -1;
        count = 0;
    }

    bool isEmpty() { return count == 0; }
    bool isFull()  { return count == MAX_QUEUE; }
    int  size()    { return count; }

    bool enqueue(string id) {
        if (isFull()) return false;
        rear = (rear + 1) % MAX_QUEUE;   // wrap around to index 0 at the end
        items[rear] = id;
        count++;
        return true;
    }

    bool dequeue(string &id) {
        if (isEmpty()) return false;
        id = items[front];
        front = (front + 1) % MAX_QUEUE;
        count--;
        return true;
    }

    bool peek(string &id) {
        if (isEmpty()) return false;
        id = items[front];
        return true;
    }

    // Item at position i counted from the front (0 = front)
    string getAt(int i) {
        return items[(front + i) % MAX_QUEUE];
    }

    // Remove one ID from anywhere in the queue, keeping the order of the rest
    bool remove(string id) {
        bool found = false;
        int n = count;
        for (int i = 0; i < n; i++) {
            string current;
            dequeue(current);
            if (current == id && !found) found = true;   // drop it
            else enqueue(current);                       // put the rest back
        }
        return found;
    }
};
