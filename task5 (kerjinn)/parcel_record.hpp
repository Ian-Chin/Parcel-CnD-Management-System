// parcel_record.hpp - Task 5 (kerjinn)
// Parcel record stored inside each BST node.
#ifndef PARCEL_RECORD_HPP
#define PARCEL_RECORD_HPP
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cctype>
using namespace std;

namespace task5 {

struct CategoryParcel {
    string parcelId;        // ORGANISATION KEY (unique)
    string recipientId;
    string recipientName;
    string deliveryCompany;
    string category;        // Standard / Large / Fragile / Document / Special Handling
    string serviceType;
    string status;
    string arrival;         // YYYY-MM-DD HH:MM
};

inline string currentDateTime() {
    time_t t = time(nullptr);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&t));
    return string(buf);
}

inline void printParcelCard(const CategoryParcel& p) {
    cout << "    Parcel ID        : " << p.parcelId << "\n"
         << "    Recipient ID     : " << p.recipientId << "\n"
         << "    Recipient Name   : " << p.recipientName << "\n"
         << "    Delivery Company : " << p.deliveryCompany << "\n"
         << "    Parcel Category  : " << p.category << "\n"
         << "    Service Type     : " << p.serviceType << "\n"
         << "    Status           : " << p.status << "\n"
         << "    Arrival          : " << p.arrival << "\n";
}

inline void printParcelRow(const CategoryParcel& p) {
    cout << "  " << left << setw(9) << p.parcelId << setw(11) << p.recipientId
         << setw(18) << p.recipientName.substr(0, 17) << setw(17) << p.category
         << setw(19) << p.serviceType << setw(18) << p.status << p.arrival << "\n";
}

inline void printParcelHeader() {
    cout << "  " << left << setw(9) << "ParcelID" << setw(11) << "Recipient"
         << setw(18) << "Name" << setw(17) << "Category" << setw(19) << "Service"
         << setw(18) << "Status" << "Arrival\n  " << string(100, '-') << "\n";
}
} // namespace task5
#endif