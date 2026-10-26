#pragma once
#include <iostream>
#include <string>
#include "parcel.hpp"

namespace ui {

const int WIDTH = 64;

inline std::string repeat(const std::string& s, int n) {
    std::string r;
    for (int i = 0; i < n; ++i) r += s;
    return r;
}

inline std::string fit(const std::string& s, size_t w) {
    if (s.size() > w) return (w > 2) ? s.substr(0, w - 2) + ".." : s.substr(0, w);
    return s + std::string(w - s.size(), ' ');
}

inline std::string center(const std::string& s, size_t w) {
    if (s.size() >= w) return s;
    size_t left = (w - s.size()) / 2;
    return std::string(left, ' ') + s;
}

inline void clearScreen() { std::cout << "\033[2J\033[H"; }

inline void pause() {
    std::cout << "\n  Press Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

inline void success(const std::string& m) { std::cout << "  [OK]    " << m << "\n"; }
inline void error(const std::string& m)   { std::cout << "  [ERROR] " << m << "\n"; }
inline void warn(const std::string& m)    { std::cout << "  [WARN]  " << m << "\n"; }
inline void info(const std::string& m)    { std::cout << "  [INFO]  " << m << "\n"; }

inline void bigTitle(const std::string& line1, const std::string& line2) {
    std::cout << "\n  " << repeat("=", WIDTH) << "\n"
              << "  " << center(line1, WIDTH) << "\n"
              << "  " << center(line2, WIDTH) << "\n"
              << "  " << repeat("=", WIDTH) << "\n";
}

inline void section(const std::string& title) {
    std::cout << "\n  " << repeat("=", WIDTH) << "\n"
              << "  " << title << "\n"
              << "  " << repeat("=", WIDTH) << "\n";
}

inline void prompt(const std::string& label) {
    std::cout << "  " << label << ": ";
}

inline void tableLine(const int w[], int n) {
    std::cout << "  +";
    for (int i = 0; i < n; ++i) std::cout << repeat("-", w[i] + 2) << "+";
    std::cout << "\n";
}
inline void rowOpen() { std::cout << "  |"; }
inline void cell(const std::string& text, int w) {
    std::cout << " " << fit(text, w) << " |";
}
inline void rowClose() { std::cout << "\n"; }

inline void parcelCard(const Parcel& p, const std::string& title) {
    std::cout << "\n  " << title << "\n  " << repeat("-", WIDTH) << "\n"
              << "  Parcel ID    : " << p.parcelId << "\n"
              << "  Recipient ID : " << p.recipientId << "\n"
              << "  Recipient    : " << p.recipientName << "\n"
              << "  Company      : " << p.deliveryCompany << "\n"
              << "  Type         : " << typeToString(p.type) << "\n"
              << "  Arrived      : " << p.arrivalDateTime << "\n"
              << "  Status       : " << statusToString(p.status) << "\n"
              << "  " << repeat("-", WIDTH) << "\n";
}

}
