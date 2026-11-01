#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "../task1 (Isac Lim)/parcel_module.hpp"
#include "resource.hpp"
#include "circular_queue.hpp"
using namespace std;

const int MAX_RESOURCES = 50;

class ResourceModule {
private:
    Resource resources[MAX_RESOURCES];   // all registered resources
    int resourceCount;

    // One round-robin queue per resource type (index 1-4, index 0 unused)
    CircularQueue rotation[5];

    ParcelModule &parcels;               // Task 1 parcel records

    // Returns position of resource in the array, or -1 if not found
    int findIndex(string id) {
        for (int i = 0; i < resourceCount; i++) {
            if (resources[i].resourceId == id) return i;
        }
        return -1;
    }

    bool isValidId(string id) {
        if (id.length() < 2 || id.length() > 10) return false;
        for (int i = 0; i < (int)id.length(); i++) {
            if (!isalnum((unsigned char)id[i])) return false;
        }
        return true;
    }

    // Allocation rule: which resource type should handle this job
    ResourceType suitableType(ServiceType service, ParcelType parcelType) {
        if (service == CAMPUS_DELIVERY || service == RE_DELIVERY) return DELIVERY_PERSONNEL;
        if (service == RETURN_TO_SENDER) return DELIVERY_DESK;
        if (service == SPECIAL_HANDLING) return SPECIAL_COUNTER;
        // Recipient collection: fragile / special parcels need the special counter
        if (parcelType == ParcelType::Fragile || parcelType == ParcelType::SpecialHandling) return SPECIAL_COUNTER;
        return COLLECTION_COUNTER;
    }

    // Round robin: go around the circular queue once and return the first
    // available resource. Every resource checked is moved to the back.
    int nextAvailable(ResourceType type) {
        int n = rotation[type].size();
        for (int i = 0; i < n; i++) {
            string id;
            rotation[type].dequeue(id);
            rotation[type].enqueue(id);
            int idx = findIndex(id);
            if (idx != -1 && resources[idx].status == AVAILABLE) return idx;
        }
        return -1;
    }

    // Change the parcel status in Task 1 only if it is different
    void setParcelStatus(string parcelId, ParcelStatus newStatus) {
        Parcel *p = parcels.findParcel(parcelId);
        if (p == nullptr) return;
        if (p->status == newStatus) return;
        if (isTerminalStatus(p->status)) return;
        ParcelStatus old = p->status;
        p->status = newStatus;
        ui::info("Parcel " + parcelId + " status: " + statusToString(old) + " -> " + statusToString(newStatus));
    }

    void clearJob(Resource &r) {
        r.parcelId = "-";
        r.requestId = "-";
    }

    bool allOccupied() {
        for (int i = 0; i < resourceCount; i++) {
            if (resources[i].status == AVAILABLE) return false;
        }
        return true;
    }

public:
    ResourceModule(ParcelModule &p) : parcels(p) {
        resourceCount = 0;
    }

    int total() { return resourceCount; }

    int countByStatus(ResourceStatus s) {
        int c = 0;
        for (int i = 0; i < resourceCount; i++) {
            if (resources[i].status == s) c++;
        }
        return c;
    }

    bool exists(string id) { return findIndex(id) != -1; }

    // ---------- 1. Register and maintain ----------

    bool registerResource(string id, ResourceType type, string location) {
        if (!isValidId(id)) {
            ui::error("Invalid Resource ID (2-10 letters/digits, no spaces).");
            return false;
        }
        if (findIndex(id) != -1) {
            ui::error("Duplicate Resource ID: " + id + " already exists.");
            return false;
        }
        if (location.find(',') != string::npos) {
            ui::error("Location cannot contain commas.");
            return false;
        }
        if (resourceCount == MAX_RESOURCES) {
            ui::error("Cannot register more resources (limit reached).");
            return false;
        }
        if (rotation[type].isFull()) {
            ui::error("Too many resources of this type (limit " + to_string(MAX_QUEUE) + ").");
            return false;
        }
        if (location == "") location = "-";

        Resource r;
        r.resourceId = id;
        r.type = type;
        r.location = location;
        r.status = AVAILABLE;
        r.parcelId = "-";
        r.requestId = "-";
        r.service = RECIPIENT_COLLECTION;
        r.transactions = 0;

        resources[resourceCount] = r;
        resourceCount++;
        rotation[type].enqueue(id);
        ui::success("Resource " + id + " (" + resourceTypeToString(type) + ") registered as Available.");
        return true;
    }

