#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "request_list.hpp"
#include "../task1 (Isac Lim)/parcel_guard.hpp"
using namespace std;

class RequestModule : public RequestLookup, public RequestLink, public ParcelGuard {
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
    // (report = false: only check, print nothing)
    bool serviceAllowed(ServiceType s, ParcelStatus st, bool report = true) {
        if (st == ParcelStatus::Received) { if (report) ui::error("Parcel is only Received. Sort the parcel first (Task 1)."); return false; }
        if (isTerminalStatus(st)) { if (report) ui::error("Parcel is already " + statusToString(st) + " and cannot take a request."); return false; }
        bool ok = true;   // Return to Sender: any status left here is fine
        if (s == RECIPIENT_COLLECTION) ok = (st == ParcelStatus::ReadyForCollection);
        if (s == CAMPUS_DELIVERY || s == RE_DELIVERY) ok = (st == ParcelStatus::OutForDelivery || st == ParcelStatus::ReadyForCollection);
        if (s == SPECIAL_HANDLING) ok = (st == ParcelStatus::Processing);
        if (!ok && report) ui::error("Invalid service operation: " + serviceToString(s) + " is not allowed while the parcel is " + statusToString(st) + ".");
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

    // True if the request's parcel was removed or is already finished
    // (Collected, Returned, Missing, Cancelled), so the request can never run.
    bool parcelFinished(Request* r) {
        Parcel* p = parcels.findParcel(r->parcelId);
        return p == NULL || isTerminalStatus(p->status);
    }

    // Closes a Pending request whose parcel is finished, so it stops blocking the queue.
    // Collection of a Collected parcel / Return of a Returned parcel already happened,
    // so those count as done; anything else is Cancelled and recorded in Task 4.
    void closeFinished(Request* r) {
        Parcel* p = parcels.findParcel(r->parcelId);
        pending.removeById(r->requestId);
        if (p != NULL && p->status == ParcelStatus::Collected && r->service == RECIPIENT_COLLECTION) r->status = RequestStatus::Completed;
        else if (p != NULL && p->status == ParcelStatus::Returned && r->service == RETURN_TO_SENDER) r->status = RequestStatus::Returned;
        else r->status = RequestStatus::Cancelled;
        history.insertEnd(r);
        if (activityLog != NULL && p != NULL && r->status == RequestStatus::Cancelled)
            activityLog->record(ActivityType::RequestCancelled, r->parcelId, r->requestId, "-", p->status, p->status);
        string why = (p == NULL) ? "no longer exists" : "is already " + statusToString(p->status);
        ui::warn("Parcel " + r->parcelId + " " + why + ". Request " + r->requestId + " closed as " + requestStatusToString(r->status) + ".");
    }

    // True if the request can be started now: parcel not finished and service allowed
    bool canStart(Request* r) {
        return !parcelFinished(r) && serviceAllowed(r->service, parcels.findParcel(r->parcelId)->status, false);
    }

    // Asks Task 3 for a resource; on success moves the request to processing.
    // Returns true if the data changed (request started, or closed because its parcel is finished).
    bool startRequest(Request* r) {
        if (parcelFinished(r)) { closeFinished(r); return true; }
        // The parcel status may have changed since the request was created
        if (!serviceAllowed(r->service, parcels.findParcel(r->parcelId)->status)) {
            ui::info("Request " + r->requestId + " stays Pending. Update the parcel first or cancel the request.");
            return false;
        }
        if (!resources.assign(r->parcelId, r->requestId, r->service)) {
            ui::info("Request " + r->requestId + " stays Pending. Try again later.");
            ui::info("Use option 6 to process another request.");
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

    // Staff ID entered at startup (kept by Task 4), "-" if not connected
    string currentStaff() { return activityLog != NULL ? activityLog->staffId() : "-"; }

    // ParcelGuard: true if a request or resource still uses this parcel.
    bool isBusy(const std::string& parcelId, std::string& reason) const override {
        Request* r = pending.findByParcel(parcelId);
        if (r == NULL) r = processing.findByParcel(parcelId);
        if (r != NULL) {
            reason = "parcel has active request " + r->requestId
                   + " (" + requestStatusToString(r->status) + ")";
            return true;
        }
        string rid = resources.resourceHolding(parcelId);
        if (rid != "-") {
            reason = "resource " + rid + " is still holding this parcel";
            return true;
        }
        return false;
    }

    // For Task 3: ID of the Pending / Processing request for this parcel, or "-"
    string activeRequestFor(string parcelId) {
        Request* r = pending.findByParcel(parcelId);
        if (r == NULL) r = processing.findByParcel(parcelId);
        return r == NULL ? "-" : r->requestId;
    }

    // For Task 4 undo: is this request being handled by a resource now?
    bool isProcessing(string id) { return processing.findById(id) != NULL; }

    // For Task 4 undo: the resource was taken back, so the request waits again
    bool backToPending(string id) {
        Request* r = processing.removeById(id);
        if (r == NULL) return false;
        r->status = RequestStatus::Pending;
        pending.insertSorted(r);
        return true;
    }

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
        if (activityLog != NULL) activityLog->record(ActivityType::ServiceRequestCreation, parcelId, id, "-", p->status, p->status, staffId);
        ui::success("Request " + id + " created (" + serviceToString(service) + "), status Pending.");
        return true;
    }

    // 2. Shows the requests still waiting for action
    void displayActive() {
        cout << "\n  Pending (priority order, top is next):\n";
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

    // 4. Shows the request that "Process next" would handle, without removing it
    void showNext() {
        if (pending.isEmpty()) { ui::info("No pending requests. The queue is empty."); return; }
        Request* r = pending.first();
        while (r != NULL && !canStart(r)) r = r->next;   // same skipping as processNext
        if (r == NULL) { ui::info("No pending request can be processed right now (check the parcel statuses)."); return; }
        cout << "  Next request to be handled:\n\n";
        showRequest(r);
    }

    // 5. Processes the highest-priority request that can be served now.
    // Requests whose parcel is finished are closed. Requests whose service is not
    // allowed for the parcel's current status are skipped but stay Pending with
    // their priority, so they run once the parcel status allows it.
    bool processNext() {
        if (pending.isEmpty()) { ui::error("No pending requests. The queue is empty."); return false; }
        bool changed = false;
        Request* r = pending.first();
        while (r != NULL) {
            Request* after = r->next;   // saved first: closing r moves it to history
            if (parcelFinished(r)) { closeFinished(r); changed = true; }
            else if (!serviceAllowed(r->service, parcels.findParcel(r->parcelId)->status, false))
                ui::warn("Request " + r->requestId + " skipped: " + serviceToString(r->service) + " is not allowed while parcel "
                         + r->parcelId + " is " + statusToString(parcels.findParcel(r->parcelId)->status) + ". It stays Pending.");
            else return startRequest(r) || changed;
            r = after;
        }
        if (pending.isEmpty()) ui::error("No pending requests. The queue is empty.");
        else ui::error("No pending request can be processed right now (check the parcel statuses).");
        return changed;
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
        if (p == NULL) { ui::error("Parcel " + r->parcelId + " not found, so request " + id + " cannot be finished. Cancel it instead."); return false; }
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
            ui::warn("No resource is handling parcel " + r->parcelId + ". Request " + id + " is back to Pending. Process it again.");
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
        Parcel* p = parcels.findParcel(r->parcelId);
        if (activityLog != NULL && p != NULL) activityLog->record(ActivityType::RequestCancelled, r->parcelId, id, "-", p->status, p->status);
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
        if (!in) { ui::warn("File '" + path + "' not found. Starting with no requests."); return 0; }
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
            // An active request whose parcel no longer exists can never run: keep it as Cancelled.
            // Finished requests stay as they are, since they are history even if the parcel was removed.
            // If a resource still holds the parcel, leave it active so cancelling it frees the resource.
            if ((r->status == RequestStatus::Pending || r->status == RequestStatus::Processing) && parcels.findParcel(r->parcelId) == NULL) {
                if (resources.resourceHolding(r->parcelId) == "-") {
                    ui::warn("Request " + r->requestId + ": parcel " + r->parcelId + " not found, marked Cancelled.");
                    r->status = RequestStatus::Cancelled;
                }
                else ui::warn("Request " + r->requestId + ": parcel " + r->parcelId + " not found. Cancel this request to free its resource.");
            }
            if (r->status == RequestStatus::Pending) pending.insertSorted(r);
            else if (r->status == RequestStatus::Processing) processing.insertEnd(r);
            else history.insertEnd(r);
            if (r->seq >= nextNumber) nextNumber = r->seq + 1;
            loaded++;
        }
        return loaded;
    }
};