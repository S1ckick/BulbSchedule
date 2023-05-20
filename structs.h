#ifndef SCHEDULE_STRUCTS
#define SCHEDULE_STRUCTS

#include <map>

#include "date.h"

#define PLANES_NUM 20

using SatName = uint32_t;

enum class SatType
{
    KINOSAT,
    ZORKIY
};

const std::map<SatType, std::string> SatNames = {{SatType::KINOSAT, "KinoSat"}, {SatType::ZORKIY, "Zorkiy"}};
const std::string KinosatName = SatNames.at(SatType::KINOSAT);
const std::string ZorkiyName = SatNames.at(SatType::ZORKIY);

struct Interval
{
    SatName sat_name;
    SatType sat_type;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    double duration;
};

struct Satellite
{
    SatName name;
    SatType type;
    std::vector<Interval> ints;
};

struct Observatory
{
    std::string name;
    std::vector<Interval> ints;
};

#endif