    bool updateLocation(string id, string location) {
        int i = findIndex(id);
        if (i == -1) { ui::error("Resource " + id + " not found."); return false; }
        if (location == "" || location.find(',') != string::npos) {
            ui::error("Location cannot be empty or contain commas.");
            return false;
        }
        resources[i].location = location;
        ui::success("Location of " + id + " updated to " + location + ".");
        return true;
    }

    bool removeResource(string id) {
        int i = findIndex(id);
        if (i == -1) { ui::error("Resource " + id + " not found."); return false; }
        if (resources[i].status == OCCUPIED) {
            ui::error("Resource " + id + " is handling parcel " + resources[i].parcelId + ". Complete or cancel the job first.");
            return false;
        }
        rotation[resources[i].type].remove(id);
        // shift the rest of the array left by one
        for (int j = i; j < resourceCount - 1; j++) {
            resources[j] = resources[j + 1];
        }
        resourceCount--;
        ui::success("Resource " + id + " removed.");
        return true;
    }

    // ---------- 2. Display ----------

    void showResource(string id) {
        int i = findIndex(id);
        if (i == -1) { ui::error("Resource " + id + " not found."); return; }
        Resource &r = resources[i];
        cout << "\n  Resource ID    : " << r.resourceId << "\n"
             << "  Type           : " << resourceTypeToString(r.type) << "\n"
             << "  Location       : " << r.location << "\n"
             << "  Status         : " << resourceStatusToString(r.status) << "\n"
             << "  Current Parcel : " << r.parcelId << "\n"
             << "  Current Request: " << r.requestId << "\n";
        if (r.status == OCCUPIED)
            cout << "  Service        : " << serviceToString(r.service) << "\n";
        cout << "  Transactions   : " << r.transactions << "\n";
    }

    void displayAll() {
        if (resourceCount == 0) { ui::info("No resources registered yet."); return; }
        int w[6] = { 8, 24, 12, 11, 8, 8 };
        ui::tableLine(w, 6);
        ui::rowOpen();
        ui::cell("ID", w[0]); ui::cell("Type", w[1]); ui::cell("Location", w[2]);
        ui::cell("Status", w[3]); ui::cell("Parcel", w[4]); ui::cell("Request", w[5]);
        ui::rowClose();
        ui::tableLine(w, 6);
        for (int i = 0; i < resourceCount; i++) {
            Resource &r = resources[i];
            ui::rowOpen();
            ui::cell(r.resourceId, w[0]);
            ui::cell(resourceTypeToString(r.type), w[1]);
            ui::cell(r.location, w[2]);
            ui::cell(resourceStatusToString(r.status), w[3]);
            ui::cell(r.parcelId, w[4]);
            ui::cell(r.requestId, w[5]);
            ui::rowClose();
        }
        ui::tableLine(w, 6);
    }

    // Grouped view: available / occupied / unavailable + maintenance
    void displayAvailability() {
        if (resourceCount == 0) { ui::info("No resources registered yet."); return; }

        cout << "\n  AVAILABLE (" << countByStatus(AVAILABLE) << ")\n";
        for (int i = 0; i < resourceCount; i++)
            if (resources[i].status == AVAILABLE)
                cout << "    " << resources[i].resourceId << "  " << resourceTypeToString(resources[i].type) << "\n";

        cout << "\n  OCCUPIED (" << countByStatus(OCCUPIED) << ")\n";
        for (int i = 0; i < resourceCount; i++)
            if (resources[i].status == OCCUPIED)
                cout << "    " << resources[i].resourceId << "  " << resourceTypeToString(resources[i].type)
                     << "  -> Parcel " << resources[i].parcelId
                     << ", Request " << resources[i].requestId
                     << " (" << serviceToString(resources[i].service) << ")\n";

        cout << "\n  UNAVAILABLE / MAINTENANCE (" << countByStatus(UNAVAILABLE) + countByStatus(MAINTENANCE) << ")\n";
        for (int i = 0; i < resourceCount; i++)
            if (resources[i].status == UNAVAILABLE || resources[i].status == MAINTENANCE)
                cout << "    " << resources[i].resourceId << "  " << resourceTypeToString(resources[i].type)
                     << "  [" << resourceStatusToString(resources[i].status) << "]\n";
    }

