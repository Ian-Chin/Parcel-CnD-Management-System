#include <iostream>
#include <string>
#include <cstdlib>
#include "resource_module.hpp"
using namespace std;

const string RESOURCE_FILE = "resources.txt";
const string PARCEL_FILE = "../task1 (Isac Lim)/parcels.txt";   // shared with Task 1

// ---------- input helpers ----------

string readLine(string label) {
    string s;
    ui::prompt(label);
    if (!getline(cin, s)) { cout << "\n"; exit(0); }
    // trim spaces at both ends
    int a = 0, b = (int)s.length() - 1;
    while (a <= b && (s[a] == ' ' || s[a] == '\t')) a++;
    while (b >= a && (s[b] == ' ' || s[b] == '\t')) b--;
    return s.substr(a, b - a + 1);
}

string readId(string label) {
    string s = readLine(label);
    for (int i = 0; i < (int)s.length(); i++) s[i] = toupper((unsigned char)s[i]);
    return s;
}

int readInt(string label, int low, int high) {
    while (true) {
        string s = readLine(label);
        bool ok = s.length() > 0 && s.length() < 6;
        for (int i = 0; i < (int)s.length(); i++) {
            if (!isdigit((unsigned char)s[i])) ok = false;
        }
        if (ok) {
            int v = atoi(s.c_str());
            if (v >= low && v <= high) return v;
        }
        ui::error("Please enter a number between " + to_string(low) + " and " + to_string(high) + ".");
    }
}

bool confirm(string question) {
    while (true) {
        string s = readLine(question + " (y/n)");
        if (s == "y" || s == "Y") return true;
        if (s == "n" || s == "N") return false;
        ui::error("Please type y or n.");
    }
}

// ---------- menu actions ----------

void showMenu(ResourceModule &m) {
    ui::bigTitle("SMART CAMPUS PARCEL SYSTEM", "Task 3 - Counter & Delivery Resources");
    cout << "\n  Resources: " << m.total()
         << "  |  Available: " << m.countByStatus(AVAILABLE)
         << "  |  Occupied: " << m.countByStatus(OCCUPIED)
         << "  |  Unavailable: " << m.countByStatus(UNAVAILABLE)
         << "  |  Maintenance: " << m.countByStatus(MAINTENANCE) << "\n";
    cout << "\n  " << ui::repeat("-", ui::WIDTH) << "\n"
         << "   RESOURCES\n"
         << "    [1]  Register new resource\n"
         << "    [2]  Display all resources\n"
         << "    [3]  Display availability\n"
         << "    [4]  Search resource by ID\n"
         << "    [5]  View allocation rotation   (Circular Queue)\n"
         << "\n   ASSIGNMENT\n"
         << "    [6]  Assign parcel / request\n"
         << "    [7]  Complete job (release resource)\n"
         << "    [8]  Cancel assignment\n"
         << "\n   MAINTAIN\n"
         << "    [9]  Update resource status\n"
         << "    [10] Update resource location\n"
         << "    [11] Remove resource\n"
         << "    [12] Utilisation report\n"
         << "\n    [0]  Exit (data is saved automatically)\n"
         << "  " << ui::repeat("-", ui::WIDTH) << "\n\n";
}

ResourceType chooseType() {
    cout << "\n    [1] Collection Counter      [2] Delivery Desk\n"
         << "    [3] Special-Handling Counter [4] Delivery Personnel\n\n";
    return (ResourceType)readInt("Resource type (1-4)", 1, 4);
}

void doRegister(ResourceModule &m) {
    ui::section("REGISTER NEW RESOURCE");
    string id = readId("Resource ID (e.g. C04, DP05)");
    ResourceType type = chooseType();
    string location = "-";
    if (type != DELIVERY_PERSONNEL) location = readLine("Location");
    cout << "\n";
    m.registerResource(id, type, location);
}

void doAssign(ResourceModule &m, ParcelModule &parcels) {
    ui::section("ASSIGN PARCEL / SERVICE REQUEST");
    parcels.displayAll();
    cout << "\n";
    string parcelId = readId("Parcel ID");
    string requestId = readId("Request ID (leave blank if none)");
    cout << "\n    [1] Recipient Collection   [2] Campus Delivery\n"
         << "    [3] Special Handling       [4] Return to Sender\n"
         << "    [5] Re-delivery\n\n";
    ServiceType service = (ServiceType)readInt("Service type (1-5)", 1, 5);
    cout << "\n";
    m.assign(parcelId, requestId, service);
}

void doChangeStatus(ResourceModule &m) {
    ui::section("UPDATE RESOURCE STATUS");
    m.displayAll();
    cout << "\n";
    string id = readId("Resource ID");
    if (!m.exists(id)) { ui::error("Resource " + id + " not found."); return; }
    cout << "\n    [1] Available   [2] Occupied   [3] Unavailable   [4] Maintenance\n\n";
    ResourceStatus s = (ResourceStatus)readInt("New status (1-4)", 1, 4);
    cout << "\n";
    m.changeStatus(id, s);
}

void doRemove(ResourceModule &m) {
    ui::section("REMOVE RESOURCE");
    m.displayAll();
    cout << "\n";
    string id = readId("Resource ID to remove");
    if (!m.exists(id)) { ui::error("Resource " + id + " not found."); return; }
    if (confirm("Remove resource " + id + "?")) m.removeResource(id);
    else ui::info("Removal cancelled.");
}

int main() {
    ParcelModule parcels;
    parcels.loadFromFile(PARCEL_FILE);

    ResourceModule module(parcels);
    int loaded = module.loadFromFile(RESOURCE_FILE);
    ui::clearScreen();
    if (loaded > 0) ui::success("Loaded " + to_string(loaded) + " resource(s) from " + RESOURCE_FILE);

    while (true) {
        showMenu(module);
        int choice = readInt("Choose an option", 0, 12);
        if (choice == 0) break;
        bool changed = true;

        if (choice == 1) doRegister(module);
        else if (choice == 2) { ui::section("ALL RESOURCES"); module.displayAll(); changed = false; }
        else if (choice == 3) { ui::section("RESOURCE AVAILABILITY"); module.displayAvailability(); changed = false; }
        else if (choice == 4) { ui::section("SEARCH RESOURCE"); module.showResource(readId("Resource ID")); changed = false; }
        else if (choice == 5) { ui::section("ALLOCATION ROTATION (CIRCULAR QUEUE)"); module.displayRotation(); changed = false; }
        else if (choice == 6) doAssign(module, parcels);
        else if (choice == 7) { ui::section("COMPLETE JOB"); module.displayAvailability(); cout << "\n"; module.completeJob(readId("Resource ID")); }
        else if (choice == 8) { ui::section("CANCEL ASSIGNMENT"); module.displayAvailability(); cout << "\n"; module.cancelJob(readId("Resource ID")); }
        else if (choice == 9) doChangeStatus(module);
        else if (choice == 10) { ui::section("UPDATE LOCATION"); string id = readId("Resource ID"); module.updateLocation(id, readLine("New location")); }
        else if (choice == 11) doRemove(module);
        else if (choice == 12) { ui::section("UTILISATION REPORT"); module.utilisationReport(); changed = false; }

        if (changed) {
            module.saveToFile(RESOURCE_FILE);
            parcels.saveToFile(PARCEL_FILE);
        }
        ui::pause();
        ui::clearScreen();
    }
    module.saveToFile(RESOURCE_FILE);
    parcels.saveToFile(PARCEL_FILE);
    cout << "\n  [OK]    Data saved. Goodbye!\n\n";
    return 0;
}
