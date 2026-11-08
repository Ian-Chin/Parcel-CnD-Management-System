#pragma once
#include <iostream>
#include <string>
#include "../task3 (Ian Chin)/resource_module.hpp"
using namespace std;

// Status of a service request
enum class RequestStatus { Pending = 1, Processing, Completed, Cancelled, Returned };

// Returns the request status as text
string requestStatusToString(RequestStatus s) {
    if (s == RequestStatus::Pending)    return "Pending";
    if (s == RequestStatus::Processing) return "Processing";
    if (s == RequestStatus::Completed)  return "Completed";
    if (s == RequestStatus::Cancelled)  return "Cancelled";
    if (s == RequestStatus::Returned)   return "Returned";
    return "Unknown";
}

// Priority of a service type (1 = most urgent).
// Re-delivery already failed once and Special Handling needs special care,
// so they go first. Requests with the same priority are first come, first served.
int servicePriority(ServiceType s) {
    if (s == RE_DELIVERY)          return 1;
    if (s == SPECIAL_HANDLING)     return 2;
    if (s == CAMPUS_DELIVERY)      return 3;
    if (s == RECIPIENT_COLLECTION) return 4;
    return 5;   // RETURN_TO_SENDER
}

// One service request = one node of the linked list
struct Request {
    string requestId;      // R001, R002, ...
    int seq;               // number part of the ID (1, 2, ...) = arrival order
    string parcelId;
    string recipientId;
    string recipientName;
    ServiceType service;
    string requestTime;
    RequestStatus status;
    string staffId;
    Request* next;
};

// Singly linked list of requests. Used 3 times: pending (priority queue),
// processing and history.
class RequestList {
private:
    Request* head;
    int count;

    // True if request a must be in front of request b (priority, then seq)
    bool goesBefore(Request* a, Request* b) {
        if (servicePriority(a->service) != servicePriority(b->service))
            return servicePriority(a->service) < servicePriority(b->service);
        return a->seq < b->seq;
    }

public:
    RequestList() { head = NULL; count = 0; }

    // Deletes every node
    ~RequestList() {
        while (head != NULL) {
            Request* temp = head;
            head = head->next;
            delete temp;
        }
    }

    bool isEmpty() { return head == NULL; }
    Request* first() { return head; }

    // Priority queue insert: puts the node after all nodes that go before it
    void insertSorted(Request* newNode) {
        newNode->next = NULL;
        if (head == NULL || goesBefore(newNode, head)) {
            newNode->next = head;
            head = newNode;
        } else {
            Request* current = head;
            while (current->next != NULL && !goesBefore(newNode, current->next))
                current = current->next;
            newNode->next = current->next;
            current->next = newNode;
        }
        count++;
    }

    // Adds the node at the end of the list
    void insertEnd(Request* newNode) {
        newNode->next = NULL;
        if (head == NULL) head = newNode;
        else {
            Request* current = head;
            while (current->next != NULL) current = current->next;
            current->next = newNode;
        }
        count++;
    }

    // Detaches the first node and returns it (NULL if empty)
    Request* removeFront() {
        if (head == NULL) return NULL;
        Request* temp = head;
        head = head->next;
        temp->next = NULL;
        count--;
        return temp;
    }

    // Detaches the node with this ID from anywhere in the list (NULL if not found)
    Request* removeById(string id) {
        if (head == NULL) return NULL;
        if (head->requestId == id) return removeFront();
        Request* current = head;
        while (current->next != NULL && current->next->requestId != id)
            current = current->next;
        if (current->next == NULL) return NULL;
        Request* temp = current->next;
        current->next = temp->next;
        temp->next = NULL;
        count--;
        return temp;
    }

    // Returns the node with this request ID, or NULL
    Request* findById(string id) {
        Request* current = head;
        while (current != NULL && current->requestId != id) current = current->next;
        return current;
    }

    // Returns the node for this parcel ID, or NULL
    Request* findByParcel(string parcelId) const {
        Request* current = head;
        while (current != NULL && current->parcelId != parcelId) current = current->next;
        return current;
    }

    // Prints the list as a table
    void display() {
        if (head == NULL) { ui::info("No requests in this list."); return; }
        int w[6] = { 6, 8, 16, 20, 10, 8 };
        ui::tableLine(w, 6);
        ui::rowOpen();
        ui::cell("ID", w[0]); ui::cell("Parcel", w[1]); ui::cell("Recipient", w[2]);
        ui::cell("Service", w[3]); ui::cell("Status", w[4]); ui::cell("Staff", w[5]);
        ui::rowClose();
        ui::tableLine(w, 6);
        for (Request* current = head; current != NULL; current = current->next) {
            ui::rowOpen();
            ui::cell(current->requestId, w[0]); ui::cell(current->parcelId, w[1]);
            ui::cell(current->recipientName, w[2]); ui::cell(serviceToString(current->service), w[3]);
            ui::cell(requestStatusToString(current->status), w[4]); ui::cell(current->staffId, w[5]);
            ui::rowClose();
        }
        ui::tableLine(w, 6);
        cout << "  " << count << " request(s)\n";
    }
};