    // Shows the circular queue order for each type (who gets the next job)
    void displayRotation() {
        for (int t = 1; t <= 4; t++) {
            cout << "\n  " << resourceTypeToString((ResourceType)t) << ": ";
            if (rotation[t].isEmpty()) { cout << "(none)\n"; continue; }
            for (int i = 0; i < rotation[t].size(); i++) {
                string id = rotation[t].getAt(i);
                int idx = findIndex(id);
                cout << id;
                if (idx != -1 && resources[idx].status != AVAILABLE) cout << "(" << resourceStatusToString(resources[idx].status) << ")";
                if (i < rotation[t].size() - 1) cout << " -> ";
            }
            cout << "\n";
        }
        cout << "\n  The first Available resource from the front gets the next job.\n";
    }

    // ---------- 3. Assign ----------

    bool assign(string parcelId, string requestId, ServiceType service) {
        if (resourceCount == 0) { ui::error("No resources registered."); return false; }

        Parcel *p = parcels.findParcel(parcelId);
        if (p == nullptr) { ui::error("Parcel " + parcelId + " not found in parcel records."); return false; }
        if (isTerminalStatus(p->status)) {
            ui::error("Parcel " + parcelId + " is already '" + statusToString(p->status) + "' and cannot be assigned.");
            return false;
        }
        if (requestId == "") requestId = "-";
        if (requestId != "-" && (requestId[0] != 'R' || !isValidId(requestId))) {
            ui::error("Invalid Request ID (must start with R, e.g. R001).");
            return false;
        }

        // Duplicate active assignment check
        for (int i = 0; i < resourceCount; i++) {
            if (resources[i].status != OCCUPIED) continue;
            if (resources[i].parcelId == parcelId) {
                ui::error("Parcel " + parcelId + " is already assigned to " + resources[i].resourceId + ".");
                return false;
            }
            if (requestId != "-" && resources[i].requestId == requestId) {
                ui::error("Request " + requestId + " is already assigned to " + resources[i].resourceId + ".");
                return false;
            }
        }

        if (allOccupied()) {
            ui::error("All resources are currently occupied or unavailable. Please try again later.");
            return false;
        }

        ResourceType type = suitableType(service, p->type);
        if (rotation[type].isEmpty()) {
            ui::error("No " + resourceTypeToString(type) + " is registered for this job.");
            return false;
        }
        int idx = nextAvailable(type);
        if (idx == -1) {
            ui::error("No suitable resource available: every " + resourceTypeToString(type) + " is busy or unavailable.");
            return false;
        }

        Resource &r = resources[idx];
        r.status = OCCUPIED;
        r.parcelId = parcelId;
        r.requestId = requestId;
        r.service = service;
        ui::success("Parcel " + parcelId + " assigned to " + r.resourceId + " (" + resourceTypeToString(r.type) + ").");

        if (service == CAMPUS_DELIVERY || service == RE_DELIVERY) setParcelStatus(parcelId, ParcelStatus::OutForDelivery);
        else if (service == RETURN_TO_SENDER) setParcelStatus(parcelId, ParcelStatus::Processing);
        else setParcelStatus(parcelId, ParcelStatus::ReadyForCollection);
        return true;
    }

    // ---------- Release ----------

