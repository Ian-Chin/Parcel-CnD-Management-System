#pragma once
#include <string>
#include "../task1 (Isac Lim)/parcel.hpp"

// Kinds of activity that Task 1 and Task 3 report to Task 4.
// Task 4 turns StatusChange / JobCompleted into a more exact name using the
// new status (e.g. StatusChange to Cancelled becomes "Cancellation").
enum class ActivityType {
    Registration = 1,
    ParcelUpdate,
    Sorting,
    Processing,
    StatusChange,
    Removal,
    Assignment,
    JobCompleted,
    JobCancelled,
    ResourceUnavailable,
    Reassignment,
    // Set by Task 4 only
    Cancellation,
    ReportedMissing,
    CollectionConfirmation,
    ReturnProcessing,
    Reversal
};

// Connector between the task modules and the Task 4 history.
// Task 1 and Task 3 keep a pointer to this. When it is nullptr (e.g. in their
// standalone programs) nothing is reported and they behave exactly as before.
class ActivityLog {
public:
    virtual void record(ActivityType type, const std::string& parcelId,
                        const std::string& requestId, const std::string& resourceId,
                        ParcelStatus oldStatus, ParcelStatus newStatus) = 0;
    virtual ~ActivityLog() {}
};
