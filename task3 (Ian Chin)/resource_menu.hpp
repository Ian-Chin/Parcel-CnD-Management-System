#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include "resource_module.hpp"

// Task 3 menu. Kept in its own namespace so the integrated main.cpp can
// include it together with the other task menus without name clashes.
namespace resource_menu {

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
         << "    [1]  Register resource\n"
         << "    [2]  View resources\n"
         << "    [3]  Assign parcel\n"
         << "    [4]  Finish job (completed or cancelled)\n"
         << "    [5]  Change resource status\n"
         << "    [6]  Remove resource\n"
         << "    [7]  Utilisation report\n"
         << "\n    [0]  Back (data is saved automatically)\n"
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
    ui::section("ASSIGN PARCEL");
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

// Shows the table, availability and rotation order on one screen
void doView(ResourceModule &m) {
    ui::section("ALL RESOURCES");
    m.displayAll();
    ui::section("AVAILABILITY");
    m.displayAvailability();
    ui::section("ALLOCATION ROTATION (CIRCULAR QUEUE)");
    m.displayRotation();
}

// One option for both ways a job can end
void doFinishJob(ResourceModule &m) {
    ui::section("FINISH JOB");
    m.displayAvailability();
    cout << "\n";
    string id = readId("Resource ID");
    cout << "\n    [1] Completed   [2] Cancelled\n\n";
    int how = readInt("How did the job end (1-2)", 1, 2);
    cout << "\n";
    if (how == 1) m.completeJob(id);
    else m.cancelJob(id);
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

// Runs the Task 3 menu until the user picks [0]. Saves both files after every change.
void run(ResourceModule &module, ParcelModule &parcels, string resourceFile, string parcelFile) {
    while (true) {
        showMenu(module);
        int choice = readInt("Choose an option", 0, 7);
        if (choice == 0) break;
        bool changed = true;

        if (choice == 1) doRegister(module);
        else if (choice == 2) { doView(module); changed = false; }
        else if (choice == 3) doAssign(module, parcels);
        else if (choice == 4) doFinishJob(module);
        else if (choice == 5) doChangeStatus(module);
        else if (choice == 6) doRemove(module);
        else if (choice == 7) { ui::section("UTILISATION REPORT"); module.utilisationReport(); changed = false; }

        if (changed) {
            module.saveToFile(resourceFile);
            parcels.saveToFile(parcelFile);
        }
        ui::pause();
        ui::clearScreen();
    }
    module.saveToFile(resourceFile);
    parcels.saveToFile(parcelFile);
}

}