    // Job finished successfully: count the transaction and free the resource
    bool completeJob(string id) {
        int i = findIndex(id);
        if (i == -1) { ui::error("Resource " + id + " not found."); return false; }
        Resource &r = resources[i];
        if (r.status != OCCUPIED) {
            ui::error("Resource " + id + " has no job to complete (status: " + resourceStatusToString(r.status) + ").");
            return false;
        }
        if (r.service == RETURN_TO_SENDER) setParcelStatus(r.parcelId, ParcelStatus::Returned);
        else setParcelStatus(r.parcelId, ParcelStatus::Collected);

        ui::success("Job for parcel " + r.parcelId + " completed by " + id + ". Resource is Available again.");
        r.transactions++;
        r.status = AVAILABLE;
        clearJob(r);
        return true;
    }

    // Job cancelled: free the resource, parcel goes back to Pending
    bool cancelJob(string id) {
        int i = findIndex(id);
        if (i == -1) { ui::error("Resource " + id + " not found."); return false; }
        Resource &r = resources[i];
        if (r.status != OCCUPIED) {
            ui::error("Resource " + id + " has no job to cancel.");
            return false;
        }
        setParcelStatus(r.parcelId, ParcelStatus::Pending);
        ui::success("Assignment of parcel " + r.parcelId + " to " + id + " cancelled.");
        r.status = AVAILABLE;
        clearJob(r);
        return true;
    }

    // ---------- 4. Update status ----------

    bool changeStatus(string id, ResourceStatus newStatus) {
        int i = findIndex(id);
        if (i == -1) { ui::error("Resource " + id + " not found."); return false; }
        Resource &r = resources[i];

        if (r.status == newStatus) {
            ui::error("Resource is already " + resourceStatusToString(newStatus) + ".");
            return false;
        }
        if (newStatus == OCCUPIED) {
            ui::error("A resource becomes Occupied only through 'Assign parcel / request'.");
            return false;
        }
        if (r.status == OCCUPIED && newStatus == AVAILABLE) {
            ui::error("Resource is handling a job. Use 'Complete job' or 'Cancel assignment' instead.");
            return false;
        }

        // Resource becomes unavailable while it is handling a job:
        // move the job to another available resource of the same type.
        if (r.status == OCCUPIED) {
            string parcelId = r.parcelId;
            string requestId = r.requestId;
            ServiceType service = r.service;

            r.status = newStatus;
            clearJob(r);
            ui::warn(id + " became " + resourceStatusToString(newStatus) + " while handling parcel " + parcelId + ".");

            int other = nextAvailable(r.type);
            if (other != -1) {
                resources[other].status = OCCUPIED;
                resources[other].parcelId = parcelId;
                resources[other].requestId = requestId;
                resources[other].service = service;
                ui::success("Parcel " + parcelId + " re-assigned to " + resources[other].resourceId + ".");
            } else {
                setParcelStatus(parcelId, ParcelStatus::Pending);
                ui::warn("No other " + resourceTypeToString(r.type) + " available. Parcel " + parcelId + " must be assigned again later.");
            }
            return true;
        }

        ResourceStatus old = r.status;
        r.status = newStatus;
        ui::success(id + " status: " + resourceStatusToString(old) + " -> " + resourceStatusToString(newStatus));
        return true;
    }

    // ---------- Utilisation report ----------

