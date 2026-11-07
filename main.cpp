#include <iostream>
#include <string>
#include "task1 (Isac Lim)/parcel_menu.hpp"
#include "task2 (Soh Ye Wen)/request_menu.hpp"
#include "task3 (Ian Chin)/resource_menu.hpp"
#include "task4 (Muhlis)/activity_menu.hpp"
#include "task5 (kerjinn)/category_module.hpp"
using namespace std;

const string PARCEL_FILE    = "data/parcels.txt";
const string RESOURCE_FILE  = "data/resources.txt";
const string CATEGORY_FILE  = "data/parcels.txt";
const string ACTIVITY_FILE  = "data/activities.txt";
const string REQUEST_FILE   = "data/requests.txt";

void showMainMenu(ParcelModule &parcels, ResourceModule &resources) {
    cout << "\n  SMART CAMPUS PARCEL SYSTEM\n  " << ui::repeat("=", ui::WIDTH) << "\n"
         << "  " << parcels.totalParcels() << " parcels ("
         << parcels.countByStatus(ParcelStatus::Pending) << " pending)    "
         << resources.total() << " resources ("
         << resources.countByStatus(AVAILABLE) << " free)\n\n"
         << "    1  Parcels      register, sort, process\n"
         << "    2  Requests     service requests\n"
         << "    3  Resources    counters and delivery staff\n"
         << "    4  History      activity log and undo\n"
         << "    5  Categories   parcel search tree\n\n"
         << "    0  Save and exit\n\n";
}

int main() {
    // One ParcelModule shared by every task, so a change made in one task
    // (e.g. a parcel assigned in Task 3) is visible straight away in the others.
    ParcelModule parcels;
    parcels.loadFromFile(PARCEL_FILE);

    ResourceModule resources(parcels);
    resources.loadFromFile(RESOURCE_FILE);

    // Task 4 records every Task 1 / Task 3 action (connected after loading)
    ActivityModule activities(parcels, resources);
    activities.loadFromFile(ACTIVITY_FILE);
    parcels.setLog(&activities);
    resources.setLog(&activities);

    // Task 2 service requests
    RequestModule requests(parcels, resources);
    requests.loadFromFile(REQUEST_FILE);
    requests.setLog(&activities);
    resources.setRequestLookup(&requests);   // Task 3 checks Task 2 before walk-in jobs

    ui::clearScreen();
    while (true) {
        showMainMenu(parcels, resources);
        int choice = resource_menu::readInt("Choose", 0, 5);
        if (choice == 0) break;

        ui::clearScreen();
        if (choice == 1) parcel_menu::run(parcels, PARCEL_FILE);
        else if (choice == 2) request_menu::run(requests, parcels, resources, REQUEST_FILE, PARCEL_FILE, RESOURCE_FILE);
        else if (choice == 3) resource_menu::run(resources, parcels, RESOURCE_FILE, PARCEL_FILE);
        else if (choice == 4) activity_menu::run(activities, parcels, resources, PARCEL_FILE, RESOURCE_FILE);
        else if (choice == 5) task5::runCategoryModule(CATEGORY_FILE);   // BST-based category organisation
        ui::clearScreen();
    }

    parcels.saveToFile(PARCEL_FILE);
    resources.saveToFile(RESOURCE_FILE);
    requests.saveToFile(REQUEST_FILE);
    cout << "\n  [OK]    All data saved. Goodbye!\n\n";
    return 0;
}