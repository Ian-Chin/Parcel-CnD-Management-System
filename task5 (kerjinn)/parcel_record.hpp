// parcel_record.hpp - Task 5 (kerjinn)
// Parcel record stored inside each BST node + shared lists and print helpers.
#ifndef PARCEL_RECORD_HPP
#define PARCEL_RECORD_HPP
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cctype>
using namespace std;

namespace task5 {

// Same numbering as data/parcels.txt:  Type 1-5  /  Status 1-9
static const int NUM_CATEGORIES = 5;
static const int NUM_STATUSES   = 9;
static const char* const CATEGORIES[NUM_CATEGORIES] = {
    "Standard Parcel", "Large Parcel", "Fragile Parcel", "Document", "Special Handling"};
static const char* const STATUSES[NUM_STATUSES] = {
    "Received", "Pending", "Processing", "Ready for Collection", "Out for Delivery",
    "Collected", "Returned", "Missing", "Cancelled"};

struct CategoryParcel {
    string parcelId;        // BST KEY (unique, 3-12 letters/digits)
    string recipientId;     // e.g. TP012345
    string recipientName;
    string deliveryCompany;
    string category;        // one of CATEGORIES
    string status;          // one of STATUSES
    string arrival;         // YYYY-MM-DD HH:MM:SS
    int    seq;             // Seq column of parcels.txt (kept so Task 1 data is not lost)
    CategoryParcel() : seq(0) {}
};

inline int indexOf(const char* const* list, int n, const string& s) {
    for (int i = 0; i < n; i++) if (s == list[i]) return i;
    return -1;
}

inline string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}

inline string toUpper(string s) { for (size_t i = 0; i < s.size(); i++) s[i] = toupper((unsigned char)s[i]); return s; }

inline string currentDateTime() {
    time_t t = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return string(buf);
}

inline string fit(const string& s, size_t w) { return s.size() > w ? s.substr(0, w) : s; }

inline void printParcelCard(const CategoryParcel& p) {
    cout << "    Parcel ID        : " << p.parcelId << "\n"
         << "    Recipient ID     : " << p.recipientId << "\n"
         << "    Recipient Name   : " << p.recipientName << "\n"
         << "    Delivery Company : " << p.deliveryCompany << "\n"
         << "    Parcel Type      : " << p.category << "\n"
         << "    Status           : " << p.status << "\n"
         << "    Arrival          : " << p.arrival << "\n";
}

inline void printParcelHeader() {
    cout << "  " << left << setw(5) << "No." << setw(11) << "ParcelID" << setw(11) << "RecipientID"
         << setw(21) << "Name" << setw(17) << "Company" << setw(18) << "Type"
         << setw(22) << "Status" << "Arrival\n"
         << "  " << string(119, '-') << "\n";
}

inline void printParcelRow(const CategoryParcel& p, int no) {
    cout << "  " << left << setw(5) << no << setw(11) << p.parcelId << setw(11) << p.recipientId
         << setw(21) << fit(p.recipientName, 20) << setw(17) << fit(p.deliveryCompany, 16)
         << setw(18) << fit(p.category, 17) << setw(22) << fit(p.status, 21) << p.arrival << "\n";
}
} // namespace task5
#endif