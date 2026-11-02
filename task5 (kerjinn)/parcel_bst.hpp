// parcel_bst.hpp - Task 5 (kerjinn)
#ifndef PARCEL_BST_HPP
#define PARCEL_BST_HPP
#include "parcel_record.hpp"
#include <fstream>
#include <sstream>

namespace task5 {

class ParcelBST {
private:
    struct Node {
        CategoryParcel data;
        Node *left, *right;
        Node(const CategoryParcel& d) : data(d), left(nullptr), right(nullptr) {}
    };
    struct QNode { Node* n; QNode* next; };
    Node* root;
    int count;

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
    void inorder(Node* n, int& i) const   { if (!n) return; inorder(n->left, i);   printParcelRow(n->data); i++; inorder(n->right, i); }
    void preorder(Node* n, int& i) const  { if (!n) return; printParcelRow(n->data); i++; preorder(n->left, i); preorder(n->right, i); }
    void postorder(Node* n, int& i) const { if (!n) return; postorder(n->left, i); postorder(n->right, i); printParcelRow(n->data); i++; }
    void destroy(Node* n) { if (!n) return; destroy(n->left); destroy(n->right); delete n; }
    
    int matchCategory(Node* n, const string& key, bool byService) const {
        if (!n) return 0;
        int c = matchCategory(n->left, key, byService);
        const string& f = byService ? n->data.serviceType : n->data.category;
        if (f == key) { printParcelRow(n->data); c++; }
        return c + matchCategory(n->right, key, byService);
    }

    // Save node in Task 1 CSV format: ParcelID,RecipientID,RecipientName,Company,Type(1-5),Arrival,Status(1-9),Seq
    void saveNode(Node* n, ofstream& out) const {
        if (!n) return;
        const CategoryParcel& p = n->data;

        int typeNum = 1;
        if (p.category == "Large Parcel") typeNum = 2;
        else if (p.category == "Fragile Parcel") typeNum = 3;
        else if (p.category == "Document") typeNum = 4;
        else if (p.category == "Special Handling") typeNum = 5;

        int statusNum = 1;
        if (p.status == "Pending") statusNum = 2;
        else if (p.status == "Processing") statusNum = 3;
        else if (p.status == "Ready for Collection") statusNum = 4;
        else if (p.status == "Out for Delivery") statusNum = 5;
        else if (p.status == "Collected") statusNum = 6;
        else if (p.status == "Returned") statusNum = 7;
        else if (p.status == "Missing") statusNum = 8;
        else if (p.status == "Cancelled") statusNum = 9;

        out << p.parcelId << ',' << p.recipientId << ',' << p.recipientName << ','
            << p.deliveryCompany << ',' << typeNum << ',' << p.arrival << ','
            << statusNum << ",0\n";
        
        saveNode(n->left, out); 
        saveNode(n->right, out);
    }

public:
    ParcelBST() : root(nullptr), count(0) {}
    ~ParcelBST() { destroy(root); }

    bool isEmpty() const { return root == nullptr; }
    int  size()    const { return count; }
    int  height()  const { return heightOf(root); }

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

    int displayInorder()   const { int i = 0; inorder(root, i);   return i; }
    int displayPreorder()  const { int i = 0; preorder(root, i);  return i; }
    int displayPostorder() const { int i = 0; postorder(root, i); return i; }

    void displayLevelOrder() const {
        if (!root) return;
        QNode *front = new QNode{root, nullptr}, *back = front;
        int level = 0;
        while (front) {
            int width = 0;
            for (QNode* q = front; q; q = q->next) width++;
            cout << "    Level " << level++ << " : ";
            for (int k = 0; k < width; k++) {
                QNode* q = front; front = front->next; if (!front) back = nullptr;
                Node* n = q->n; delete q;
                cout << n->data.parcelId << "   ";
                if (n->left)  { QNode* t = new QNode{n->left,  nullptr}; if (back) back->next = t; else front = t; back = t; }
                if (n->right) { QNode* t = new QNode{n->right, nullptr}; if (back) back->next = t; else front = t; back = t; }
            }
            cout << "\n";
        }
    }

    void drawTree(Node* n, int depth) const {
        if (!n) return;
        drawTree(n->right, depth + 1);
        cout << "    " << string(depth * 6, ' ') << n->data.parcelId << "\n";
        drawTree(n->left, depth + 1);
    }
    void displayStructure() const { drawTree(root, 0); }

    int searchByCategory(const string& key, bool byService = false) const { return matchCategory(root, key, byService); }

    bool saveToFile(const string& file) const {
        ofstream out(file.c_str());
        if (!out) return false;
        out << "# ParcelID,RecipientID,RecipientName,Company,Type(1-5),Arrival,Status(1-9),Seq\n";
        saveNode(root, out);
        return true;
    }

    int loadFromFile(const string& file) {
        ifstream in(file.c_str());
        if (!in) return 0;
        string line; int loaded = 0;
        while (getline(in, line)) {
            if (line.empty() || line[0] == '#') continue;
            stringstream ss(line);
            string f[8]; int k = 0;
            while (k < 8 && getline(ss, f[k], ',')) k++;
            if (k < 8) continue;

            CategoryParcel p;
            p.parcelId = f[0]; 
            p.recipientId = f[1]; 
            p.recipientName = f[2];
            p.deliveryCompany = f[3];

            // Map numeric type (1-5) to category string[cite: 2]
            int typeNum = stoi(f[4]);
            static const char* catMap[] = {"Standard Parcel", "Large Parcel", "Fragile Parcel", "Document", "Special Handling"};
            if (typeNum >= 1 && typeNum <= 5) p.category = catMap[typeNum - 1];
            else p.category = "Standard Parcel";

            p.serviceType = "Recipient Collection";

            // Map numeric status (1-9) to status string[cite: 2]
            int statusNum = stoi(f[6]);
            static const char* statMap[] = {"Received", "Pending", "Processing", "Ready for Collection", "Out for Delivery", "Collected", "Returned", "Missing", "Cancelled"};
            if (statusNum >= 1 && statusNum <= 9) p.status = statMap[statusNum - 1];
            else p.status = "Received";

            p.arrival = f[5];

            if (insert(p)) loaded++;
        }
        return loaded;
    }
};
} // namespace task5
#endif