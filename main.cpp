#include <iostream>
#include <string>
#include "task1 (Isac Lim)/parcel_menu.hpp"
#include "task3 (Ian Chin)/resource_menu.hpp"
using namespace std;

const string PARCEL_FILE   = "task1 (Isac Lim)/parcels.txt";
const string RESOURCE_FILE = "task3 (Ian Chin)/resources.txt";

void showMainMenu(ParcelModule &parcels, ResourceModule &resources) {
    ui::bigTitle("SMART CAMPUS PARCEL SYSTEM", "Integrated Prototype - Main Menu");
    cout << "\n  Parcels: " << parcels.totalParcels()
         << "  |  Pending: " << parcels.countByStatus(ParcelStatus::Pending)
         << "  |  Ready: " << parcels.countByStatus(ParcelStatus::ReadyForCollection)
         << "  |  Out for Delivery: " << parcels.countByStatus(ParcelStatus::OutForDelivery) << "\n"
         << "  Resources: " << resources.total()
         << "  |  Available: " << resources.countByStatus(AVAILABLE)
         << "  |  Occupied: " << resources.countByStatus(OCCUPIED) << "\n";
    cout << "\n  " << ui::repeat("-", ui::WIDTH) << "\n"
         << "    [1]  Task 1 - Parcel Intake, Sorting & Processing\n"
         << "    [2]  Task 2 - Service & Collection Requests     (coming soon)\n"
         << "    [3]  Task 3 - Counter & Delivery Resources\n"
         << "    [4]  Task 4 - Activity & Record History         (coming soon)\n"
         << "    [5]  Task 5 - Parcel Category Organisation      (coming soon)\n"
         << "\n    [0]  Exit (data is saved automatically)\n"
         << "  " << ui::repeat("-", ui::WIDTH) << "\n\n";
}

int main() {
    // One ParcelModule shared by every task, so a change made in one task
    // (e.g. a parcel assigned in Task 3) is visible straight away in the others.
    ParcelModule parcels;
    parcels.loadFromFile(PARCEL_FILE);

    ResourceModule resources(parcels);
    resources.loadFromFile(RESOURCE_FILE);

    ui::clearScreen();
    while (true) {
        showMainMenu(parcels, resources);
        int choice = resource_menu::readInt("Choose a module", 0, 5);
        if (choice == 0) break;

        ui::clearScreen();
        if (choice == 1) {
            parcel_menu::run(parcels, PARCEL_FILE);
        } else if (choice == 3) {
            resource_menu::run(resources, parcels, RESOURCE_FILE, PARCEL_FILE);
        } else {
            ui::info("Task " + to_string(choice) + " has not been added to the integrated system yet.");
            ui::pause();
        }
        ui::clearScreen();
    }

    parcels.saveToFile(PARCEL_FILE);
    resources.saveToFile(RESOURCE_FILE);
    cout << "\n  [OK]    All data saved. Goodbye!\n\n";
    return 0;
}
