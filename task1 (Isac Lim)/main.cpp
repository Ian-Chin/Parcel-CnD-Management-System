#include <iostream>
#include "parcel_menu.hpp"
using namespace std;

static const char* DATA_FILE = "parcels.txt";

int main() {
    ParcelModule module;
    int loaded = module.loadFromFile(DATA_FILE);
    ui::clearScreen();
    if (loaded > 0) cout << "\n  [OK]    Loaded " << loaded << " parcel(s) from " << DATA_FILE << "\n";

    parcel_menu::run(module, DATA_FILE);
    cout << "\n  [OK]    Data saved to " << DATA_FILE << ". Goodbye!\n\n";
    return 0;
}
