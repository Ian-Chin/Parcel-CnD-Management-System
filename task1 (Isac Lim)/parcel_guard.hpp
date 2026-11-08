#pragma once
#include <string>

class ParcelGuard {
public:
    virtual ~ParcelGuard() {}
    // Returns true if a request or resource still uses this parcel.
    // 'reason' is shown to the user.
    virtual bool isBusy(const std::string& parcelId, std::string& reason) const = 0;
};