#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include "../task3 (Ian Chin)/resource_module.hpp"
#include "activity.hpp"
#include "activity_stack.hpp"

// Task 4 - Parcel Activity and Record Management.
//
// Data structures (all Stack, written in activity_stack.hpp):
//   history   - every activity, most recent on top. Owns the Activity objects.
//   undoStack - pointers to activities that may be undone, most recent on top.
//   temp      - local stack used to walk the history. Popping history onto
//               temp reverses the order, which gives oldest-first output.
//
// Task 1, Task 2 and Task 3 report activities through record() (ActivityLog
// connector). Undo reaches the Task 2 requests through RequestLink.
class ActivityModule : public ActivityLog {
private:
    Stack<Activity*> history;
    Stack<Activity*> undoStack;
    ParcelModule&    parcels;
    ResourceModule&  resources;
    RequestLink*     requests;     // Task 2 (nullptr = not connected)
    int              nextNumber;   // number used for the next Activity ID
    std::string      filePath;     // activities.txt
    std::string      staff;        // staff ID entered at startup ("-" = none)

    // ---------- helpers ----------

    std::string makeId() {
        char buf[16];
        snprintf(buf, sizeof(buf), "A%04d", nextNumber++);
        return std::string(buf);
    }

    // Gives StatusChange / JobCompleted a more exact name from the new status.
    static ActivityType refineType(ActivityType t, ParcelStatus newStatus) {
        if (t == ActivityType::StatusChange || t == ActivityType::JobCompleted) {
            if (newStatus == ParcelStatus::Cancelled) return ActivityType::Cancellation;
            if (newStatus == ParcelStatus::Missing)   return ActivityType::ReportedMissing;
            if (newStatus == ParcelStatus::Collected) return ActivityType::CollectionConfirmation;
            if (newStatus == ParcelStatus::Returned)  return ActivityType::ReturnProcessing;
        }
        return t;
    }

    // Shorter status names so the tables fit on screen.
    static std::string shortStatus(int s) {
        if (s == static_cast<int>(ParcelStatus::ReadyForCollection)) return "Ready";
        return statusLabel(s);
    }

    static std::string activityName(const Activity* a) {
        if (a->type == ActivityType::Reversal) return "Undo of " + a->reverses;
        return activityTypeToString(a->type);
    }

    // Puts a new or loaded activity on the history stack, and on the undo
    // stack if it is a type that can be undone and has not been undone yet.
    void add(Activity* a) {
        history.push(a);
        if (isReversibleType(a->type) && !a->reversed) undoStack.push(a);
    }

    // ---------- walking the history with a temp stack ----------
    // history: top = newest. Popping everything onto temp makes temp's top the
    // oldest. Popping temp back onto history restores the original order.

    void moveAllToTemp(Stack<Activity*>& temp) {
        Activity* a;
        while (history.pop(a)) temp.push(a);
    }

    void restoreFromTemp(Stack<Activity*>& temp) {
        Activity* a;
        while (temp.pop(a)) history.push(a);
    }

    // Newest activity for this parcel that still counts for undo.
    Activity* latestFor(const std::string& parcelId) {
        Stack<Activity*> temp;
        Activity* found = nullptr;
        Activity* a;
        while (history.pop(a)) {
            temp.push(a);
            if (a->parcelId == parcelId && !a->reversed &&
                a->type != ActivityType::Reversal && a->type != ActivityType::ParcelUpdate &&
                a->type != ActivityType::ServiceRequestCreation &&
                a->type != ActivityType::RequestCancelled) {
                found = a;
                break;
            }
        }
        restoreFromTemp(temp);
        return found;
    }

    // ---------- file ----------

    static std::string toLine(const Activity& a) {
        return a.activityId + "," + a.parcelId + "," + a.requestId + "," + a.resourceId + "," +
               std::to_string(static_cast<int>(a.type)) + "," + a.dateTime + "," +
               std::to_string(a.prevStatus) + "," + std::to_string(a.newStatus) + "," +
               (a.reversed ? "1" : "0") + "," + a.reverses + "," + a.staffId;
    }

