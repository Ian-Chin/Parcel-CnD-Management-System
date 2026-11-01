#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "parcel.hpp"
#include "ui.hpp"
#include "parcel_registry.hpp"
#include "parcel_queue.hpp"
#include "parcel_priority_queue.hpp"

class ParcelModule {
private:
    ParcelRegistry      registry;
    ParcelQueue         intakeQueue;
    ParcelPriorityQueue processQueue;
    long                seqCounter;

    static bool isValidId(const std::string& s) {
        if (s.size() < 3 || s.size() > 12) return false;
        for (size_t i = 0; i < s.size(); ++i)
            if (!isalnum(static_cast<unsigned char>(s[i]))) return false;
        return true;
    }
    static bool isValidText(const std::string& s) {
        if (s.empty() || s.size() > 40) return false;
        bool hasChar = false;
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == ',') return false;
            if (!isspace(static_cast<unsigned char>(s[i]))) hasChar = true;
        }
        return hasChar;
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
        if (s.empty() || s.size() > 9) return false;
        for (size_t i = 0; i < s.size(); ++i)
            if (!isdigit(static_cast<unsigned char>(s[i]))) return false;
        out = std::stoi(s);
        return true;
    }

public:
    ParcelModule() : seqCounter(0) {}

    bool registerParcel(const std::string& parcelId, const std::string& recipientId,
                        const std::string& recipientName, const std::string& company,
                        ParcelType type) {
        if (!isValidId(parcelId))        { ui::error("Invalid Parcel ID (3-12 letters/digits, no spaces)."); return false; }
        if (!isValidId(recipientId))     { ui::error("Invalid Recipient ID (3-12 letters/digits, no spaces)."); return false; }
        if (!isValidText(recipientName)) { ui::error("Recipient name cannot be empty, too long or contain commas."); return false; }
        if (!isValidText(company))       { ui::error("Delivery company cannot be empty, too long or contain commas."); return false; }
        if (registry.find(parcelId))     { ui::error("Duplicate Parcel ID: " + parcelId + " already exists."); return false; }

        Parcel p;
        p.parcelId = parcelId;
        p.recipientId = recipientId;
        p.recipientName = recipientName;
        p.deliveryCompany = company;
        p.type = type;
        p.arrivalDateTime = currentDateTime();
        p.status = ParcelStatus::Received;
        p.arrivalSeq = ++seqCounter;

        registry.insert(p);
        intakeQueue.enqueue(parcelId);
        ui::success("Parcel " + parcelId + " registered and added to the intake queue.");
        ui::parcelCard(p, "NEW PARCEL");
        return true;
    }

    void displayAll() const { registry.displayAll(); }

    void searchById(const std::string& id) {
        Parcel* p = registry.find(id);
        if (!p) { ui::error("Parcel " + id + " not found."); return; }
        ui::parcelCard(*p, "PARCEL DETAILS");
    }

    void searchByRecipient(const std::string& rid) const {
        if (registry.displayByRecipient(rid) == 0)
            ui::error("No parcels found for recipient " + rid + ".");
    }

    void displayIntakeQueue() {
        int n = 0;
        std::string* ids = intakeQueue.snapshot(n);
        Parcel** valid = new Parcel*[n > 0 ? n : 1];
        int k = 0;
        for (int i = 0; i < n; ++i) {
            Parcel* p = registry.find(ids[i]);
            if (p && p->status == ParcelStatus::Received) valid[k++] = p;
        }
        if (k == 0) {
            ui::info("Intake queue is empty - no parcels waiting to be sorted.");
        } else {
            const int w[5] = { 10, 9, 16, 16, 19 };
            ui::tableLine(w, 5);
            ui::rowOpen();
            ui::cell("Position", w[0]); ui::cell("Parcel ID", w[1]);
            ui::cell("Type", w[2]);     ui::cell("Recipient", w[3]);
            ui::cell("Arrived", w[4]);  ui::rowClose();
            ui::tableLine(w, 5);
            for (int i = 0; i < k; ++i) {
                std::string pos = std::to_string(i + 1);
                if (i == 0) pos += " FRONT"; else if (i == k - 1) pos += " REAR";
                ui::rowOpen();
                ui::cell(pos, w[0]);
                ui::cell(valid[i]->parcelId, w[1]);
                ui::cell(typeToString(valid[i]->type), w[2]);
                ui::cell(valid[i]->recipientName, w[3]);
                ui::cell(valid[i]->arrivalDateTime, w[4]);
                ui::rowClose();
            }
            ui::tableLine(w, 5);
            std::cout << "  FIFO: the FRONT parcel is sorted first.\n";
        }
        delete[] valid;
        delete[] ids;
    }

    void displayProcessQueue() {
        int n = 0;
        PQEntry* entries = processQueue.sortedSnapshot(n);
        Parcel** valid = new Parcel*[n > 0 ? n : 1];
        int k = 0;
        for (int i = 0; i < n; ++i) {
            Parcel* p = registry.find(entries[i].parcelId);
            if (p && p->status == ParcelStatus::Pending) valid[k++] = p;
        }
        if (k == 0) {
            ui::info("Processing queue is empty - nothing is waiting to be processed.");
        } else {
            const int w[6] = { 6, 9, 16, 8, 16, 19 };
            ui::tableLine(w, 6);
            ui::rowOpen();
            ui::cell("Order", w[0]);     ui::cell("Parcel ID", w[1]);
            ui::cell("Type", w[2]);      ui::cell("Priority", w[3]);
            ui::cell("Recipient", w[4]); ui::cell("Arrived", w[5]);
            ui::rowClose();
            ui::tableLine(w, 6);
            for (int i = 0; i < k; ++i) {
                ui::rowOpen();
                ui::cell(std::to_string(i + 1), w[0]);
                ui::cell(valid[i]->parcelId, w[1]);
                ui::cell(typeToString(valid[i]->type), w[2]);
                ui::cell("Level " + std::to_string(typePriority(valid[i]->type)), w[3]);
                ui::cell(valid[i]->recipientName, w[4]);
                ui::cell(valid[i]->arrivalDateTime, w[5]);
                ui::rowClose();
            }
            ui::tableLine(w, 6);
            std::cout << "  Level 1 = highest priority. Same level: earlier arrival first.\n";
        }
        delete[] valid;
        delete[] entries;
    }

    int sortIncoming() {
        if (intakeQueue.isEmpty()) { ui::info("No incoming parcels to sort (intake queue is empty)."); return 0; }
        int moved = 0;
        std::string id;
        while (intakeQueue.dequeue(id)) {
            Parcel* p = registry.find(id);
            if (!p || p->status != ParcelStatus::Received) continue;
            p->status = ParcelStatus::Pending;
            processQueue.push(typePriority(p->type), p->arrivalSeq, id);
            std::cout << "  - " << ui::fit(id, 9) << ui::fit(typeToString(p->type), 18)
                      << "-> priority level " << typePriority(p->type) << "\n";
            ++moved;
        }
        if (moved == 0) ui::info("No valid parcels were waiting in the intake queue.");
        else ui::success(std::to_string(moved) + " parcel(s) sorted into the processing queue.");
        return moved;
    }

    Parcel* processNext(std::string& outRoute) {
        std::string id;
        while (processQueue.pop(id)) {
            Parcel* p = registry.find(id);
            if (!p || p->status != ParcelStatus::Pending) continue;
            p->status = ParcelStatus::Processing;
            switch (p->type) {
                case ParcelType::SpecialHandling:
                    outRoute = "Special-Handling Counter (needs a Special Handling service request)";
                    break;
                case ParcelType::Fragile:
                    outRoute = "Collection Counter (handle with care)";
                    p->status = ParcelStatus::ReadyForCollection;
                    break;
                case ParcelType::Large:
                    outRoute = "Delivery Desk (campus delivery)";
                    p->status = ParcelStatus::OutForDelivery;
                    break;
                case ParcelType::Document:
                case ParcelType::Standard:
                    outRoute = "Collection Counter";
                    p->status = ParcelStatus::ReadyForCollection;
                    break;
            }
            return p;
        }
        ui::info("No pending parcels to process (processing queue is empty). Sort incoming parcels first.");
        return nullptr;
    }

    bool updateRecipient(const std::string& id, const std::string& newName) {
        Parcel* p = registry.find(id);
        if (!p) { ui::error("Parcel " + id + " not found."); return false; }
        if (!isValidText(newName)) { ui::error("Invalid name (empty, too long or contains commas)."); return false; }
        p->recipientName = newName;
        ui::success("Recipient name updated.");
        return true;
    }

    bool updateType(const std::string& id, ParcelType newType) {
        Parcel* p = registry.find(id);
        if (!p) { ui::error("Parcel " + id + " not found."); return false; }
        if (p->status != ParcelStatus::Received) {
            ui::error("Type can only be changed while the status is 'Received'."); return false;
        }
        p->type = newType;
        ui::success("Parcel type updated to " + typeToString(newType) + ".");
        return true;
    }

    bool updateStatus(const std::string& id, ParcelStatus newStatus) {
        Parcel* p = registry.find(id);
        if (!p) { ui::error("Parcel " + id + " not found."); return false; }
        if (isTerminalStatus(p->status)) {
            ui::error("Parcel is already '" + statusToString(p->status) + "' - it cannot be changed."); return false;
        }
        if (p->status == newStatus) { ui::error("Parcel already has that status."); return false; }
        ParcelStatus old = p->status;
        p->status = newStatus;
        ui::success("Status changed: " + statusToString(old) + " -> " + statusToString(newStatus));
        return true;
    }

    bool removeParcel(const std::string& id) {
        if (registry.isEmpty()) { ui::error("There are no records to remove."); return false; }
        if (!registry.remove(id)) { ui::error("Parcel " + id + " not found."); return false; }
        ui::success("Parcel " + id + " removed from the records.");
        return true;
    }

    bool saveToFile(const std::string& path) const {
        std::ofstream out(path.c_str());
        if (!out) { ui::error("Cannot write data file: " + path); return false; }
        out << "# ParcelID,RecipientID,RecipientName,Company,Type(1-5),Arrival,Status(1-9),Seq\n";
        out << "# Type: 1=Standard 2=Large 3=Fragile 4=Document 5=Special-Handling\n";
        out << "# Status: 1=Received 2=Pending 3=Processing 4=Ready 5=OutForDelivery 6=Collected 7=Returned 8=Missing 9=Cancelled\n";
        for (ParcelNode* c = registry.first(); c; c = c->next) {
            const Parcel& p = c->data;
            out << p.parcelId << ',' << p.recipientId << ',' << p.recipientName << ','
                << p.deliveryCompany << ',' << static_cast<int>(p.type) << ','
                << p.arrivalDateTime << ',' << static_cast<int>(p.status) << ','
                << p.arrivalSeq << '\n';
        }
        return true;
    }

    int loadFromFile(const std::string& path) {
        std::ifstream in(path.c_str());
        if (!in) { ui::warn("Data file '" + path + "' not found - starting with empty records."); return 0; }
        std::string line;
        int lineNo = 0, loaded = 0;
        while (std::getline(in, line)) {
            ++lineNo;
            if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
            if (line.empty() || line[0] == '#') continue;

            std::string f[9];
            int n = splitFields(line, f, 9);
            int type = 0, status = 0, seq = 0;
            if (n != 8 || !toInt(f[4], type) || !toInt(f[6], status) || !toInt(f[7], seq) ||
                type < 1 || type > 5 || status < 1 || status > 9 || seq < 1 ||
                !isValidId(f[0]) || !isValidId(f[1]) || !isValidText(f[2]) || !isValidText(f[3]) || f[5].empty()) {
                ui::warn("Data file line " + std::to_string(lineNo) + " is invalid - skipped.");
                continue;
            }
            if (registry.find(f[0])) {
                ui::warn("Data file line " + std::to_string(lineNo) + ": duplicate ID " + f[0] + " - skipped.");
                continue;
            }
            Parcel p;
            p.parcelId = f[0]; p.recipientId = f[1]; p.recipientName = f[2];
            p.deliveryCompany = f[3]; p.type = static_cast<ParcelType>(type);
            p.arrivalDateTime = f[5]; p.status = static_cast<ParcelStatus>(status);
            p.arrivalSeq = seq;
            registry.insert(p);
            if (seq > seqCounter) seqCounter = seq;

            if (p.status == ParcelStatus::Received)     intakeQueue.enqueue(p.parcelId);
            else if (p.status == ParcelStatus::Pending) processQueue.push(typePriority(p.type), p.arrivalSeq, p.parcelId);
            ++loaded;
        }
        return loaded;
    }

    Parcel* findParcel(const std::string& id) { return registry.find(id); }
    int totalParcels() const { return registry.size(); }
    int countByStatus(ParcelStatus s) const { return registry.countByStatus(s); }
};
