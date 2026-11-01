#pragma once
#include <string>
#include <ctime>
#include <cctype>

enum class ParcelType { Standard = 1, Large, Fragile, Document, SpecialHandling };

enum class ParcelStatus {
    Received = 1, Pending, Processing, ReadyForCollection,
    OutForDelivery, Collected, Returned, Missing, Cancelled
};

inline std::string typeToString(ParcelType t) {
    switch (t) {
        case ParcelType::Standard:        return "Standard";
        case ParcelType::Large:           return "Large";
        case ParcelType::Fragile:         return "Fragile";
        case ParcelType::Document:        return "Document";
        case ParcelType::SpecialHandling: return "Special-Handling";
    }
    return "Unknown";
}

inline std::string statusToString(ParcelStatus s) {
    switch (s) {
        case ParcelStatus::Received:           return "Received";
        case ParcelStatus::Pending:            return "Pending";
        case ParcelStatus::Processing:         return "Processing";
        case ParcelStatus::ReadyForCollection: return "Ready for Collection";
        case ParcelStatus::OutForDelivery:     return "Out for Delivery";
        case ParcelStatus::Collected:          return "Collected";
        case ParcelStatus::Returned:           return "Returned";
        case ParcelStatus::Missing:            return "Missing";
        case ParcelStatus::Cancelled:          return "Cancelled";
    }
    return "Unknown";
}

inline bool isTerminalStatus(ParcelStatus s) {
    return s == ParcelStatus::Collected || s == ParcelStatus::Returned ||
           s == ParcelStatus::Cancelled || s == ParcelStatus::Missing;
}

inline int typePriority(ParcelType t) {
    switch (t) {
        case ParcelType::SpecialHandling: return 1;
        case ParcelType::Fragile:         return 2;
        case ParcelType::Large:           return 3;
        case ParcelType::Document:        return 4;
        case ParcelType::Standard:        return 5;
    }
    return 99;
}

struct Parcel {
    std::string  parcelId;
    std::string  recipientId;
    std::string  recipientName;
    std::string  deliveryCompany;
    ParcelType   type;
    std::string  arrivalDateTime;
    ParcelStatus status;
    long         arrivalSeq;
};

inline std::string currentDateTime() {
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buf);
}
