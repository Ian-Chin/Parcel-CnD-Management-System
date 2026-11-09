// category_module.hpp - Task 5 (kerjinn)
#ifndef CATEGORY_MODULE_HPP
#define CATEGORY_MODULE_HPP
#include "parcel_tree.hpp"
#include "../task1 (Isac Lim)/parcel_module.hpp"   // shared status-change rule
#include "../task4 (Muhlis)/activity_log.hpp"

namespace task5 {

inline string readLine(const string& prompt) {
    string s; cout << prompt;
    if (!getline(cin, s)) { cout << "\n"; exit(0); }   // input closed
    return trim(s);
}

// Status text -> Task 1 status number (same order as STATUSES)
inline ParcelStatus toStatus(const string& s) {
    int i = indexOf(STATUSES, NUM_STATUSES, s);
    return static_cast<ParcelStatus>(i < 0 ? 1 : i + 1);
}

// Parcel ID: 3 to 12 letters/digits
inline bool validParcelId(const string& s) {
    if (s.size() < 3 || s.size() > 12) return false;
    for (size_t i = 0; i < s.size(); i++) if (!isalnum((unsigned char)s[i])) return false;
    return true;
}
// Recipient ID: same rule as Task 1, 3 to 12 letters/digits, e.g. TP012345
inline bool validRecipientId(const string& s) {
    return validParcelId(s);
}
// Name / company: same rule as Task 1, 1 to 40 characters, no commas
inline bool validText(const string& s) {
    return !s.empty() && s.find(',') == string::npos && s.size() <= 40;
}
inline bool validName(const string& s) {
    return validText(s);
}

// Numbered list; returns chosen item or "" if invalid. If 'current' is given, Enter keeps it.
template <size_t N>
string pickFromList(const char* const (&list)[N], const string& title, const string& current = "") {
    cout << "    " << title << ":\n";
    for (size_t i = 0; i < N; i++) cout << "      [" << i + 1 << "] " << list[i] << "\n";
    string in = readLine(current.empty() ? string("    Choose: ")
                                         : "    Choose (Enter = keep \"" + current + "\"): ");
    if (in.empty() && !current.empty()) return current;
    if (in.size() == 1 && in[0] >= '1' && in[0] < char('1' + N)) return list[in[0] - '1'];
    return "";
}

class CategoryModule {
private:
    ParcelTree tree;      // "records" held in memory
    string dataFile;      // data/parcels.txt
    ParcelGuard* guard;   // Task 2/3 "is this parcel in use?" check (nullptr = standalone)
    ActivityLog* log;     // Task 4 history (nullptr = standalone)

    // True if a request or resource still uses this parcel
    bool inUse(const string& id, string& why) { return guard && guard->isBusy(id, why); }

    bool requireData() {
        if (tree.isEmpty()) {
            cout << "  [!] There are no records loaded. Use Insert > Import from " << dataFile << " first.\n";
            return false;
        }
        return true;
    }
    void fileError() { cout << "  [!] Could not write to " << dataFile << " (check the path).\n"; }