    static void writeHeader(std::ofstream& out) {
        out << "# ActivityID,ParcelID,RequestID,ResourceID,Type(1-18),DateTime,PrevStatus(0-9),NewStatus(1-9),Reversed(0/1),Reverses,StaffID\n";
        out << "# Type: 1=Registration 2=ParcelUpdate 3=Sorting 4=Processing 5=StatusChange 6=Removal 7=Assignment 8=JobCompleted\n";
        out << "#       9=JobCancelled 10=ResourceUnavailable 11=Reassignment 12=Cancellation 13=ReportedMissing\n";
        out << "#       14=CollectionConfirmation 15=ReturnProcessing 16=Reversal 17=ServiceRequestCreation 18=RequestCancelled\n";
        out << "# Status: 0=none 1=Received 2=Pending 3=Processing 4=Ready 5=OutForDelivery 6=Collected 7=Returned 8=Missing 9=Cancelled\n";
    }

    // New activities are added to the end of the file straight away, so the
    // history is never behind parcels.txt / resources.txt (both saved after
    // every change by Task 1 and Task 3).
    void appendToFile(const Activity& a) {
        if (filePath.empty()) return;
        bool exists = false;
        { std::ifstream check(filePath.c_str()); exists = check.good(); }
        std::ofstream out(filePath.c_str(), std::ios::app);
        if (!out) { ui::warn("Cannot write activity file: " + filePath); return; }
        if (!exists) writeHeader(out);
        out << toLine(a) << '\n';
    }

    static int splitFields(const std::string& line, std::string out[], int maxF) {
        int n = 0;
        size_t start = 0;
        while (n < maxF) {
            size_t pos = line.find(',', start);
            if (pos == std::string::npos) { out[n++] = line.substr(start); break; }
            out[n++] = line.substr(start, pos - start);
            start = pos + 1;
        }
        return n;
    }

    static bool toInt(const std::string& s, int& out) {
        if (s.empty() || s.size() > 6) return false;
        for (size_t i = 0; i < s.size(); ++i)
            if (!isdigit(static_cast<unsigned char>(s[i]))) return false;
        out = std::stoi(s);
        return true;
    }

    // "A0012" -> 12, or -1 if the format is wrong.
    static int idNumber(const std::string& id) {
        if (id.size() < 2 || id[0] != 'A') return -1;
        int n = 0;
        if (!toInt(id.substr(1), n) || n < 1) return -1;
        return n;
    }

    // ---------- table output ----------

    static const int COLS = 9;

    static void tableHeader(const int w[], bool trace) {
        ui::tableLine(w, COLS);
        ui::rowOpen();
        if (trace) { ui::cell("Step", w[0]); ui::cell("ID", w[1]); ui::cell("Date/Time", w[2]); }
        else       { ui::cell("ID", w[0]);   ui::cell("Date/Time", w[1]); ui::cell("Parcel", w[2]); }
        ui::cell("Activity", w[3]); ui::cell("From", w[4]); ui::cell("To", w[5]);
        ui::cell("Request", w[6]);  ui::cell("Resource", w[7]); ui::cell("Staff", w[8]);
        ui::rowClose();
        ui::tableLine(w, COLS);
    }

    static void tableRow(const int w[], const Activity* a, int step) {
        std::string id = a->activityId + (a->reversed ? "*" : "");
        ui::rowOpen();
        if (step > 0) { ui::cell(std::to_string(step), w[0]); ui::cell(id, w[1]); ui::cell(a->dateTime, w[2]); }
        else          { ui::cell(id, w[0]); ui::cell(a->dateTime, w[1]); ui::cell(a->parcelId, w[2]); }
        ui::cell(activityName(a), w[3]);
        ui::cell(shortStatus(a->prevStatus), w[4]);
        ui::cell(shortStatus(a->newStatus), w[5]);
        ui::cell(a->requestId, w[6]);
        ui::cell(a->resourceId, w[7]);
        ui::cell(a->staffId, w[8]);
        ui::rowClose();
    }

public:
    ActivityModule(ParcelModule& p, ResourceModule& r)
        : parcels(p), resources(r), requests(nullptr), nextNumber(1), staff("-") {}

    void setRequestLink(RequestLink* r) { requests = r; }
    RequestLink* requestLink() const { return requests; }

    // Staff ID entered at startup. Used for every activity that does not
    // bring its own (Task 1, Task 3 and undo).
    void setStaff(const std::string& id) { staff = id.empty() ? "-" : id; }
    const std::string& currentStaff() const { return staff; }

