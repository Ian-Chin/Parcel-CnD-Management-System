#pragma once
#include <iostream>
#include <string>
#include "parcel.hpp"
#include "ui.hpp"

struct ParcelNode {
    Parcel      data;
    ParcelNode* next;
    explicit ParcelNode(const Parcel& p) : data(p), next(nullptr) {}
};

class ParcelRegistry {
private:
    ParcelNode* head;
    ParcelNode* tail;
    int         count;

    static const int* widths() {
        static const int w[7] = { 9, 12, 16, 12, 16, 19, 20 };
        return w;
    }

public:
    ParcelRegistry() : head(nullptr), tail(nullptr), count(0) {}
    ~ParcelRegistry() {
        while (head) { ParcelNode* t = head; head = head->next; delete t; }
    }
    ParcelRegistry(const ParcelRegistry&) = delete;
    ParcelRegistry& operator=(const ParcelRegistry&) = delete;

    int  size() const { return count; }
    bool isEmpty() const { return count == 0; }
    ParcelNode* first() const { return head; }

    void insert(const Parcel& p) {
        ParcelNode* n = new ParcelNode(p);
        if (!head) head = tail = n;
        else { tail->next = n; tail = n; }
        ++count;
    }

    Parcel* find(const std::string& id) {
        for (ParcelNode* c = head; c; c = c->next)
            if (c->data.parcelId == id) return &c->data;
        return nullptr;
    }

    bool remove(const std::string& id) {
        ParcelNode* prev = nullptr;
        for (ParcelNode* c = head; c; prev = c, c = c->next) {
            if (c->data.parcelId == id) {
                if (prev) prev->next = c->next; else head = c->next;
                if (c == tail) tail = prev;
                delete c;
                --count;
                return true;
            }
        }
        return false;
    }

    int countByStatus(ParcelStatus s) const {
        int n = 0;
        for (ParcelNode* c = head; c; c = c->next) if (c->data.status == s) ++n;
        return n;
    }

    static void printHeader() {
        const int* w = widths();
        ui::tableLine(w, 7);
        ui::rowOpen();
        const char* names[7] = { "Parcel ID", "Recipient ID", "Name", "Company", "Type", "Arrival", "Status" };
        for (int i = 0; i < 7; ++i) ui::cell(names[i], w[i]);
        ui::rowClose();
        ui::tableLine(w, 7);
    }
    static void printRow(const Parcel& p) {
        const int* w = widths();
        ui::rowOpen();
        ui::cell(p.parcelId, w[0]);
        ui::cell(p.recipientId, w[1]);
        ui::cell(p.recipientName, w[2]);
        ui::cell(p.deliveryCompany, w[3]);
        ui::cell(typeToString(p.type), w[4]);
        ui::cell(p.arrivalDateTime, w[5]);
        ui::cell(statusToString(p.status), w[6]);
        ui::rowClose();
    }
    static void printFooter() {
        ui::tableLine(widths(), 7);
    }

    void displayAll() const {
        if (!head) { ui::info("No parcel records."); return; }
        printHeader();
        for (ParcelNode* c = head; c; c = c->next) printRow(c->data);
        printFooter();
        std::cout << "  Total: " << count << " record(s)\n";
    }

    int displayByRecipient(const std::string& rid) const {
        int found = 0;
        for (ParcelNode* c = head; c; c = c->next) {
            if (c->data.recipientId == rid) {
                if (found == 0) printHeader();
                printRow(c->data);
                ++found;
            }
        }
        if (found > 0) {
            printFooter();
            std::cout << "  Total: " << found << " record(s)\n";
        }
        return found;
    }
};
