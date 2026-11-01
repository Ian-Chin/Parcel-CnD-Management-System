#include <iostream>
#include <string>
#include <cstdlib>
#include "parcel_module.hpp"
using namespace std;

static const char* DATA_FILE = "parcels.txt";

static string readLine(const string& label) {
    string s;
    ui::prompt(label);
    if (!getline(cin, s)) { cout << "\n"; exit(0); }
    size_t a = s.find_first_not_of(" \t"), b = s.find_last_not_of(" \t");
    return (a == string::npos) ? "" : s.substr(a, b - a + 1);
}

static string readId(const string& label) {
    string s = readLine(label);
    for (size_t i = 0; i < s.size(); ++i) s[i] = static_cast<char>(toupper(static_cast<unsigned char>(s[i])));
    return s;
}

static int readInt(const string& label, int lo, int hi) {
    while (true) {
        string s = readLine(label);
        bool ok = !s.empty() && s.size() < 6;
        for (size_t i = 0; i < s.size() && ok; ++i) ok = isdigit(static_cast<unsigned char>(s[i])) != 0;
        if (ok) { int v = stoi(s); if (v >= lo && v <= hi) return v; }
        ui::error("Please enter a number between " + to_string(lo) + " and " + to_string(hi) + ".");
    }
}

static bool confirm(const string& question) {
    while (true) {
        string s = readLine(question + " (y/n)");
        if (s == "y" || s == "Y") return true;
        if (s == "n" || s == "N") return false;
        ui::error("Please type y or n.");
    }
}

static ParcelType chooseType() {
    cout << "\n    [1] Standard      [2] Large       [3] Fragile\n"
         << "    [4] Document      [5] Special-Handling\n\n";
    return static_cast<ParcelType>(readInt("Parcel type (1-5)", 1, 5));
}

static bool chooseManualStatus(ParcelStatus& out) {
    cout << "\n    [1] Ready for Collection\n"
         << "    [2] Out for Delivery\n"
         << "    [3] Collected\n"
         << "    [4] Returned\n"
         << "    [5] Missing\n"
         << "    [6] Cancelled\n"
         << "    [0] Back\n\n";
    int c = readInt("New status", 0, 6);
    if (c == 0) return false;
    ParcelStatus map[6] = { ParcelStatus::ReadyForCollection, ParcelStatus::OutForDelivery,
                            ParcelStatus::Collected, ParcelStatus::Returned,
                            ParcelStatus::Missing, ParcelStatus::Cancelled };
    out = map[c - 1];
    return true;
}

static void showMenu(const ParcelModule& m) {
    ui::bigTitle("SMART CAMPUS PARCEL SYSTEM", "Task 1 - Intake, Sorting & Processing");
    cout << "\n  Total: " << m.totalParcels()
         << "  |  Received: " << m.countByStatus(ParcelStatus::Received)
         << "  |  Pending: " << m.countByStatus(ParcelStatus::Pending)
         << "  |  Processing: " << m.countByStatus(ParcelStatus::Processing)
         << "  |  Ready: " << m.countByStatus(ParcelStatus::ReadyForCollection) << "\n";
    cout << "\n  " << ui::repeat("-", ui::WIDTH) << "\n"
         << "   REGISTER & SEARCH\n"
         << "    [1]  Register new parcel\n"
         << "    [2]  Display all parcels\n"
         << "    [3]  Search by Parcel ID\n"
         << "    [4]  Search by Recipient ID\n"
         << "\n   SORT & PROCESS\n"
         << "    [5]  View intake queue        (Queue, FIFO)\n"
         << "    [6]  Sort incoming parcels    (Queue -> Priority Queue)\n"
         << "    [7]  View processing order    (Priority Queue)\n"
         << "    [8]  Process next parcel\n"
         << "\n   MANAGE RECORDS\n"
         << "    [9]  Update parcel\n"
         << "    [10] Remove parcel\n"
         << "\n    [0]  Exit (data is saved automatically)\n"
         << "  " << ui::repeat("-", ui::WIDTH) << "\n\n";
}

