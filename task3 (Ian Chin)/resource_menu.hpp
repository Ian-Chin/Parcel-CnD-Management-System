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

// Light heading used inside Task 3 (one title line + a thin rule)
void heading(string title) {
    cout << "\n  " << title << "\n  " << ui::repeat("-", ui::WIDTH) << "\n";
}

// Cancel helpers: 0 (or Enter on an ID prompt) leaves the action straight
// away. run() sees the flag and returns to the menu without saving or pausing.
bool backedOut = false;

bool cancelled(string input) {
    backedOut = (input == "" || input == "0");
    return backedOut;
}

bool cancelled(int choice) {
    backedOut = (choice == 0);
    return backedOut;
}

void showMenu() {
    cout << "\n  TASK 3  RESOURCES\n\n"
         << "    1  View resources\n"
         << "    2  Assign a parcel\n"
         << "    3  Finish a job\n"
         << "    4  Utilisation report\n"
         << "    5  Manage resources (add, status, remove)\n\n"
         << "    0  Back\n\n";
}

// Returns 0 if the user cancels
int chooseType() {
    cout << "\n    1 Collection Counter   2 Delivery Desk\n"
         << "    3 Special-Handling     4 Delivery Personnel\n\n";
    return readInt("Type (0 to cancel)", 0, 4);
}

void doRegister(ResourceModule &m) {
    heading("ADD RESOURCE");
    string id = readId("Resource ID, e.g. C04 (0 to cancel)");
    if (cancelled(id)) return;
    int choice = chooseType();
    if (cancelled(choice)) return;
    ResourceType type = (ResourceType)choice;
    string location = "-";
    if (type != DELIVERY_PERSONNEL) location = readLine("Location");
    cout << "\n";
    m.registerResource(id, type, location);
}

void doAssign(ResourceModule &m, ParcelModule &parcels) {
    heading("ASSIGN A PARCEL");
    parcels.displayAll();
    cout << "\n";
    ui::info("Walk-in jobs only. Parcels with a service request are assigned from Requests (Task 2).");
    string parcelId = readId("Parcel ID (0 to cancel)");
    if (cancelled(parcelId)) return;
    cout << "\n    1 Collection   2 Delivery   3 Special Handling\n"
         << "    4 Return       5 Re-delivery\n\n";
    int service = readInt("Service (0 to cancel)", 0, 5);
    if (cancelled(service)) return;
    cout << "\n";
    m.assign(parcelId, "-", (ServiceType)service);
}

// One table plus the circular queue order (who gets the next job)
void doView(ResourceModule &m) {
    heading("RESOURCES");
    m.displayAll();
    cout << "\n  Next-job order (circular queue):\n";
    m.displayRotation();
}

// One option for both ways a job can end. Only busy resources are listed.
void doFinishJob(ResourceModule &m) {
    heading("FINISH A JOB");
    if (m.countByStatus(OCCUPIED) == 0) { ui::info("No resource is busy right now."); return; }
    m.displayOccupied();
    cout << "\n";
    string id = readId("Resource ID (0 to cancel)");
    if (cancelled(id)) return;
    int how = readInt("1 Completed  2 Cancelled  0 Go back", 0, 2);
    if (cancelled(how)) return;
    cout << "\n";
    if (how == 1) m.completeJob(id);
    else m.cancelJob(id);
}

void doChangeStatus(ResourceModule &m) {
    heading("CHANGE STATUS");
    m.displayAll();
    cout << "\n";
    string id = readId("Resource ID (0 to cancel)");
    if (cancelled(id)) return;
    if (!m.exists(id)) { ui::error("Resource " + id + " not found."); return; }
    cout << "\n    1 Available   2 Occupied   3 Unavailable   4 Maintenance\n\n";
    int status = readInt("New status (0 to cancel)", 0, 4);
    if (cancelled(status)) return;
    cout << "\n";
    m.changeStatus(id, (ResourceStatus)status);
}

void doRemove(ResourceModule &m) {
    heading("REMOVE RESOURCE");
    m.displayAll();
    cout << "\n";
    string id = readId("Resource ID to remove (0 to cancel)");
    if (cancelled(id)) return;
    if (!m.exists(id)) { ui::error("Resource " + id + " not found."); return; }
    if (confirm("Remove resource " + id + "?")) m.removeResource(id);
    else ui::info("Removal cancelled.");
}

// Less common admin actions, kept off the main Task 3 menu
void doManage(ResourceModule &m) {
    heading("MANAGE RESOURCES");
    cout << "    1  Add resource\n"
         << "    2  Change status\n"
         << "    3  Remove resource\n\n"
         << "    0  Back\n\n";
    int choice = readInt("Choose", 0, 3);
    if (cancelled(choice)) return;
    if (choice == 1) doRegister(m);
    else if (choice == 2) doChangeStatus(m);
    else if (choice == 3) doRemove(m);
}

// Runs the Task 3 menu until the user picks 0. Saves both files after every change.
void run(ResourceModule &module, ParcelModule &parcels, string resourceFile, string parcelFile) {
    while (true) {
        showMenu();
        int choice = readInt("Choose", 0, 5);
        if (choice == 0) break;
        bool changed = true;
        backedOut = false;
        ui::clearScreen();   // each action starts on a clean screen

        if (choice == 1) { doView(module); changed = false; }
        else if (choice == 2) doAssign(module, parcels);
        else if (choice == 3) doFinishJob(module);
        else if (choice == 4) { heading("UTILISATION REPORT"); module.utilisationReport(); changed = false; }
        else if (choice == 5) doManage(module);

        // User typed 0 part-way through: nothing changed, go straight back
        if (backedOut) { ui::clearScreen(); continue; }

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
