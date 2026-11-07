#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "../task3 (Ian Chin)/resource_menu.hpp"
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
    Request* findByParcel(string parcelId) {
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

class RequestModule {
private:
    RequestList pending;       // priority queue of requests waiting for a resource
    RequestList processing;    // requests a resource is handling now
    RequestList history;       // Completed / Cancelled / Returned
    int nextNumber;            // number for the next new Request ID
    ParcelModule& parcels;
    ResourceModule& resources;
    ActivityLog* activityLog;  // Task 4 history (NULL = not connected)

    // Makes a Request ID from a number, e.g. 7 -> "R007"
    string makeId(int n) {
        string s = to_string(n);
        while (s.length() < 3) s = "0" + s;
        return "R" + s;
    }

    // Looks for a request in all three lists (NULL if not found)
    Request* findAnywhere(string id) {
        Request* r = pending.findById(id);
        if (r == NULL) r = processing.findById(id);
        if (r == NULL) r = history.findById(id);
        return r;
    }

    // Checks the service is allowed for the parcel's current status
    bool serviceAllowed(ServiceType s, ParcelStatus st) {
        if (st == ParcelStatus::Received) { ui::error("Parcel is only Received - sort the parcel first (Task 1)."); return false; }
        if (isTerminalStatus(st)) { ui::error("Parcel is already " + statusToString(st) + " and cannot take a request."); return false; }
        bool ok = true;   // Return to Sender: any status left here is fine
        if (s == RECIPIENT_COLLECTION) ok = (st == ParcelStatus::ReadyForCollection);
        if (s == CAMPUS_DELIVERY || s == RE_DELIVERY) ok = (st == ParcelStatus::OutForDelivery || st == ParcelStatus::ReadyForCollection);
        if (s == SPECIAL_HANDLING) ok = (st == ParcelStatus::Processing);
        if (!ok) ui::error("Invalid service operation: " + serviceToString(s) + " is not allowed while the parcel is " + statusToString(st) + ".");
        return ok;
    }

    // Creates a new node with all the request details
    Request* newRequest(string id, string parcelId, string recipientId, string name, ServiceType service, string time, RequestStatus status, string staffId) {
        Request* newNode = new Request;
        newNode->requestId = id;
        newNode->seq = atoi(id.substr(1).c_str());
        newNode->parcelId = parcelId;
        newNode->recipientId = recipientId;
        newNode->recipientName = name;
        newNode->service = service;
        newNode->requestTime = time;
        newNode->status = status;
        newNode->staffId = staffId;
        newNode->next = NULL;
        return newNode;
    }

    // Prints every detail of one request
    void showRequest(Request* r) {
        cout << "  Request ID   : " << r->requestId << "\n"
             << "  Parcel ID    : " << r->parcelId << "\n"
             << "  Recipient ID : " << r->recipientId << "\n"
             << "  Recipient    : " << r->recipientName << "\n"
             << "  Service      : " << serviceToString(r->service) << " (priority " << servicePriority(r->service) << ")\n"
             << "  Requested at : " << r->requestTime << "\n"
             << "  Status       : " << requestStatusToString(r->status) << "\n"
             << "  Staff ID     : " << r->staffId << "\n";
    }

    // Asks Task 3 for a resource; on success moves the request to processing
    bool startRequest(Request* r) {
        if (!resources.assign(r->parcelId, r->requestId, r->service)) {
            ui::info("Request " + r->requestId + " stays Pending. Try again later.");
            return false;
        }
        pending.removeById(r->requestId);
        r->status = RequestStatus::Processing;
        processing.insertEnd(r);
        ui::success("Request " + r->requestId + " is now Processing.");
        return true;
    }

    // Writes one list to the file, one request per line
    void saveList(ofstream& out, RequestList& list) {
        for (Request* r = list.first(); r != NULL; r = r->next)
            out << r->requestId << "," << r->parcelId << "," << r->recipientId << "," << r->recipientName << ","
                << r->service << "," << r->requestTime << "," << (int)r->status << "," << r->staffId << "\n";
    }

public:
    RequestModule(ParcelModule& p, ResourceModule& r) : parcels(p), resources(r) {
        nextNumber = 1;
        activityLog = NULL;
    }

    void setLog(ActivityLog* log) { activityLog = log; }

    // 1. Creates a new Pending request after checking all the rules
    bool createRequest(string staffId, string parcelId, string recipientId, ServiceType service) {
        if (staffId == "" || parcelId == "" || recipientId == "") { ui::error("Staff ID, Parcel ID and Recipient ID are all required."); return false; }
        if (staffId.find(',') != string::npos) { ui::error("Staff ID cannot contain commas."); return false; }
        Parcel* p = parcels.findParcel(parcelId);
        if (p == NULL) { ui::error("Invalid Parcel ID: " + parcelId + " not found."); return false; }
        if (p->recipientId != recipientId) { ui::error("Incorrect recipient information: parcel " + parcelId + " is not for " + recipientId + "."); return false; }
        if (pending.findByParcel(parcelId) != NULL || processing.findByParcel(parcelId) != NULL) {
            ui::error("Duplicate request: parcel " + parcelId + " already has an active request.");
            return false;
        }
        if (!serviceAllowed(service, p->status)) return false;

        string id = makeId(nextNumber);
        nextNumber++;
        pending.insertSorted(newRequest(id, parcelId, recipientId, p->recipientName, service, currentDateTime(), RequestStatus::Pending, staffId));
        //TODO change to ServiceRequestCreation when Task 4 adds it
        if (activityLog != NULL) activityLog->record(ActivityType::ParcelUpdate, parcelId, id, "-", p->status, p->status);
        ui::success("Request " + id + " created (" + serviceToString(service) + "), status Pending.");
        return true;
    }

    // 2. Shows the requests still waiting for action
    void displayActive() {
        cout << "\n  Pending (priority order - top is next):\n";
        pending.display();
        cout << "\n  Processing (handled by a resource):\n";
        processing.display();
    }

    // 3. Shows one request by ID
    void viewRequest(string id) {
        Request* r = findAnywhere(id);
        if (r == NULL) { ui::error("Invalid Request ID: " + id + " not found."); return; }
        showRequest(r);
    }

    // 4. Shows the next request without removing it
    void showNext() {
        if (pending.isEmpty()) { ui::info("No pending requests. The queue is empty."); return; }
        cout << "  Next request to be handled:\n\n";
        showRequest(pending.first());
    }

    // 5. Processes the request at the front of the priority queue
    bool processNext() {
        if (pending.isEmpty()) { ui::error("No pending requests. The queue is empty."); return false; }
        return startRequest(pending.first());
    }

    // 6. Processes a request chosen by ID
    bool processById(string id) {
        Request* r = pending.findById(id);
        if (r != NULL) return startRequest(r);
        if (processing.findById(id) != NULL) { ui::error("Request " + id + " is already being processed."); return false; }
        r = history.findById(id);
        if (r != NULL) { ui::error("Request " + id + " is already " + requestStatusToString(r->status) + ", cannot be processed again."); return false; }
        ui::error("Invalid Request ID: " + id + " not found.");
        return false;
    }

    // 7. Finishes a request that a resource is handling
    bool finishRequest(string id) {
        Request* r = processing.findById(id);
        if (r == NULL) {
            if (pending.findById(id) != NULL) ui::error("Request " + id + " is still Pending. Process it first.");
            else if (history.findById(id) != NULL) ui::error("Request " + id + " is already " + requestStatusToString(history.findById(id)->status) + ".");
            else ui::error("Invalid Request ID: " + id + " not found.");
            return false;
        }
        string rid = resources.resourceHolding(r->parcelId);
        Parcel* p = parcels.findParcel(r->parcelId);
        if (rid != "-") {
            resources.completeJob(rid);   // Task 3 updates the parcel status
            if (r->service == RETURN_TO_SENDER) r->status = RequestStatus::Returned;
            else r->status = RequestStatus::Completed;
        }
        else if (p != NULL && p->status == ParcelStatus::Collected) r->status = RequestStatus::Completed;
        else if (p != NULL && p->status == ParcelStatus::Returned) r->status = RequestStatus::Returned;
        else {
            processing.removeById(id);
            r->status = RequestStatus::Pending;
            pending.insertSorted(r);
            ui::warn("No resource is handling parcel " + r->parcelId + ". Request " + id + " is back to Pending - process it again.");
            return true;
        }
        processing.removeById(id);
        history.insertEnd(r);
        ui::success("Request " + id + " is now " + requestStatusToString(r->status) + ".");
        return true;
    }

    // 8. Cancels a pending or processing request
    bool cancelRequest(string id) {
        Request* r = history.findById(id);
        if (r != NULL) { ui::error("Request " + id + " is already " + requestStatusToString(r->status) + ", cannot be cancelled."); return false; }
        r = pending.removeById(id);
        if (r == NULL) {
            r = processing.findById(id);
            if (r == NULL) { ui::error("Invalid Request ID: " + id + " not found."); return false; }
            string rid = resources.resourceHolding(r->parcelId);
            if (rid != "-") resources.cancelJob(rid);   // Task 3 frees the resource, parcel -> Pending
            processing.removeById(id);
        }
        r->status = RequestStatus::Cancelled;
        history.insertEnd(r);
        ui::success("Request " + id + " cancelled.");
        return true;
    }

    // 9. Shows the finished requests
    void displayHistory() { history.display(); }

    // Saves pending, then processing, then history
    bool saveToFile(string path) {
        ofstream out(path);
        if (!out) { ui::error("Cannot write file " + path); return false; }
        out << "# RequestID,ParcelID,RecipientID,RecipientName,Service(1-5),RequestTime,Status(1-5),StaffID\n";
        out << "# Service: 1=Collection 2=Campus Delivery 3=Special Handling 4=Return to Sender 5=Re-delivery\n";
        out << "# Status: 1=Pending 2=Processing 3=Completed 4=Cancelled 5=Returned\n";
        saveList(out, pending);
        saveList(out, processing);
        saveList(out, history);
        return true;
    }

    // Loads requests into the right list; returns how many were loaded
    int loadFromFile(string path) {
        ifstream in(path);
        if (!in) { ui::warn("File '" + path + "' not found - starting with no requests."); return 0; }
        string line;
        int loaded = 0;
        while (getline(in, line)) {
            if (line == "" || line[0] == '#' || line[0] == '\r') continue;
            string f[8];
            int n = 0;
            for (int i = 0; i < (int)line.length(); i++) {   // split by commas
                if (line[i] == ',') { n++; if (n > 7) break; }   // too many commas = bad line
                else if (line[i] != '\r') f[n] += line[i];
            }
            int service = atoi(f[4].c_str());
            int status = atoi(f[6].c_str());
            if (n != 7 || f[0].length() < 2 || f[0][0] != 'R' || service < 1 || service > 5 || status < 1 || status > 5) { ui::warn("Skipped bad line: " + line); continue; }
            if (findAnywhere(f[0]) != NULL) { ui::warn("Skipped duplicate request: " + f[0]); continue; }

            Request* r = newRequest(f[0], f[1], f[2], f[3], (ServiceType)service, f[5], (RequestStatus)status, f[7]);
            if (r->status == RequestStatus::Pending) pending.insertSorted(r);
            else if (r->status == RequestStatus::Processing) processing.insertEnd(r);
            else history.insertEnd(r);
            if (r->seq >= nextNumber) nextNumber = r->seq + 1;
            loaded++;
        }
        return loaded;
    }
};

// Task 2 menu
namespace request_menu {

using resource_menu::readId;
using resource_menu::readInt;
using resource_menu::confirm;
using resource_menu::heading;

// Prints the Task 2 menu
void showMenu() {
    cout << "\n  TASK 2  SERVICE REQUESTS\n\n"
         << "    1  Create a service request\n"
         << "    2  Display active requests\n"
         << "    3  View one request\n"
         << "    4  Show next request\n"
         << "    5  Process next request\n"
         << "    6  Process a chosen request\n"
         << "    7  Finish a request\n"
         << "    8  Cancel a request\n"
         << "    9  Request history\n\n"
         << "    0  Back\n\n";
}

// Asks for the request details and creates it
bool doCreate(RequestModule& m, ParcelModule& parcels) {
    heading("CREATE SERVICE REQUEST");
    parcels.displayAll();
    cout << "\n";
    string staffId = readId("Staff ID (0 to cancel)");
    if (staffId == "0") return false;
    string parcelId = readId("Parcel ID");
    string recipientId = readId("Recipient ID");
    cout << "\n    1 Recipient Collection   2 Campus Delivery   3 Special Handling\n"
         << "    4 Return to Sender       5 Re-delivery\n\n";
    int service = readInt("Service (0 to cancel)", 0, 5);
    if (service == 0) return false;
    cout << "\n";
    return m.createRequest(staffId, parcelId, recipientId, (ServiceType)service);
}

// Shows the active requests and processes the one the user picks
bool doProcessChosen(RequestModule& m) {
    heading("PROCESS A CHOSEN REQUEST");
    m.displayActive();
    cout << "\n";
    string id = readId("Request ID (0 to cancel)");
    if (id == "0") return false;
    cout << "\n";
    return m.processById(id);
}

// Shows the active requests and finishes the one the user picks
bool doFinish(RequestModule& m) {
    heading("FINISH A REQUEST");
    m.displayActive();
    cout << "\n";
    string id = readId("Request ID (0 to cancel)");
    if (id == "0") return false;
    cout << "\n";
    return m.finishRequest(id);
}

// Shows the active requests and cancels the one the user picks
bool doCancel(RequestModule& m) {
    heading("CANCEL A REQUEST");
    m.displayActive();
    cout << "\n";
    string id = readId("Request ID (0 to cancel)");
    if (id == "0") return false;
    if (!confirm("Cancel request " + id + "?")) { ui::info("Nothing cancelled."); return false; }
    cout << "\n";
    return m.cancelRequest(id);
}

// Saves requests, parcels and resources
void saveAll(RequestModule& m, ParcelModule& parcels, ResourceModule& resources, string requestFile, string parcelFile, string resourceFile) {
    m.saveToFile(requestFile);
    parcels.saveToFile(parcelFile);
    resources.saveToFile(resourceFile);
}

// Runs the Task 2 menu until the user picks 0. Saves after every change.
void run(RequestModule& module, ParcelModule& parcels, ResourceModule& resources, string requestFile, string parcelFile, string resourceFile) {
    while (true) {
        showMenu();
        int choice = readInt("Choose", 0, 9);
        if (choice == 0) break;
        ui::clearScreen();
        bool changed = false;

        if (choice == 1) changed = doCreate(module, parcels);
        else if (choice == 2) { heading("ACTIVE REQUESTS"); module.displayActive(); }
        else if (choice == 3) { heading("VIEW REQUEST"); module.viewRequest(readId("Request ID")); }
        else if (choice == 4) { heading("NEXT REQUEST"); module.showNext(); }
        else if (choice == 5) { heading("PROCESS NEXT REQUEST"); changed = module.processNext(); }
        else if (choice == 6) changed = doProcessChosen(module);
        else if (choice == 7) changed = doFinish(module);
        else if (choice == 8) changed = doCancel(module);
        else if (choice == 9) { heading("REQUEST HISTORY"); module.displayHistory(); }

        if (changed) saveAll(module, parcels, resources, requestFile, parcelFile, resourceFile);
        ui::pause();
        ui::clearScreen();
    }
    saveAll(module, parcels, resources, requestFile, parcelFile, resourceFile);
}

}
