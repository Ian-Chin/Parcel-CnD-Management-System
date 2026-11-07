#include <iostream>
#include <string>
#include "request_menu.hpp"
using namespace std;

const string REQUEST_FILE = "../data/requests.txt";
const string PARCEL_FILE = "../data/parcels.txt";
const string RESOURCE_FILE = "../data/resources.txt";

int main() {
    ParcelModule parcels;
    parcels.loadFromFile(PARCEL_FILE);

    ResourceModule resources(parcels);
    resources.loadFromFile(RESOURCE_FILE);

    RequestModule module(parcels, resources);
    int loaded = module.loadFromFile(REQUEST_FILE);
    resources.setRequestLookup(&module);
    ui::clearScreen();
    if (loaded > 0) ui::success("Loaded " + to_string(loaded) + " request(s) from " + REQUEST_FILE);

    request_menu::run(module, parcels, resources, REQUEST_FILE, PARCEL_FILE, RESOURCE_FILE);
    cout << "\n  [OK]    Data saved. Goodbye!\n\n";
    return 0;
}
