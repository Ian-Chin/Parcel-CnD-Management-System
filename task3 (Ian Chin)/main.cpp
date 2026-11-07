#include <iostream>
#include <string>
#include "resource_menu.hpp"
using namespace std;

const string RESOURCE_FILE = "../data/resources.txt";
const string PARCEL_FILE = "../data/parcels.txt";

int main() {
    ParcelModule parcels;
    parcels.loadFromFile(PARCEL_FILE);

    ResourceModule module(parcels);
    int loaded = module.loadFromFile(RESOURCE_FILE);
    ui::clearScreen();
    if (loaded > 0) ui::success("Loaded " + to_string(loaded) + " resource(s) from " + RESOURCE_FILE);

    resource_menu::run(module, parcels, RESOURCE_FILE, PARCEL_FILE);
    cout << "\n  [OK]    Data saved. Goodbye!\n\n";
    return 0;
}
