// parcel_tree.hpp - Task 5 (kerjinn)
// ParcelTree: Binary Search Tree of parcels (key = Parcel ID) + helpers that
// read / edit single lines of data/parcels.txt.
#ifndef PARCEL_TREE_HPP
#define PARCEL_TREE_HPP
#include "parcel_record.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>

namespace task5 {

class ParcelTree {
private:
    struct Node {
        CategoryParcel data;
        Node *left, *right;
        Node(const CategoryParcel& d) : data(d), left(nullptr), right(nullptr) {}
    };
    Node* root;
    int count;

    // ---------- BST helpers ----------
    Node* insertNode(Node* cur, const CategoryParcel& p, bool& ok) {
        if (!cur) { ok = true; return new Node(p); }
        if (p.parcelId < cur->data.parcelId)      cur->left  = insertNode(cur->left,  p, ok);
        else if (p.parcelId > cur->data.parcelId) cur->right = insertNode(cur->right, p, ok);
        else ok = false;
        return cur;
    }
    Node* findNode(Node* cur, const string& id) const {
        while (cur && cur->data.parcelId != id)
            cur = (id < cur->data.parcelId) ? cur->left : cur->right;
        return cur;
    }
    Node* minNode(Node* cur) const { while (cur->left) cur = cur->left; return cur; }
    Node* removeNode(Node* cur, const string& id, bool& found) {
        if (!cur) return nullptr;
        if (id < cur->data.parcelId)      cur->left  = removeNode(cur->left,  id, found);
        else if (id > cur->data.parcelId) cur->right = removeNode(cur->right, id, found);
        else {
            found = true;
            if (!cur->left)  { Node* r = cur->right; delete cur; return r; }
            if (!cur->right) { Node* l = cur->left;  delete cur; return l; }
            Node* succ = minNode(cur->right);
            cur->data = succ->data;
            bool dummy = false;
            cur->right = removeNode(cur->right, succ->data.parcelId, dummy);
        }
        return cur;
    }
    int heightOf(Node* n) const {
        if (!n) return 0;
        int l = heightOf(n->left), r = heightOf(n->right);
        return 1 + (l > r ? l : r);
    }
    int maxSeqOf(Node* n) const {
        if (!n) return 0;
        int m = n->data.seq, l = maxSeqOf(n->left), r = maxSeqOf(n->right);
        if (l > m) m = l;
        if (r > m) m = r;
        return m;
    }
    void destroy(Node* n) { if (!n) return; destroy(n->left); destroy(n->right); delete n; }

    // ---------- the three standard recursive traversals ----------
    void visit(const CategoryParcel& p, int& no, int* cat) const {
        printParcelRow(p, ++no);
        int c = indexOf(CATEGORIES, NUM_CATEGORIES, p.category);
        if (c >= 0) cat[c]++;
    }
    // Left -> Node -> Right   (sorted by Parcel ID)
    void inorder(Node* n, int& no, int* cat) const {
        if (!n) return;
        inorder(n->left, no, cat);
        visit(n->data, no, cat);
        inorder(n->right, no, cat);
    }
    // Node -> Left -> Right   (parents before children)
    void preorder(Node* n, int& no, int* cat) const {
        if (!n) return;
        visit(n->data, no, cat);
        preorder(n->left, no, cat);
        preorder(n->right, no, cat);
    }
    // Left -> Right -> Node   (children before parents)
    void postorder(Node* n, int& no, int* cat) const {
        if (!n) return;
        postorder(n->left, no, cat);
        postorder(n->right, no, cat);
        visit(n->data, no, cat);
    }

    // ---------- BST structure drawing (right subtree on top, left below) ----------
    void drawTree(Node* n, int depth, char side) const {
        if (!n) return;
        drawTree(n->right, depth + 1, '/');
        cout << "    " << string(depth * 8, ' ');
        if (side == '/')       cout << "/-- ";
        else if (side == '\\') cout << "\\-- ";
        cout << n->data.parcelId << "\n";
        drawTree(n->left, depth + 1, '\\');
    }

    void matchCategory(Node* n, const string& key, int& no) const {
        if (!n) return;
        matchCategory(n->left, key, no);
        if (n->data.category == key) printParcelRow(n->data, ++no);
        matchCategory(n->right, key, no);
    }

public:
    enum FileOp { FILE_APPEND, FILE_UPDATE, FILE_DELETE };

    ParcelTree() : root(nullptr), count(0) {}
    ~ParcelTree() { destroy(root); }

    bool isEmpty() const { return root == nullptr; }
    int  size()    const { return count; }
    int  height()  const { return heightOf(root); }
    int  maxSeq()  const { return maxSeqOf(root); }

    bool insert(const CategoryParcel& p) {
        bool ok = false;
        root = insertNode(root, p, ok);
        if (ok) count++;
        return ok;
    }
    CategoryParcel* search(const string& id) {
        Node* n = findNode(root, id);
        return n ? &n->data : nullptr;
    }
    bool remove(const string& id) {
        bool found = false;
        root = removeNode(root, id, found);
        if (found) count--;
        return found;
    }
    void clear() { destroy(root); root = nullptr; count = 0; }

