// main.cpp - Task 5 (kerjinn) standalone launcher
#include "category_module.hpp"

int main() {
    task5::runCategoryModule("../data/parcels.txt");   // writes the shared parcel file immediately
    return 0;
}