    void utilisationReport() {
        if (resourceCount == 0) { ui::info("No resources registered yet."); return; }

        int totalJobs = 0;
        for (int i = 0; i < resourceCount; i++) totalJobs += resources[i].transactions;

        int w[4] = { 8, 24, 12, 8 };
        ui::tableLine(w, 4);
        ui::rowOpen();
        ui::cell("ID", w[0]); ui::cell("Type", w[1]); ui::cell("Completed", w[2]); ui::cell("Share", w[3]);
        ui::rowClose();
        ui::tableLine(w, 4);
        for (int i = 0; i < resourceCount; i++) {
            int share = 0;
            if (totalJobs > 0) share = resources[i].transactions * 100 / totalJobs;
            ui::rowOpen();
            ui::cell(resources[i].resourceId, w[0]);
            ui::cell(resourceTypeToString(resources[i].type), w[1]);
            ui::cell(to_string(resources[i].transactions), w[2]);
            ui::cell(to_string(share) + "%", w[3]);
            ui::rowClose();
        }
        ui::tableLine(w, 4);

        cout << "\n  Jobs completed by type:\n";
        for (int t = 1; t <= 4; t++) {
            int sum = 0;
            for (int i = 0; i < resourceCount; i++)
                if (resources[i].type == t) sum += resources[i].transactions;
            cout << "    " << ui::fit(resourceTypeToString((ResourceType)t), 26) << sum << "\n";
        }

        int busiest = 0;
        for (int i = 1; i < resourceCount; i++)
            if (resources[i].transactions > resources[busiest].transactions) busiest = i;

        cout << "\n  Total jobs completed : " << totalJobs << "\n"
             << "  Currently occupied   : " << countByStatus(OCCUPIED) << " of " << resourceCount << "\n";
        if (totalJobs > 0)
            cout << "  Busiest resource     : " << resources[busiest].resourceId
                 << " (" << resources[busiest].transactions << " jobs)\n";
    }

    // ---------- File save / load ----------
    // Format: ID,Type,Location,Status,ParcelID,RequestID,Service,Transactions

    bool saveToFile(string path) {
        ofstream out(path);
        if (!out) { ui::error("Cannot write file " + path); return false; }
        out << "# ID,Type(1-4),Location,Status(1-4),ParcelID,RequestID,Service(1-5),Transactions\n";
        out << "# Type: 1=Collection Counter 2=Delivery Desk 3=Special-Handling Counter 4=Delivery Personnel\n";
        out << "# Status: 1=Available 2=Occupied 3=Unavailable 4=Maintenance\n";
        for (int i = 0; i < resourceCount; i++) {
            Resource &r = resources[i];
            out << r.resourceId << "," << r.type << "," << r.location << "," << r.status << ","
                << r.parcelId << "," << r.requestId << "," << r.service << "," << r.transactions << "\n";
        }
        return true;
    }

    int loadFromFile(string path) {
        ifstream in(path);
        if (!in) { ui::warn("File '" + path + "' not found - starting with no resources."); return 0; }
        string line;
        int loaded = 0;
        while (getline(in, line)) {
            if (line == "" || line[0] == '#') continue;

            // split the line by commas into 8 fields
            string f[8];
            int n = 0;
            string current = "";
            for (int i = 0; i < (int)line.length(); i++) {
                if (line[i] == ',' && n < 7) { f[n] = current; n++; current = ""; }
                else if (line[i] != '\r') current += line[i];
            }
            f[n] = current;
            n++;
            if (n != 8) { ui::warn("Skipped bad line: " + line); continue; }

            int type = atoi(f[1].c_str());
            int status = atoi(f[3].c_str());
            int service = atoi(f[6].c_str());
            if (type < 1 || type > 4 || status < 1 || status > 4 || service < 1 || service > 5) {
                ui::warn("Skipped bad line: " + line);
                continue;
            }
            if (!registerQuiet(f[0], (ResourceType)type, f[2])) { ui::warn("Skipped line: " + line); continue; }

            Resource &r = resources[resourceCount - 1];
            r.status = (ResourceStatus)status;
            r.parcelId = f[4];
            r.requestId = f[5];
            r.service = (ServiceType)service;
            r.transactions = atoi(f[7].c_str());
            if (r.status != OCCUPIED) clearJob(r);
            loaded++;
        }
        return loaded;
    }

private:
    // Same as registerResource but without printing (used when loading)
    bool registerQuiet(string id, ResourceType type, string location) {
        if (!isValidId(id) || findIndex(id) != -1) return false;
        if (resourceCount == MAX_RESOURCES || rotation[type].isFull()) return false;
        Resource r;
        r.resourceId = id;
        r.type = type;
        r.location = location;
        r.status = AVAILABLE;
        r.parcelId = "-";
        r.requestId = "-";
        r.service = RECIPIENT_COLLECTION;
        r.transactions = 0;
        resources[resourceCount] = r;
        resourceCount++;
        rotation[type].enqueue(id);
        return true;
    }
};
