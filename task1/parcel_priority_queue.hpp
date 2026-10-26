#pragma once
#include <string>

struct PQEntry {
    int         priority;
    long        seq;
    std::string parcelId;
};

class ParcelPriorityQueue {
private:
    PQEntry* heap;
    int      capacity;
    int      count;

    static bool higher(const PQEntry& a, const PQEntry& b) {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.seq < b.seq;
    }
    static void siftDownArray(PQEntry* a, int n, int i) {
        while (true) {
            int l = 2 * i + 1, r = 2 * i + 2, best = i;
            if (l < n && higher(a[l], a[best])) best = l;
            if (r < n && higher(a[r], a[best])) best = r;
            if (best == i) break;
            PQEntry t = a[i]; a[i] = a[best]; a[best] = t;
            i = best;
        }
    }
    void grow() {
        int newCap = capacity * 2;
        PQEntry* nh = new PQEntry[newCap];
        for (int i = 0; i < count; ++i) nh[i] = heap[i];
        delete[] heap;
        heap = nh;
        capacity = newCap;
    }
    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (!higher(heap[i], heap[p])) break;
            PQEntry t = heap[i]; heap[i] = heap[p]; heap[p] = t;
            i = p;
        }
    }

public:
    ParcelPriorityQueue() : heap(new PQEntry[16]), capacity(16), count(0) {}
    ~ParcelPriorityQueue() { delete[] heap; }
    ParcelPriorityQueue(const ParcelPriorityQueue&) = delete;
    ParcelPriorityQueue& operator=(const ParcelPriorityQueue&) = delete;

    bool isEmpty() const { return count == 0; }
    int  size() const { return count; }

    void push(int priority, long seq, const std::string& id) {
        if (count == capacity) grow();
        heap[count].priority = priority;
        heap[count].seq = seq;
        heap[count].parcelId = id;
        siftUp(count);
        ++count;
    }

    bool pop(std::string& outId) {
        if (isEmpty()) return false;
        outId = heap[0].parcelId;
        heap[0] = heap[count - 1];
        --count;
        if (count > 0) siftDownArray(heap, count, 0);
        return true;
    }

    PQEntry* sortedSnapshot(int& n) const {
        n = count;
        if (count == 0) return nullptr;
        PQEntry* tmp = new PQEntry[count];
        PQEntry* out = new PQEntry[count];
        for (int i = 0; i < count; ++i) tmp[i] = heap[i];
        int m = count;
        for (int k = 0; k < n; ++k) {
            out[k] = tmp[0];
            tmp[0] = tmp[m - 1];
            --m;
            if (m > 0) siftDownArray(tmp, m, 0);
        }
        delete[] tmp;
        return out;
    }
};