    ~ActivityModule() {
        Activity* a;
        while (history.pop(a)) delete a;   // history owns every activity
    }
    ActivityModule(const ActivityModule&) = delete;
    ActivityModule& operator=(const ActivityModule&) = delete;

    bool isEmpty() const { return history.isEmpty(); }
    int  total() const { return history.size(); }

    // ---------- 1. Record (called by Task 1 / Task 2 / Task 3) ----------

    void record(ActivityType type, const std::string& parcelId,
                const std::string& requestId, const std::string& resourceId,
                ParcelStatus oldStatus, ParcelStatus newStatus,
                const std::string& staffId = "-") override {
        Activity* a = new Activity;
        a->activityId = makeId();
        a->parcelId   = parcelId;
        a->requestId  = requestId.empty() ? "-" : requestId;
        a->resourceId = resourceId.empty() ? "-" : resourceId;
        a->type       = refineType(type, newStatus);
        a->dateTime   = currentDateTime();
        a->prevStatus = (type == ActivityType::Registration) ? 0 : static_cast<int>(oldStatus);
        a->newStatus  = static_cast<int>(newStatus);
        a->reversed   = false;
        a->reverses   = "-";
        a->staffId    = (staffId.empty() || staffId == "-") ? staff : staffId;
        add(a);
        appendToFile(*a);
    }

    // ---------- 2. Display history ----------

    // Most recent first: pop from history until k activities are shown.
    void displayRecent(int k) {
        if (isEmpty()) { ui::info("No activity history yet."); return; }
        const int w[COLS] = { 6, 19, 9, 23, 16, 16, 7, 8, 5 };
        tableHeader(w, false);
        Stack<Activity*> temp;
        Activity* a;
        int shown = 0;
        while (shown < k && history.pop(a)) {
            temp.push(a);
            tableRow(w, a, 0);
            ++shown;
        }
        restoreFromTemp(temp);
        ui::tableLine(w, COLS);
        std::cout << "  Showing the " << shown << " most recent of " << total()
                  << " activities (newest first).  * = undone\n";
    }

    // Oldest first, so the table shows the order in which things happened.
    void displayAll() {
        if (isEmpty()) { ui::info("No activity history yet."); return; }
        const int w[COLS] = { 6, 19, 9, 23, 16, 16, 7, 8, 5 };
        tableHeader(w, false);
        Stack<Activity*> temp;
        moveAllToTemp(temp);                 // temp top = oldest
        Activity* a;
        while (temp.pop(a)) {
            tableRow(w, a, 0);
            history.push(a);                 // rebuilds history, newest on top again
        }
        ui::tableLine(w, COLS);
        std::cout << "  Total: " << total() << " activities (oldest first).  * = undone\n";
    }

    // ---------- 3. Review a parcel ----------

    void traceParcel(const std::string& parcelId) {
        if (isEmpty()) { ui::info("No activity history yet."); return; }
        Parcel* p = parcels.findParcel(parcelId);

        const int w[COLS] = { 4, 6, 19, 23, 16, 16, 7, 8, 5 };
        Stack<Activity*> temp;
        moveAllToTemp(temp);                 // temp top = oldest
        Activity* a;
        Activity* last = nullptr;
        int steps = 0, undone = 0;
        while (temp.pop(a)) {
            if (a->parcelId == parcelId) {
                if (steps == 0) {
                    std::cout << "\n  Parcel ID      : " << parcelId << "\n"
                              << "  Current status : " << (p ? statusToString(p->status) : "Record removed") << "\n\n";
                    tableHeader(w, true);
                }
                ++steps;
                if (a->reversed) ++undone;
                tableRow(w, a, steps);
                last = a;
            }
            history.push(a);
        }
        if (steps == 0) {
            if (!p) ui::error("Parcel " + parcelId + " not found and has no activity history.");
            else    ui::info("No activities recorded for parcel " + parcelId + " yet (status: " + statusToString(p->status) + ").");
            return;
        }
        ui::tableLine(w, COLS);
        std::cout << "  Steps          : " << steps << " (" << undone << " undone)  * = undone\n"
                  << "  Most recent    : " << last->activityId << " " << activityName(last)
                  << " at " << last->dateTime << "\n"
                  << "  Previous status: " << statusLabel(last->prevStatus)
                  << "  ->  " << statusLabel(last->newStatus) << "\n";

        Activity* latest = latestFor(parcelId);
        if (latest && checkUndo(latest).empty())
            std::cout << "  Can undo now   : " << latest->activityId << " " << activityName(latest) << "\n";
        else
            std::cout << "  Can undo now   : nothing\n";
    }

