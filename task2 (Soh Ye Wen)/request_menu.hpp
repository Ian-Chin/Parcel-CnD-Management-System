#pragma once
#include <iostream>
#include <string>
#include "../task3 (Ian Chin)/resource_menu.hpp"
#include "request_module.hpp"
using namespace std;

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
    string parcelId = readId("Parcel ID (0 to cancel)");
    if (parcelId == "0") return false;
    string recipientId = readId("Recipient ID (0 to cancel)");
    if (recipientId == "0") return false;
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
    if (id == "" || id == "0") return false;
    cout << "\n";
    return m.processById(id);
}

// Shows the active requests and finishes the one the user picks
bool doFinish(RequestModule& m) {
    heading("FINISH A REQUEST");
    m.displayActive();
    cout << "\n";
    string id = readId("Request ID (0 to cancel)");
    if (id == "" || id == "0") return false;
    cout << "\n";
    return m.finishRequest(id);
}

// Shows the active requests and cancels the one the user picks
bool doCancel(RequestModule& m) {
    heading("CANCEL A REQUEST");
    m.displayActive();
    cout << "\n";
    string id = readId("Request ID (0 to cancel)");
    if (id == "" || id == "0") return false;
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
        else if (choice == 3) {
            heading("VIEW REQUEST");
            string id = readId("Request ID (0 to cancel)");
            if (id != "" && id != "0") module.viewRequest(id);
        }
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
