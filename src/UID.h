#pragma once

#include <string>

class UID
{
public:
    UID();
    UID(std::string uid);

    std::string ToString() const;

private:
    std::string mID;
};