    // ---------- 4. Undo ----------

    // Full details of one activity (shown before undo).
    void showActivity(const Activity* a) const {
        std::cout << "\n  ACTIVITY " << a->activityId << "\n  " << ui::repeat("-", ui::WIDTH) << "\n"
                  << "  Parcel ID      : " << a->parcelId << "\n"
                  << "  Activity       : " << activityName(a) << "\n"
                  << "  Date/Time      : " << a->dateTime << "\n"
                  << "  Status         : " << statusLabel(a->prevStatus) << "  ->  " << statusLabel(a->newStatus) << "\n"
                  << "  Request ID     : " << a->requestId << "\n"
                  << "  Resource ID    : " << a->resourceId << "\n"
                  << "  Staff ID       : " << a->staffId << "\n"
                  << "  Undone         : " << (a->reversed ? "Yes" : "No") << "\n"
                  << "  " << ui::repeat("-", ui::WIDTH) << "\n";
    }

    // Activity with this ID, or nullptr. O(n).
    Activity* findById(const std::string& id) {
        Stack<Activity*> temp;
        Activity* found = nullptr;
        Activity* a;
        while (history.pop(a)) {
            temp.push(a);
            if (a->activityId == id) { found = a; break; }
        }
        restoreFromTemp(temp);
        return found;
    }

    static bool isValidActivityId(const std::string& id) { return idNumber(id) != -1; }

    // Most recent activity that may still be undone. Activities already undone
    // (e.g. through "undo by ID") are removed from the top first.
    Activity* undoCandidate() {
        Activity* a;
        while (undoStack.peek(a) && a->reversed) undoStack.pop(a);
        return undoStack.peek(a) ? a : nullptr;
    }

    // Removes the top of the undo stack (used when it can never be undone).
    void dropUndoCandidate() {
        Activity* a;
        undoStack.pop(a);
    }

    // Returns "" if the activity can be undone right now, otherwise the reason.
    std::string checkUndo(Activity* a) {
        if (!isReversibleType(a->type)) return whyNotReversible(a->type);
        if (a->reversed) return "it has already been undone.";

        Parcel* p = parcels.findParcel(a->parcelId);
        if (!p) return "parcel " + a->parcelId + " no longer exists.";

        Activity* latest = latestFor(a->parcelId);
        if (latest != a)
            return "it is not the latest activity for parcel " + a->parcelId + " (latest is " +
                   latest->activityId + " " + activityName(latest) + ").";

        if (static_cast<int>(p->status) != a->newStatus)
            return "parcel " + a->parcelId + " is now '" + statusToString(p->status) +
                   "', not '" + statusLabel(a->newStatus) + "' (it was changed outside the history).";

        std::string holder = resources.resourceHolding(a->parcelId);
        if (a->type == ActivityType::Assignment) {
            if (holder != a->resourceId)
                return "resource " + a->resourceId + " is no longer handling parcel " + a->parcelId + ".";
            // Assignment made for a Task 2 request: the request must go back too.
            if (a->requestId != "-") {
                if (!requests)
                    return "request " + a->requestId + " cannot be updated because Task 2 is not connected.";
                if (!requests->isProcessing(a->requestId))
                    return "request " + a->requestId + " is no longer Processing.";
            }
        } else if (holder != "-" && (a->prevStatus == static_cast<int>(ParcelStatus::Received) ||
                                     a->prevStatus == static_cast<int>(ParcelStatus::Pending))) {
            return "parcel " + a->parcelId + " is still held by resource " + holder + ", so it cannot go back to '" +
                   statusLabel(a->prevStatus) + "'.";
        }
        return "";
    }