    // ---------- 1. manual insert: records + file ----------
    void insertManual() {
        cout << "\n  --- Enter a New Record Manually ---\n";
        CategoryParcel p;
        p.parcelId = toUpper(readLine("    Parcel ID (e.g. P011): "));
        if (!validParcelId(p.parcelId)) { cout << "  [!] Invalid Parcel ID (must be 3-12 letters/digits).\n"; return; }
        CategoryParcel tmp;
        if (tree.search(p.parcelId)) { cout << "  [!] Parcel ID " << p.parcelId << " already exists in the records.\n"; return; }
        if (ParcelTree::findInFile(dataFile, p.parcelId, tmp)) {
            cout << "  [!] Parcel ID " << p.parcelId << " already exists in " << dataFile << ". Use \"Import ONE record\" instead.\n"; return;
        }
        p.recipientId = toUpper(readLine("    Recipient ID (e.g. TP012345): "));
        if (!validRecipientId(p.recipientId)) { cout << "  [!] Invalid Recipient ID (3-12 letters/digits, no spaces).\n"; return; }
        p.recipientName = readLine("    Recipient Name: ");
        if (!validName(p.recipientName)) { cout << "  [!] Invalid Recipient Name (cannot be empty, longer than 40 or contain commas).\n"; return; }
        p.deliveryCompany = readLine("    Delivery Company: ");
        if (!validText(p.deliveryCompany)) { cout << "  [!] Invalid Delivery Company (no commas, max 40).\n"; return; }
        p.category = pickFromList(CATEGORIES, "Parcel Type");
        if (p.category.empty()) { cout << "  [!] Invalid parcel type choice.\n"; return; }

        p.status  = "Received";                                  // automatic (same start as Task 1)
        p.arrival = currentDateTime();                           // automatic
        int fileSeq = ParcelTree::maxSeqInFile(dataFile);
        int treeSeq = tree.maxSeq();
        p.seq = (fileSeq > treeSeq ? fileSeq : treeSeq) + 1;     // automatic

        if (!ParcelTree::editFile(dataFile, ParcelTree::FILE_APPEND, p.parcelId, &p)) { fileError(); return; }
        tree.insert(p);
        if (log) log->record(ActivityType::Registration, p.parcelId, "-", "-", ParcelStatus::Received, ParcelStatus::Received);
        cout << "\n  [OK] Record added to the records and saved to " << dataFile << " (status: Received):\n";
        printParcelCard(p);
    }

    // ---------- 2. import ONE ----------
    void importOne() {
        cout << "\n  --- Import ONE Record from " << dataFile << " ---\n";
        string id = toUpper(readLine("    Parcel ID to import: "));
        if (!validParcelId(id)) { cout << "  [!] Invalid Parcel ID (must be 3-12 letters/digits).\n"; return; }
        if (tree.search(id)) { cout << "  [!] Parcel " << id << " is already in the records.\n"; return; }
        CategoryParcel p;
        if (!ParcelTree::findInFile(dataFile, id, p)) { cout << "  [!] Parcel " << id << " does not exist in " << dataFile << ".\n"; return; }
        tree.insert(p);
        cout << "  [OK] Record imported:\n"; printParcelCard(p);
    }

    // ---------- 3. import ALL ----------
    void importAll() {
        cout << "\n  --- Import ALL Records from " << dataFile << " ---\n";
        int skipped = 0;
        int n = tree.importAll(dataFile, skipped);
        if (n < 0)       cout << "  [!] Cannot open " << dataFile << ".\n";
        else if (n == 0 && skipped == 0) cout << "  [!] " << dataFile << " contains no records.\n";
        else cout << "  [OK] Imported " << n << " record(s); skipped " << skipped << " already in the records.\n"
                  << "       Records now: " << tree.size() << "\n";
    }

    // ---------- 4. clear ----------
    void clearRecords() {
        cout << "\n  --- Clear Records ---\n";
        if (tree.isEmpty()) { cout << "  [!] There are no records to clear.\n"; return; }
        cout << "  This removes all " << tree.size() << " record(s) from memory. " << dataFile << " is NOT changed.\n";
        if (toUpper(readLine("    Confirm clear? (Y/N): ")) != "Y") { cout << "  Cancelled.\n"; return; }
        tree.clear();
        cout << "  [OK] Records cleared (now 0).\n";
    }

