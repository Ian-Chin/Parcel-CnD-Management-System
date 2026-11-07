#pragma once
#include <string>
#include "activity_log.hpp"

// One entry in the activity history.
struct Activity {
    std::string  activityId;   // A0001, A0002, ...
    std::string  parcelId;
    std::string  requestId;    // "-" when there is no service request
    std::string  resourceId;   // "-" when no resource is involved
    ActivityType type;
    std::string  dateTime;
    int          prevStatus;   // ParcelStatus as a number, 0 = none (registration)
    int          newStatus;    // ParcelStatus as a number
    bool         reversed;     // true once this activity has been undone
    std::string  reverses;     // Reversal records only: ID of the activity undone, else "-"
};

inline std::string activityTypeToString(ActivityType t) {
    switch (t) {
        case ActivityType::Registration:           return "Parcel Registration";
        case ActivityType::ParcelUpdate:           return "Parcel Update";
        case ActivityType::Sorting:                return "Sorting";
        case ActivityType::Processing:             return "Processing";
        case ActivityType::StatusChange:           return "Status Change";
        case ActivityType::Removal:                return "Parcel Removal";
        case ActivityType::Assignment:             return "Resource Assignment";
        case ActivityType::JobCompleted:           return "Job Completed";
        case ActivityType::JobCancelled:           return "Assignment Cancelled";
        case ActivityType::ResourceUnavailable:    return "Resource Unavailable";
        case ActivityType::Reassignment:           return "Resource Re-assignment";
        case ActivityType::Cancellation:           return "Cancellation";
        case ActivityType::ReportedMissing:        return "Reported Missing";
        case ActivityType::CollectionConfirmation: return "Collection Confirmation";
        case ActivityType::ReturnProcessing:       return "Return Processing";
        case ActivityType::Reversal:               return "Reversal (Undo)";
        case ActivityType::ServiceRequestCreation: return "Service Request Creation";
    }
    return "Unknown";
}

// Which activity types may be undone.
inline bool isReversibleType(ActivityType t) {
    return t == ActivityType::Processing   || t == ActivityType::StatusChange ||
           t == ActivityType::Cancellation || t == ActivityType::ReportedMissing ||
           t == ActivityType::Assignment;
}

inline std::string whyNotReversible(ActivityType t) {
    switch (t) {
        case ActivityType::Registration:           return "a registration cannot be undone (it would delete the parcel record).";
        case ActivityType::ParcelUpdate:           return "a parcel detail update does not change the status, so there is nothing to undo.";
        case ActivityType::Sorting:                return "sorting cannot be undone (the parcel is already in the priority queue).";
        case ActivityType::Removal:                return "the parcel record was deleted and cannot be restored.";
        case ActivityType::CollectionConfirmation: return "a confirmed collection cannot be undone.";
        case ActivityType::ReturnProcessing:       return "a processed return cannot be undone.";
        case ActivityType::JobCompleted:           return "a completed job cannot be undone.";
        case ActivityType::JobCancelled:           return "a cancelled assignment cannot be undone - assign the parcel again instead.";
        case ActivityType::ResourceUnavailable:
        case ActivityType::Reassignment:           return "it was done automatically by the system.";
        case ActivityType::Reversal:               return "an undo cannot itself be undone.";
        case ActivityType::ServiceRequestCreation: return "a service request is cancelled in Task 2 (option 8), not undone here.";
        default:                                   return "this activity type cannot be undone.";
    }
}

// Status number -> text. 0 means "no status" (used for registration).
inline std::string statusLabel(int s) {
    if (s < 1 || s > 9) return "-";
    return statusToString(static_cast<ParcelStatus>(s));
}
