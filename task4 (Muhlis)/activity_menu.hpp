#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include "activity_module.hpp"

// Task 4 menu. Own namespace so it can sit next to the Task 1 and Task 3
// menus in the integrated main.cpp without name clashes.
namespace activity_menu {

const int RECENT_COUNT = 10;

static std::string readLine(const std::string& label) {
    std::string s;
    ui::prompt(label);
    if (!std::getline(std::cin, s)) { std::cout << "\n"; std::exit(0); }
    size_t a = s.find_first_not_of(" \t"), b = s.find_last_not_of(" \t");
    return (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
}

static std::string readId(const std::string& label) {
    std::string s = readLine(label);
    for (size_t i = 0; i < s.size(); ++i) s[i] = static_cast<char>(toupper(static_cast<unsigned char>(s[i])));
    return s;
}

static int readInt(const std::string& label, int lo, int hi) {
    while (true) {
        std::string s = readLine(label);
        bool ok = !s.empty() && s.size() < 6;
        for (size_t i = 0; i < s.size() && ok; ++i) ok = isdigit(static_cast<unsigned char>(s[i])) != 0;
        if (ok) { int v = std::stoi(s); if (v >= lo && v <= hi) return v; }
        ui::error("Please enter a number between " + std::to_string(lo) + " and " + std::to_string(hi) + ".");
    }
}

static bool confirm(const std::string& question) {
    while (true) {
        std::string s = readLine(question + " (y/n)");
        if (s == "y" || s == "Y") return true;
        if (s == "n" || s == "N") return false;
        ui::error("Please type y or n.");
    }
}

// Same rule as Task 1 Parcel IDs: 3-12 letters/digits.
static bool isValidParcelId(const std::string& s) {
    if (s.size() < 3 || s.size() > 12) return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (!isalnum(static_cast<unsigned char>(s[i]))) return false;
    return true;
}

// Asked once when the program starts. Every activity recorded in this run is
// stamped with it (Task 2 requests use their own Staff ID instead).
// Same idea as a Task 2 Staff ID: 2-10 letters/digits, e.g. ST01.
inline std::string askStaffId() {
    while (true) {
        std::string s = readId("Staff ID (e.g. ST01)");
        bool ok = s.size() >= 2 && s.size() <= 10;
        for (size_t i = 0; i < s.size() && ok; ++i) ok = isalnum(static_cast<unsigned char>(s[i])) != 0;
        if (ok) return s;
        ui::error("Staff ID must be 2-10 letters/digits, no spaces.");
    }
}

static void showMenu(ActivityModule& m) {
    ui::bigTitle("SMART CAMPUS PARCEL SYSTEM", "Task 4 - Activity & Record History");
    std::cout << "\n  Activities recorded: " << m.total() << "   |   Staff: " << m.currentStaff() << "\n";
    std::cout << "\n  " << ui::repeat("-", ui::WIDTH) << "\n"
              << "   HISTORY\n"
              << "    [1]  Recent activities        (newest first)\n"
              << "    [2]  Full activity history    (oldest first)\n"
              << "    [3]  Trace a parcel\n"
              << "\n   UNDO\n"
              << "    [4]  Undo latest activity     (top of undo stack)\n"
              << "    [5]  Undo by Activity ID\n"
              << "\n    [0]  Back (data is saved automatically)\n"
              << "  " << ui::repeat("-", ui::WIDTH) << "\n\n";
}

static void doTrace(ActivityModule& m) {
    ui::section("TRACE A PARCEL");
    if (m.isEmpty()) { ui::info("No activity history yet."); return; }
    std::string pid = readId("Parcel ID");
    if (!isValidParcelId(pid)) { ui::error("Invalid Parcel ID (3-12 letters/digits, no spaces)."); return; }
    m.traceParcel(pid);
}

// Returns true if something was undone (so the caller saves the data files).
static bool doUndoLatest(ActivityModule& m) {
    ui::section("UNDO LATEST ACTIVITY");
    if (m.isEmpty()) { ui::info("No activity history yet - nothing to undo."); return false; }
    Activity* a = m.undoCandidate();
    if (!a) { ui::info("Nothing to undo - no activity that can be undone is left."); return false; }
    m.showActivity(a);
    std::string reason = m.checkUndo(a);
    if (!reason.empty()) {
        ui::error(a->activityId + " cannot be undone: " + reason);
        m.dropUndoCandidate();
        ui::info(a->activityId + " has been removed from the undo list. Choose [4] again for the next one.");
        return false;
    }
    std::cout << "\n";
    if (!confirm("Undo this activity?")) { ui::info("Undo cancelled. Nothing was changed."); return false; }
    std::cout << "\n";
    return m.undo(a);
}

static bool doUndoById(ActivityModule& m) {
    ui::section("UNDO BY ACTIVITY ID");
    if (m.isEmpty()) { ui::info("No activity history yet - nothing to undo."); return false; }
    m.displayRecent(RECENT_COUNT);
    std::cout << "\n";
    std::string id = readId("Activity ID (e.g. A0005)");
    if (!ActivityModule::isValidActivityId(id)) { ui::error("Invalid Activity ID format (expected A followed by digits, e.g. A0005)."); return false; }
    Activity* a = m.findById(id);
    if (!a) { ui::error("Activity " + id + " not found."); return false; }
    m.showActivity(a);
    std::string reason = m.checkUndo(a);
    if (!reason.empty()) { ui::error(id + " cannot be undone: " + reason); return false; }
    std::cout << "\n";
    if (!confirm("Undo this activity?")) { ui::info("Undo cancelled. Nothing was changed."); return false; }
    std::cout << "\n";
    return m.undo(a);
}

// Runs the Task 4 menu until the user picks [0]. An undo can change parcels,
// resources and requests too, so their files are saved after it.
inline void run(ActivityModule& module, ParcelModule& parcels, ResourceModule& resources,
                const std::string& parcelFile, const std::string& resourceFile,
                const std::string& requestFile) {
    while (true) {
        showMenu(module);
        int c = readInt("Choose an option", 0, 5);
        if (c == 0) break;
        bool changed = false;
        switch (c) {
            case 1: ui::section("RECENT ACTIVITIES"); module.displayRecent(RECENT_COUNT); break;
            case 2: ui::section("FULL ACTIVITY HISTORY"); module.displayAll(); break;
            case 3: doTrace(module); break;
            case 4: changed = doUndoLatest(module); break;
            case 5: changed = doUndoById(module); break;
        }
        if (changed) {
            parcels.saveToFile(parcelFile);
            resources.saveToFile(resourceFile);
            if (module.requestLink()) module.requestLink()->saveToFile(requestFile);
        }
        ui::pause();
        ui::clearScreen();
    }
}

}