    // ---------- report printing ----------
    void report(const string& title, const string& note, int mode) {
        cout << "\n  " << title << "\n  " << note << "\n\n";
        printParcelHeader();
        int cat[NUM_CATEGORIES];
        int total = (mode == 1) ? tree.displayInorder(cat) : (mode == 2) ? tree.displayPreorder(cat) : tree.displayPostorder(cat);
        cout << "  " << string(119, '-') << "\n  Total records: " << total << "\n\n  Summary by type:\n";
        for (int i = 0; i < NUM_CATEGORIES; i++)
            cout << "    " << left << setw(18) << CATEGORIES[i] << ": " << cat[i] << "\n";
    }

public:
    CategoryModule(const string& file, ParcelGuard* g = nullptr, ActivityLog* l = nullptr)
        : dataFile(file), guard(g), log(l) {
        int skipped = 0;
        if (tree.importAll(dataFile, skipped) < 0)
            cout << "  [!] Could not load records from " << dataFile << ".\n";
    }
    size_t recordCount() const { return tree.size(); }
    int treeHeight() const { return tree.height(); }

    // ---------- INSERT submenu ----------
    void insertMenu() {
        string c;
        do {
            cout << "\n  --- INSERT PARCEL RECORD ---\n"
                 << "    [1] Enter a new record manually\n"
                 << "    [2] Import ONE record from parcels.txt\n"
                 << "    [3] Import ALL records from parcels.txt\n"
                 << "    [4] Clear records\n\n"
                 << "    [0] Back\n";
            c = readLine("\n  Choose: ");
            if      (c == "1") insertManual();
            else if (c == "2") importOne();
            else if (c == "3") importAll();
            else if (c == "4") clearRecords();
            else if (c != "0") cout << "  [!] Invalid option.\n";
            if (c != "0") readLine("\n  Press Enter to continue...");
        } while (c != "0");
    }

    // ---------- SEARCH ----------
    void searchById() {
        cout << "\n  --- Search by Parcel ID ---\n";
        if (!requireData()) return;
        string id = toUpper(readLine("    Parcel ID: "));
        if (!validParcelId(id)) { cout << "  [!] Invalid Parcel ID (must be 3-12 letters/digits).\n"; return; }
        CategoryParcel* p = tree.search(id);
        if (!p) cout << "  [!] Parcel " << id << " does not exist in the records.\n";
        else { cout << "  [OK] Record found:\n"; printParcelCard(*p); }
    }

    void searchByCategory() {
        cout << "\n  --- Search by Parcel Type ---\n";
        if (!requireData()) return;
        string key = pickFromList(CATEGORIES, "Parcel Type");
        if (key.empty()) { cout << "  [!] Invalid parcel type choice.\n"; return; }
        cout << "\n  Results for type: " << key << "\n"; printParcelHeader();
        int n = tree.searchByCategory(key);
        if (n == 0) cout << "  [!] No parcels of type \"" << key << "\" exist in the records.\n";
        else        cout << "  " << string(119, '-') << "\n  " << n << " parcel(s) found.\n";
    }

    // ---------- DISPLAY ----------
    void displayRecords() {
        cout << "\n  --- Display All Parcels ---\n";
        if (!requireData()) return;
        cout << "    [1] In-order       Left-Root-Right   sorted by Parcel ID  (recommended for staff reports)\n"
             << "    [2] Pre-order      Root-Left-Right   parent before children (shows how the tree was built)\n"
             << "    [3] Post-order     Left-Right-Root   children before parent (order used to free the tree)\n"
             << "    [4] Tree structure                   BST diagram\n\n"
             << "    [0] Back\n";
        string c = readLine("    Choose: ");
        if (c == "0")
            return;
        if (c == "1")
            report("PARCEL REPORT - IN-ORDER TRAVERSAL",
                   "Visits Left -> Root -> Right, so records are listed in ascending Parcel ID order.", 1);
        else if (c == "2")
            report("PARCEL LIST - PRE-ORDER TRAVERSAL",
                   "Visits Root -> Left -> Right: each parent appears before its children (insertion/backup order).", 2);
        else if (c == "3")
            report("PARCEL LIST - POST-ORDER TRAVERSAL",
                   "Visits Left -> Right -> Root: each parent appears after its children.", 3);
        else if (c == "4") {
            cout << "\n  PARCEL ORGANISATION STRUCTURE (BST)\n"
                 << "  Nodes: " << tree.size() << "   Height: " << tree.height() << "\n"
                 << "  Root is at the left; right children are drawn above (/--), left children below (\\--).\n\n";
            tree.displayStructure();
        }
        else cout << "  [!] Invalid choice.\n";
    }