    // Each prints rows, fills cat[NUM_CATEGORIES], returns number of records
    int displayInorder(int* cat)   const { int no = 0; for (int i = 0; i < NUM_CATEGORIES; i++) cat[i] = 0; inorder(root, no, cat);   return no; }
    int displayPreorder(int* cat)  const { int no = 0; for (int i = 0; i < NUM_CATEGORIES; i++) cat[i] = 0; preorder(root, no, cat);  return no; }
    int displayPostorder(int* cat) const { int no = 0; for (int i = 0; i < NUM_CATEGORIES; i++) cat[i] = 0; postorder(root, no, cat); return no; }
    void displayStructure() const { drawTree(root, 0, ' '); }
    int searchByCategory(const string& key) const { int no = 0; matchCategory(root, key, no); return no; }

    // ---------- data/parcels.txt helpers ----------
    static string fileHeader() {
        return "# ParcelID,RecipientID,RecipientName,Company,Type(1-5),Arrival,Status(1-9),Seq\n"
               "# Type: 1=Standard 2=Large 3=Fragile 4=Document 5=Special-Handling\n"
               "# Status: 1=Received 2=Pending 3=Processing 4=Ready 5=OutForDelivery 6=Collected 7=Returned 8=Missing 9=Cancelled\n";
    }
    static string toLine(const CategoryParcel& p) {
        int type = indexOf(CATEGORIES, NUM_CATEGORIES, p.category);
        int stat = indexOf(STATUSES, NUM_STATUSES, p.status);
        ostringstream o;
        o << p.parcelId << ',' << p.recipientId << ',' << p.recipientName << ','
          << p.deliveryCompany << ',' << (type < 0 ? 1 : type + 1) << ',' << p.arrival << ','
          << (stat < 0 ? 1 : stat + 1) << ',' << p.seq;
        return o.str();
    }
    static bool parseLine(const string& raw, CategoryParcel& p) {
        string line = trim(raw);
        if (line.empty() || line[0] == '#') return false;
        stringstream ss(line);
        string f[9]; int k = 0;
        while (k < 9 && getline(ss, f[k], ',')) { f[k] = trim(f[k]); k++; }
        // Same rules as Task 1's loader, so both modules hold the same parcels:
        // exactly 8 fields, known type / status, seq >= 1, no empty fields.
        if (k != 8) return false;
        for (int i = 0; i < 8; i++) if (f[i].empty()) return false;
        if (f[0].size() < 3 || f[0].size() > 12) return false;
        for (size_t i = 0; i < f[0].size(); i++) if (!isalnum((unsigned char)f[0][i])) return false;
        int type = atoi(f[4].c_str()), stat = atoi(f[6].c_str()), seq = atoi(f[7].c_str());
        if (type < 1 || type > NUM_CATEGORIES || stat < 1 || stat > NUM_STATUSES || seq < 1) return false;
        p.parcelId = toUpper(f[0]);  p.recipientId = f[1];
        p.recipientName = f[2];      p.deliveryCompany = f[3];
        p.category = CATEGORIES[type - 1];
        p.arrival = f[5];
        p.status = STATUSES[stat - 1];
        p.seq = seq;
        return true;
    }
    static bool findInFile(const string& file, const string& id, CategoryParcel& out) {
        ifstream in(file.c_str());
        string line; CategoryParcel p;
        while (getline(in, line))
            if (parseLine(line, p) && p.parcelId == id) { out = p; return true; }
        return false;
    }
    static int maxSeqInFile(const string& file) {
        ifstream in(file.c_str());
        string line; CategoryParcel p; int m = 0;
        while (getline(in, line)) if (parseLine(line, p) && p.seq > m) m = p.seq;
        return m;
    }
    // Edits ONE record line in the file and leaves every other line untouched.
    //   FILE_APPEND : add p at the end (creates the file if missing)
    //   FILE_UPDATE : replace the line with this Parcel ID by p (appends if not found)
    //   FILE_DELETE : remove the line with this Parcel ID
    static bool editFile(const string& file, FileOp op, const string& id, const CategoryParcel* p) {
        string tmp = file + ".tmp";
        ifstream in(file.c_str());
        if (!in && op != FILE_APPEND) return false;
        ofstream out(tmp.c_str());
        if (!out) return false;
        bool done = false;
        if (!in) out << fileHeader();
        else {
            string line;
            while (getline(in, line)) {
                string t = trim(line);
                if (op != FILE_APPEND && !t.empty() && t[0] != '#') {
                    string first = toUpper(trim(t.substr(0, t.find(','))));
                    if (first == id) {
                        if (op == FILE_UPDATE && p && !done) out << toLine(*p) << "\n";
                        done = true;
                        continue;
                    }
                }
                out << line << "\n";
            }
            in.close();
        }
        if (p && (op == FILE_APPEND || (op == FILE_UPDATE && !done))) out << toLine(*p) << "\n";
        bool good = out.good();
        out.close();
        if (!good) { ::remove(tmp.c_str()); return false; }
        ::remove(file.c_str());
        return ::rename(tmp.c_str(), file.c_str()) == 0;
    }

    // Reads every record of the file into the tree. Records already in the tree are skipped.
    int importAll(const string& file, int& skipped) {
        skipped = 0;
        ifstream in(file.c_str());
        if (!in) return -1;
        string line; CategoryParcel p; int added = 0;
        while (getline(in, line)) {
            if (!parseLine(line, p)) continue;
            if (insert(p)) added++; else skipped++;
        }
        return added;
    }
};
} // namespace task5
#endif