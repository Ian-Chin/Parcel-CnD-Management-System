// category_module.hpp - Task 5 (kerjinn)
#ifndef CATEGORY_MODULE_HPP
#define CATEGORY_MODULE_HPP
#include "parcel_bst.hpp"

namespace task5 {

static const char* CATEGORIES[] = {"Standard Parcel", "Large Parcel", "Fragile Parcel", "Document", "Special Handling"};
static const char* SERVICES[]   = {"Recipient Collection", "Campus Delivery", "Special Handling", "Return to Sender", "Re-delivery"};
static const char* STATUSES[]   = {"Received", "Pending", "Processing", "Ready for Collection", "Out for Delivery", "Collected", "Returned", "Missing", "Cancelled"};

inline string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
inline string readLine(const string& prompt) {
    string s; cout << prompt; getline(cin, s); return trim(s);
}
inline string toUpper(string s) { for (size_t i = 0; i < s.size(); i++) s[i] = toupper((unsigned char)s[i]); return s; }

inline bool validId(const string& s) {
    if (s.size() < 2 || s.size() > 10) return false;
    for (size_t i = 0; i < s.size(); i++) if (!isalnum((unsigned char)s[i])) return false;
    return true;
}
inline bool validText(const string& s) {
    return !s.empty() && s.find(',') == string::npos && s.size() <= 40;
}
inline bool validName(const string& s) {
    if (!validText(s)) return false;
    for (size_t i = 0; i < s.size(); i++)
        if (!isalpha((unsigned char)s[i]) && s[i] != ' ' && s[i] != '\'' && s[i] != '.' && s[i] != '-' && s[i] != '/') return false;
    return true;
}
template <size_t N>
string pickFromList(const char* (&list)[N], const string& title) {
    cout << "    " << title << ":\n";
    for (size_t i = 0; i < N; i++) cout << "      [" << i + 1 << "] " << list[i] << "\n";
    string in = readLine("    Choose: ");
    if (in.size() == 1 && in[0] >= '1' && in[0] < char('1' + N)) return list[in[0] - '1'];
    return "";
}

class CategoryModule {
private:
    ParcelBST tree;
    string dataFile;

    void pause() { readLine("\n  Press Enter to continue..."); }
    void save()  { tree.saveToFile(dataFile); }
    bool requireData() {
        if (tree.isEmpty()) { cout << "  [!] The parcel organisation structure is empty.\n"; return false; }
        return true;
    }

    bool inputParcel(CategoryParcel& p, bool newRecord) {
        if (newRecord) {
            p.parcelId = toUpper(readLine("    Parcel ID (e.g. P011): "));
            if (!validId(p.parcelId)) { cout << "  [!] Invalid Parcel ID (2-10 letters/digits).\n"; return false; }
            if (tree.search(p.parcelId)) { cout << "  [!] Duplicate Parcel ID: " << p.parcelId << " already exists.\n"; return false; }
        }
        p.recipientId = toUpper(readLine("    Recipient ID: "));
        if (!validId(p.recipientId)) { cout << "  [!] Invalid Recipient ID.\n"; return false; }
        p.recipientName = readLine("    Recipient Name: ");
        if (!validName(p.recipientName)) { cout << "  [!] Invalid Recipient Name.\n"; return false; }
        p.deliveryCompany = readLine("    Delivery Company: ");
        if (!validText(p.deliveryCompany)) { cout << "  [!] Invalid Delivery Company.\n"; return false; }
        p.category = pickFromList(CATEGORIES, "Parcel Category");
        if (p.category.empty()) { cout << "  [!] Invalid category choice.\n"; return false; }
        p.serviceType = pickFromList(SERVICES, "Service Type");
        if (p.serviceType.empty()) { cout << "  [!] Invalid service choice.\n"; return false; }
        p.status = pickFromList(STATUSES, "Parcel Status");
        if (p.status.empty()) { cout << "  [!] Invalid status choice.\n"; return false; }
        if (newRecord) p.arrival = currentDateTime();
        return true;
    }

public:
    // Ensure the default parameter points to Task 1's file
    CategoryModule(const string& file = "task1 (Isac Lim)/parcels.txt") : dataFile(file) {
        int n = tree.loadFromFile(dataFile);
        if (n > 0) cout << "  Loaded " << n << " parcel record(s) from Task 1 file into the BST.\n";
    }

    bool syncInsert(const CategoryParcel& p) { bool ok = tree.insert(p); if (ok) save(); return ok; }
    bool syncStatus(const string& id, const string& status) {
        CategoryParcel* p = tree.search(id); if (!p) return false;
        p->status = status; save(); return true;
    }
    bool syncRemove(const string& id) { bool ok = tree.remove(id); if (ok) save(); return ok; }

    void insertParcel() {
        cout << "\n  --- Insert New Parcel Record ---\n";
        CategoryParcel p;
        if (!inputParcel(p, true)) return;
        tree.insert(p); save();
        cout << "\n  [OK] Parcel inserted into BST and synced to Task 1 file:\n"; printParcelCard(p);
    }

    void searchById() {
        cout << "\n  --- Search by Parcel ID ---\n";
        if (!requireData()) return;
        string id = toUpper(readLine("    Parcel ID: "));
        if (!validId(id)) { cout << "  [!] Invalid Parcel ID.\n"; return; }
        CategoryParcel* p = tree.search(id);
        if (!p) cout << "  [!] Parcel " << id << " not found.\n";
        else { cout << "  [OK] Record found:\n"; printParcelCard(*p); }
    }