    // ---------- UPDATE: records + file ----------
    void updateParcel() {
        cout << "\n  --- Update Parcel ---\n";
        if (!requireData()) return;
        string id = toUpper(readLine("    Parcel ID to update: "));
        if (!validParcelId(id)) { cout << "  [!] Invalid Parcel ID (must be 3-12 letters/digits).\n"; return; }
        CategoryParcel* cur = tree.search(id);
        if (!cur) {
            CategoryParcel tmp;
            if (ParcelTree::findInFile(dataFile, id, tmp))
                cout << "  [!] Parcel " << id << " is in " << dataFile << " but not in the records. Import it first (Insert > option 2).\n";
            else cout << "  [!] Cannot update: Parcel " << id << " does not exist.\n";
            return;
        }
        cout << "  Current record:\n"; printParcelCard(*cur);
        cout << "  Enter new details (press Enter to keep the current value; Parcel ID cannot be changed):\n";

        CategoryParcel np = *cur;
        string in = toUpper(readLine("    Recipient ID [" + cur->recipientId + "]: "));
        if (!in.empty()) { if (!validRecipientId(in)) { cout << "  [!] Invalid Recipient ID (3-12 letters/digits, no spaces).\n"; return; } np.recipientId = in; }
        in = readLine("    Recipient Name [" + cur->recipientName + "]: ");
        if (!in.empty()) { if (!validName(in)) { cout << "  [!] Invalid Recipient Name.\n"; return; } np.recipientName = in; }
        in = readLine("    Delivery Company [" + cur->deliveryCompany + "]: ");
        if (!in.empty()) { if (!validText(in)) { cout << "  [!] Invalid Delivery Company.\n"; return; } np.deliveryCompany = in; }
        np.category = pickFromList(CATEGORIES, "Parcel Type", cur->category);
        if (np.category.empty()) { cout << "  [!] Invalid parcel type choice.\n"; return; }
        np.status = pickFromList(STATUSES, "Parcel Status", cur->status);
        if (np.status.empty()) { cout << "  [!] Invalid status choice.\n"; return; }

        // Same rules as Task 1, so the other modules stay consistent
        bool statusChanged  = np.status != cur->status;
        bool detailsChanged = np.recipientId != cur->recipientId || np.recipientName != cur->recipientName ||
                              np.deliveryCompany != cur->deliveryCompany || np.category != cur->category;
        if (!statusChanged && !detailsChanged) { cout << "  [!] Nothing was changed.\n"; return; }
        if (np.category != cur->category && cur->status != "Received") {
            cout << "  [!] Parcel type can only be changed while the status is Received.\n"; return;
        }
        string why;
        if (np.recipientId != cur->recipientId && inUse(id, why)) {
            cout << "  [!] Cannot change the Recipient ID: " << why << ".\n"; return;
        }
        if (statusChanged) {
            ParcelStatus oldS = toStatus(cur->status), newS = toStatus(np.status);
            if (isTerminalStatus(oldS)) { cout << "  [!] Parcel is already " << cur->status << ", its status cannot be changed.\n"; return; }
            if (newS == ParcelStatus::Received || newS == ParcelStatus::Pending || newS == ParcelStatus::Processing) {
                cout << "  [!] " << np.status << " is set by Task 1 sorting/processing, not by hand.\n"; return;
            }
            if (!ParcelModule::isAllowedTransition(oldS, newS)) {
                cout << "  [!] Invalid change: " << cur->status << " -> " << np.status << ".\n"
                     << "      From " << cur->status << " you can only go to: " << ParcelModule::allowedTargets(oldS) << ".\n";
                return;
            }
            if (inUse(id, why)) { cout << "  [!] Cannot change the status: " << why << ". Finish or cancel that job first.\n"; return; }
        }

        if (!ParcelTree::editFile(dataFile, ParcelTree::FILE_UPDATE, id, &np)) { fileError(); return; }
        if (log) {
            if (detailsChanged) log->record(ActivityType::ParcelUpdate, id, "-", "-", toStatus(cur->status), toStatus(cur->status));
            if (statusChanged)  log->record(ActivityType::StatusChange, id, "-", "-", toStatus(cur->status), toStatus(np.status));
        }
        *cur = np;
        cout << "\n  [OK] Record updated in the records and in " << dataFile << ":\n"; printParcelCard(*cur);
    }

