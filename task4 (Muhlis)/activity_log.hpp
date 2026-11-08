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
    Reversal,
    // Reported by Task 2 (kept last so saved type numbers do not shift)
    ServiceRequestCreation,
    RequestCancelled
};

// Connector between the task modules and the Task 4 history.
// Task 1, Task 2 and Task 3 keep a pointer to this. When it is nullptr (e.g. in
// their standalone programs) nothing is reported and they behave exactly as before.
// staffId: who did it. "-" means "use the staff ID entered at startup".
class ActivityLog {
public:
    virtual void record(ActivityType type, const std::string& parcelId,
                        const std::string& requestId, const std::string& resourceId,
                        ParcelStatus oldStatus, ParcelStatus newStatus,
                        const std::string& staffId = "-") = 0;
    virtual ~ActivityLog() {}
};

// Connector from Task 4 to the Task 2 requests, used when an undo must also
// put a request back. Task 2's RequestModule implements it. When it is nullptr
// (Task 2 not connected) an undo that involves a request is refused.
class RequestLink {
public:
    virtual bool isProcessing(std::string requestId) = 0;    // request is Processing now?
    virtual bool backToPending(std::string requestId) = 0;   // Processing -> Pending
    virtual bool saveToFile(std::string path) = 0;           // writes requests.txt
    virtual ~RequestLink() {}
};
