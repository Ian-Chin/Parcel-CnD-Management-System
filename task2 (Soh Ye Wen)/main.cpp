#include <iostream>
#include <string>
#include "request_menu.hpp"
#include "../task4 (Muhlis)/activity_menu.hpp"
using namespace std;

const string REQUEST_FILE = "../data/requests.txt";
const string PARCEL_FILE = "../data/parcels.txt";
const string RESOURCE_FILE = "../data/resources.txt";
const string ACTIVITY_FILE = "../data/activities.txt";   // Task 4 history

int main() {
    ParcelModule parcels;
    parcels.loadFromFile(PARCEL_FILE);

    ResourceModule resources(parcels);
    resources.loadFromFile(RESOURCE_FILE);

    // Task 4 log, connected after loading so loading is not recorded
    ActivityModule activities(parcels, resources);
    activities.loadFromFile(ACTIVITY_FILE);
    parcels.setLog(&activities);
    resources.setLog(&activities);

    RequestModule module(parcels, resources);
    int loaded = module.loadFromFile(REQUEST_FILE);
    module.setLog(&activities);
    resources.setRequestLookup(&module);
    activities.setRequestLink(&module);
    parcels.setGuard(&module);

    cout << "\n";
    activities.setStaff(activity_menu::askStaffId());   // used when an activity has no Staff ID

    ui::clearScreen();
    if (loaded > 0) ui::success("Loaded " + to_string(loaded) + " request(s) from " + REQUEST_FILE);

    request_menu::run(module, parcels, resources, REQUEST_FILE, PARCEL_FILE, RESOURCE_FILE);
    cout << "\n  [OK]    Data saved. Goodbye!\n\n";
    return 0;
}