    void searchByCategory() {
        cout << "\n  --- Search by Category / Service Type ---\n";
        if (!requireData()) return;
        cout << "    [1] Parcel Category   [2] Service Type\n";
        string c = readLine("    Search by: ");
        bool bySvc = (c == "2");
        if (c != "1" && c != "2") { cout << "  [!] Invalid choice.\n"; return; }
        string key = bySvc ? pickFromList(SERVICES, "Service Type") : pickFromList(CATEGORIES, "Parcel Category");
        if (key.empty()) { cout << "  [!] Invalid choice.\n"; return; }
        cout << "\n"; printParcelHeader();
        int n = tree.searchByCategory(key, bySvc);
        if (n == 0) cout << "  [!] No parcels found for \"" << key << "\".\n";
        else cout << "\n  " << n << " parcel(s) found for \"" << key << "\".\n";
    }

    void displayRecords() {
        cout << "\n  --- Display / Traverse Parcel Records ---\n";
        if (!requireData()) return;
        cout << "    [1] In-order    (sorted by Parcel ID - best for reports)\n"
             << "    [2] Pre-order   (root first - shows how tree was built / backup order)\n"
             << "    [3] Post-order  (children first - order used to safely free the tree)\n"
             << "    [4] Level-order (level by level - shows tree balance)\n";
        string c = readLine("    Traversal: ");
        cout << "\n";
        if (c == "4") { tree.displayLevelOrder(); return; }
        if (c != "1" && c != "2" && c != "3") { cout << "  [!] Invalid choice.\n"; return; }
        printParcelHeader();
        int n = (c == "1") ? tree.displayInorder() : (c == "2") ? tree.displayPreorder() : tree.displayPostorder();
        cout << "\n  Total: " << n << " record(s).\n";
    }

    void showStructure() {
        cout << "\n  --- Parcel Organisation Structure (BST) ---\n";
        if (!requireData()) return;
        cout << "  Nodes: " << tree.size() << "   Height: " << tree.height()
             << "   (root at left, right subtree on top)\n\n";
        tree.displayStructure();
        cout << "\n  Level-order view:\n"; tree.displayLevelOrder();
    }

    void updateParcel() {
        cout << "\n  --- Update Parcel Record ---\n";
        if (!requireData()) return;
        string id = toUpper(readLine("    Parcel ID to update: "));
        if (!validId(id)) { cout << "  [!] Invalid Parcel ID.\n"; return; }
        CategoryParcel* cur = tree.search(id);
        if (!cur) { cout << "  [!] Cannot update: Parcel " << id << " does not exist.\n"; return; }
        cout << "  Current record:\n"; printParcelCard(*cur);
        cout << "  Enter new details (Parcel ID / key cannot be changed):\n";
        CategoryParcel np = *cur;
        if (!inputParcel(np, false)) return;
        np.parcelId = cur->parcelId; np.arrival = cur->arrival;
        *cur = np; save();
        cout << "\n  [OK] Record updated and synced to Task 1 file:\n"; printParcelCard(*cur);
    }

    void removeParcel() {
        cout << "\n  --- Remove Parcel Record ---\n";
        if (!requireData()) return;
        string id = toUpper(readLine("    Parcel ID to remove: "));
        if (!validId(id)) { cout << "  [!] Invalid Parcel ID.\n"; return; }
        CategoryParcel* p = tree.search(id);
        if (!p) { cout << "  [!] Cannot delete: Parcel " << id << " does not exist.\n"; return; }
        printParcelCard(*p);
        if (toUpper(readLine("    Confirm delete? (Y/N): ")) != "Y") { cout << "  Cancelled.\n"; return; }
        tree.remove(id); save();
        cout << "  [OK] Parcel " << id << " removed and synced to Task 1 file.\n";
    }

    void run() {
        string c;
        do {
            cout << "\n  ================================================================\n"
                 << "                     SMART CAMPUS PARCEL SYSTEM\n"
                 << "                 Task 5 - Parcel Category Organisation\n"
                 << "  ================================================================\n\n"
                 << "  Records in BST: " << tree.size() << "  |  Height: " << tree.height() << "\n\n"
                 << "  ----------------------------------------------------------------\n"
                 << "     [1]  Insert new parcel record\n"
                 << "     [2]  Search by Parcel ID                    (BST search)\n"
                 << "     [3]  Search by Category / Service\n"
                 << "     [4]  Display / traverse records     (In/Pre/Post/Level-order)\n"
                 << "     [5]  View tree structure\n"
                 << "     [6]  Update parcel record\n"
                 << "     [7]  Remove parcel record\n\n"
                 << "     [0]  Back (data is saved automatically)\n"
                 << "  ----------------------------------------------------------------\n";
            c = readLine("\n  Choose an option: ");
            if      (c == "1") insertParcel();
            else if (c == "2") searchById();
            else if (c == "3") searchByCategory();
            else if (c == "4") displayRecords();
            else if (c == "5") showStructure();
            else if (c == "6") updateParcel();
            else if (c == "7") removeParcel();
            else if (c != "0") cout << "  [!] Invalid option.\n";
            if (c != "0") pause();
        } while (c != "0");
        save();
    }
};

inline void runCategoryModule(const std::string& file = "task1 (Isac Lim)/parcels.txt") { CategoryModule m(file); m.run(); }} // namespace task5
#endif