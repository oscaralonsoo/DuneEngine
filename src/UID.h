#pragma once

#include <string>

class UID
{
public:
    UID();
    UID(std::string uid);

    std::string ToString() const;

    // Comparison operator
    bool operator==(const UID& other) const;

private:
    std::string mID;
};
