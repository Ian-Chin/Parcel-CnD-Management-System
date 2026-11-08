#include <iostream>
#include <string>
#include "../task1 (Isac Lim)/parcel_menu.hpp"
#include "../task2 (Soh Ye Wen)/request_menu.hpp"
#include "../task3 (Ian Chin)/resource_menu.hpp"
#include "activity_menu.hpp"
using namespace std;

// Standalone Task 4 program. Tasks 1, 2 and 3 are included because they are
// the modules that create the activities Task 4 records and undoes.
const string PARCEL_FILE   = "../data/parcels.txt";     // shared with Task 1
const string REQUEST_FILE  = "../data/requests.txt";    // shared with Task 2
const string RESOURCE_FILE = "../data/resources.txt";   // shared with Task 3
const string ACTIVITY_FILE = "../data/activities.txt";

int main() {
    ParcelModule parcels;
    parcels.loadFromFile(PARCEL_FILE);
    ResourceModule resources(parcels);
    resources.loadFromFile(RESOURCE_FILE);

    ActivityModule activities(parcels, resources);
    int loaded = activities.loadFromFile(ACTIVITY_FILE);
    parcels.setLog(&activities);      // connect after loading, so loading is not recorded
    resources.setLog(&activities);

    RequestModule requests(parcels, resources);
    requests.loadFromFile(REQUEST_FILE);
    requests.setLog(&activities);
    resources.setRequestLookup(&requests);
    activities.setRequestLink(&requests);

    cout << "\n";
    activities.setStaff(activity_menu::askStaffId());

    ui::clearScreen();
    if (loaded > 0) ui::success("Loaded " + to_string(loaded) + " activities from " + ACTIVITY_FILE);
    while (true) {
        ui::bigTitle("SMART CAMPUS PARCEL SYSTEM", "Task 4 - Standalone Test Program");
        cout << "\n    [1]  Task 1 - Parcels      (creates activities)\n"
             << "    [2]  Task 2 - Requests     (creates activities)\n"
             << "    [3]  Task 3 - Resources    (creates activities)\n"
             << "    [4]  Task 4 - Activity History & Undo\n"
             << "\n    [0]  Exit (data is saved automatically)\n\n";
        int choice = activity_menu::readInt("Choose a module", 0, 4);
        if (choice == 0) break;
        ui::clearScreen();
        if (choice == 1)      parcel_menu::run(parcels, PARCEL_FILE);
        else if (choice == 2) request_menu::run(requests, parcels, resources, REQUEST_FILE, PARCEL_FILE, RESOURCE_FILE);
        else if (choice == 3) resource_menu::run(resources, parcels, RESOURCE_FILE, PARCEL_FILE);
        else                  activity_menu::run(activities, parcels, resources, PARCEL_FILE, RESOURCE_FILE, REQUEST_FILE);
        ui::clearScreen();
    }
    parcels.saveToFile(PARCEL_FILE);
    resources.saveToFile(RESOURCE_FILE);
    requests.saveToFile(REQUEST_FILE);
    cout << "\n  [OK]    All data saved. Goodbye!\n\n";
    return 0;
}