static void doRegister(ParcelModule& m) {
    ui::section("REGISTER NEW PARCEL");
    string pid  = readId("Parcel ID       ");
    string rid  = readId("Recipient ID    ");
    string name = readLine("Recipient name  ");
    string comp = readLine("Delivery company");
    ParcelType t = chooseType();
    cout << "\n";
    m.registerParcel(pid, rid, name, comp, t);
}

static void doProcess(ParcelModule& m) {
    ui::section("PROCESS NEXT PARCEL");
    string route;
    Parcel* p = m.processNext(route);
    if (p) {
        ui::success("Parcel " + p->parcelId + " processed.");
        ui::parcelCard(*p, "PROCESSED PARCEL");
        cout << "  Routed to: " << route << "\n";
    }
}

static void doUpdate(ParcelModule& m) {
    ui::section("UPDATE PARCEL");
    m.displayAll();
    cout << "\n";
    string pid = readId("Parcel ID to update");
    Parcel* p = m.findParcel(pid);
    if (!p) { ui::error("Parcel " + pid + " not found."); return; }
    ui::parcelCard(*p, "CURRENT RECORD");
    cout << "\n    [1] Recipient name\n"
         << "    [2] Parcel type (only while status is Received)\n"
         << "    [3] Status\n"
         << "    [0] Back\n\n";
    int f = readInt("Field to update", 0, 3);
    cout << "\n";
    if (f == 1) {
        m.updateRecipient(pid, readLine("New name"));
    } else if (f == 2) {
        if (p->status != ParcelStatus::Received) ui::error("Type can only be changed while the status is 'Received'.");
        else m.updateType(pid, chooseType());
    } else if (f == 3) {
        ParcelStatus s;
        if (chooseManualStatus(s)) { cout << "\n"; m.updateStatus(pid, s); }
    }
}

static void doRemove(ParcelModule& m) {
    ui::section("REMOVE PARCEL");
    m.displayAll();
    cout << "\n";
    string pid = readId("Parcel ID to remove");
    Parcel* p = m.findParcel(pid);
    if (!p) { ui::error("Parcel " + pid + " not found."); return; }
    ui::parcelCard(*p, "RECORD TO REMOVE");
    cout << "\n";
    if (confirm("Delete this parcel permanently?")) m.removeParcel(pid);
    else ui::info("Removal cancelled.");
}

int main() {
    ParcelModule module;
    int loaded = module.loadFromFile(DATA_FILE);
    ui::clearScreen();
    if (loaded > 0) cout << "\n  [OK]    Loaded " << loaded << " parcel(s) from " << DATA_FILE << "\n";

    while (true) {
        showMenu(module);
        int c = readInt("Choose an option", 0, 10);
        if (c == 0) break;
        bool changed = false;
        switch (c) {
            case 1:  doRegister(module); changed = true; break;
            case 2:  ui::section("ALL PARCELS"); module.displayAll(); break;
            case 3:  ui::section("SEARCH BY PARCEL ID"); module.searchById(readId("Parcel ID")); break;
            case 4:  ui::section("SEARCH BY RECIPIENT ID"); module.searchByRecipient(readId("Recipient ID")); break;
            case 5:  ui::section("INTAKE QUEUE (FIFO)"); module.displayIntakeQueue(); break;
            case 6:  ui::section("SORT INCOMING PARCELS"); module.sortIncoming(); changed = true; break;
            case 7:  ui::section("PROCESSING ORDER (PRIORITY QUEUE)"); module.displayProcessQueue(); break;
            case 8:  doProcess(module); changed = true; break;
            case 9:  doUpdate(module); changed = true; break;
            case 10: doRemove(module); changed = true; break;
        }
        if (changed) module.saveToFile(DATA_FILE);
        ui::pause();
        ui::clearScreen();
    }
    module.saveToFile(DATA_FILE);
    cout << "\n  [OK]    Data saved to " << DATA_FILE << ". Goodbye!\n\n";
    return 0;
}