    // ---------- DELETE: records + file ----------
    void removeParcel() {
        cout << "\n  --- Delete Parcel ---\n";
        if (!requireData()) return;
        string id = toUpper(readLine("    Parcel ID to delete: "));
        if (!validParcelId(id)) { cout << "  [!] Invalid Parcel ID (must be 3-12 letters/digits).\n"; return; }
        CategoryParcel* p = tree.search(id);
        if (!p) {
            CategoryParcel tmp;
            if (ParcelTree::findInFile(dataFile, id, tmp))
                cout << "  [!] Parcel " << id << " is in " << dataFile << " but not in the records. Import it first (Insert > option 2).\n";
            else cout << "  [!] Cannot delete: Parcel " << id << " does not exist.\n";
            return;
        }
        printParcelCard(*p);
        string why;
        if (inUse(id, why)) { cout << "  [!] Cannot delete " << id << ": " << why << ".\n"; return; }
        if (toUpper(readLine("    Confirm delete? (Y/N): ")) != "Y") { cout << "  Cancelled.\n"; return; }
        if (!ParcelTree::editFile(dataFile, ParcelTree::FILE_DELETE, id, nullptr)) { fileError(); return; }
        if (log) log->record(ActivityType::Removal, id, "-", "-", toStatus(p->status), toStatus(p->status));
        tree.remove(id);
        cout << "  [OK] Parcel " << id << " deleted from the records and from " << dataFile << ".\n";
    }
};

// Runs the Task 5 menu and returns to the caller when the user chooses 0.
// Every change is already written to the data file at the moment it is made.
// guard / log connect Task 5 to Tasks 2-4 in the integrated program (nullptr when standalone).
inline void runCategoryModule(const string& file, ParcelGuard* guard = nullptr, ActivityLog* log = nullptr) {
    CategoryModule module(file, guard, log);
    string c;
    do {
        cout << "\n  ================================================================\n"
             << "                     SMART CAMPUS PARCEL SYSTEM\n"
             << "                 Task 5 - Parcel Category Organisation\n"
             << "  ================================================================\n"
             << "  Records: " << module.recordCount() << "  |  Tree height: " << module.treeHeight() << "\n"
             << "  ----------------------------------------------------------------\n"
             << "     [1]  Insert parcel record   (manual / import / clear)\n"
             << "     [2]  Search by Parcel ID\n"
             << "     [3]  Search by Parcel Type\n"
             << "     [4]  Display all parcels    (In/Pre/Post-order, tree structure)\n"
             << "     [5]  Update parcel\n"
             << "     [6]  Delete parcel\n\n"
             << "     [0]  Back\n"
             << "  ----------------------------------------------------------------\n";
        c = readLine("\n  Choose an option: ");
        if      (c == "1") module.insertMenu();
        else if (c == "2") module.searchById();
        else if (c == "3") module.searchByCategory();
        else if (c == "4") module.displayRecords();
        else if (c == "5") module.updateParcel();
        else if (c == "6") module.removeParcel();
        else if (c != "0") cout << "  [!] Invalid option.\n";
        if (c != "0" && c != "1") readLine("\n  Press Enter to continue...");
    } while (c != "0");
    cout << "\n  [OK] " << file << " is up to date. Returning to the main menu.\n";
}

} // namespace task5
#endif