    // Reverses the activity: frees the resource for an assignment (and puts its
    // Task 2 request back to Pending), restores the previous parcel status, puts
    // the parcel back in its queue if needed and records the undo. Eligibility
    // is checked again here before anything changes.
    bool undo(Activity* a) {
        std::string reason = checkUndo(a);
        if (!reason.empty()) { ui::error(a->activityId + " cannot be undone: " + reason); return false; }

        Parcel* p = parcels.findParcel(a->parcelId);
        ParcelStatus before  = p->status;
        ParcelStatus restore = static_cast<ParcelStatus>(a->prevStatus);

        if (a->type == ActivityType::Assignment) {
            if (!resources.releaseResource(a->resourceId)) {
                ui::error("Resource " + a->resourceId + " could not be released. Nothing was changed.");
                return false;
            }
            ui::success("Resource " + a->resourceId + " released and Available again.");
            // checkUndo() confirmed the request is Processing, so this succeeds.
            if (a->requestId != "-" && requests->backToPending(a->requestId))
                ui::success("Request " + a->requestId + " is back to Pending.");
        }

        p->status = restore;
        parcels.requeue(a->parcelId);   // only acts if the restored status is Received / Pending
        a->reversed = true;

        Activity* r = new Activity;
        r->activityId = makeId();
        r->parcelId   = a->parcelId;
        r->requestId  = a->requestId;
        r->resourceId = a->resourceId;
        r->type       = ActivityType::Reversal;
        r->dateTime   = currentDateTime();
        r->prevStatus = static_cast<int>(before);
        r->newStatus  = static_cast<int>(restore);
        r->reversed   = false;
        r->reverses   = a->activityId;
        r->staffId    = staff;
        history.push(r);
        saveToFile();                   // rewrite: the undone flag of an old line changed

        ui::success(a->activityId + " " + activityName(a) + " undone (recorded as " + r->activityId + ").");
        ui::success("Parcel " + p->parcelId + " status: " + statusToString(before) + " -> " + statusToString(restore));
        if (restore == ParcelStatus::Pending)  ui::info("Parcel " + p->parcelId + " is back in the processing queue.");
        if (restore == ParcelStatus::Received) ui::info("Parcel " + p->parcelId + " is back in the intake queue.");
        return true;
    }

    // ---------- 5. File ----------

    bool saveToFile() {
        if (filePath.empty()) return false;
        std::ofstream out(filePath.c_str());
        if (!out) { ui::error("Cannot write activity file: " + filePath); return false; }
        writeHeader(out);
        Stack<Activity*> temp;
        moveAllToTemp(temp);                 // oldest first, so the file reads in time order
        Activity* a;
        while (temp.pop(a)) {
            out << toLine(*a) << '\n';
            history.push(a);
        }
        return true;
    }

    // Reads activities oldest first and pushes them, so the newest ends on top.
    // Lines have 11 fields (with Staff ID). Older 10-field lines are still
    // accepted and get Staff ID "-".
    int loadFromFile(const std::string& path) {
        filePath = path;
        std::ifstream in(path.c_str());
        if (!in) { ui::warn("Activity file '" + path + "' not found - starting with an empty history."); return 0; }
        std::string line;
        int lineNo = 0, loaded = 0, lastNumber = 0;
        while (std::getline(in, line)) {
            ++lineNo;
            if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
            if (line.empty() || line[0] == '#') continue;

            std::string f[12];
            int n = splitFields(line, f, 12);   // 12 = room to spot a line with too many fields
            if (n == 10) f[10] = "-";           // old line without Staff ID
            int type = 0, prev = 0, now = 0, rev = 0;
            int number = (n == 10 || n == 11) ? idNumber(f[0]) : -1;
            if (number == -1 || f[1].empty() || f[2].empty() || f[3].empty() || f[5].empty() ||
                !toInt(f[4], type) || !toInt(f[6], prev) || !toInt(f[7], now) || !toInt(f[8], rev) ||
                type < 1 || type > 18 || prev > 9 || now < 1 || now > 9 || rev > 1 || f[9].empty() || f[10].empty()) {
                ui::warn("Activity file line " + std::to_string(lineNo) + " is invalid - skipped.");
                continue;
            }
            if (number <= lastNumber) {       // IDs must keep increasing (oldest first)
                ui::warn("Activity file line " + std::to_string(lineNo) + ": duplicate or out-of-order ID " + f[0] + " - skipped.");
                continue;
            }
            Activity* a = new Activity;
            a->activityId = f[0]; a->parcelId = f[1]; a->requestId = f[2]; a->resourceId = f[3];
            a->type = static_cast<ActivityType>(type); a->dateTime = f[5];
            a->prevStatus = prev; a->newStatus = now; a->reversed = (rev == 1); a->reverses = f[9];
            a->staffId = f[10];
            add(a);
            lastNumber = number;
            ++loaded;
        }
        nextNumber = lastNumber + 1;
        return loaded;
    }
};
