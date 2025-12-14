#include "UID.h"

#include <random>
#include <sstream>
#include <iomanip>

UID::UID()
{
    static std::random_device rd;
    static std::mt19937_64 eng(rd());
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

    uint32_t data[4] = { dist(eng), dist(eng), dist(eng), dist(eng) };
    data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000;
    data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000;

    std::stringstream ss;
    ss << std::hex << std::nouppercase << std::setfill('0')
       << std::setw(8) << data[0] << "_"
       << std::setw(4) << (data[1] >> 16) << "_"
       << std::setw(4) << (data[1] & 0xFFFF) << "_"
       << std::setw(4) << (data[2] >> 16) << "_"
       << std::setw(4) << (data[2] & 0xFFFF)
       << std::setw(8) << data[3];

    mID = ss.str();
}

UID::UID(std::string uid)
{
    mID = uid;
}

std::string UID::ToString() const
{
    return mID;
}

bool UID::operator==(const UID& other) const
{
    return mID == other.mID;
}
