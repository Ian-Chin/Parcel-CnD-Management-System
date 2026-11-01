#pragma once
#include <string>
using namespace std;

// Kinds of resources at the campus delivery centre
enum ResourceType {
    COLLECTION_COUNTER = 1,
    DELIVERY_DESK,
    SPECIAL_COUNTER,
    DELIVERY_PERSONNEL
};

// Current state of a resource
enum ResourceStatus {
    AVAILABLE = 1,
    OCCUPIED,
    UNAVAILABLE,
    MAINTENANCE
};

// Service types (same list as Task 2 service requests)
enum ServiceType {
    RECIPIENT_COLLECTION = 1,
    CAMPUS_DELIVERY,
    SPECIAL_HANDLING,
    RETURN_TO_SENDER,
    RE_DELIVERY
};

// One collection counter / desk / delivery person
struct Resource {
    string resourceId;
    ResourceType type;
    string location;       // "-" if not applicable (e.g. delivery personnel)
    ResourceStatus status;
    string parcelId;       // "-" when nothing is assigned
    string requestId;      // "-" when the job has no service request
    ServiceType service;   // service of the current job
    int transactions;      // number of jobs completed by this resource
};

string resourceTypeToString(ResourceType t) {
    if (t == COLLECTION_COUNTER) return "Collection Counter";
    if (t == DELIVERY_DESK)      return "Delivery Desk";
    if (t == SPECIAL_COUNTER)    return "Special-Handling Counter";
    if (t == DELIVERY_PERSONNEL) return "Delivery Personnel";
    return "Unknown";
}

string resourceStatusToString(ResourceStatus s) {
    if (s == AVAILABLE)   return "Available";
    if (s == OCCUPIED)    return "Occupied";
    if (s == UNAVAILABLE) return "Unavailable";
    if (s == MAINTENANCE) return "Maintenance";
    return "Unknown";
}

string serviceToString(ServiceType s) {
    if (s == RECIPIENT_COLLECTION) return "Recipient Collection";
    if (s == CAMPUS_DELIVERY)      return "Campus Delivery";
    if (s == SPECIAL_HANDLING)     return "Special Handling";
    if (s == RETURN_TO_SENDER)     return "Return to Sender";
    if (s == RE_DELIVERY)          return "Re-delivery";
    return "Unknown";
}
