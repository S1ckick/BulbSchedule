#ifndef SCHEDULE_STRUCTS
#define SCHEDULE_STRUCTS

#include <map>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <string>

#define PLANES_NUM 20

using SatName = uint32_t;
using ObsName = std::string;

enum class SatType
{
    KINOSAT,
    ZORKIY
};

enum class State
{
    FREE,
    BROADCAST,
    RECORDING // for satellite
};

const std::map<SatType, std::string> SatNames = {{SatType::KINOSAT, "KinoSat"}, {SatType::ZORKIY, "Zorkiy"}};
const std::string KinosatName = SatNames.at(SatType::KINOSAT);
const std::string ZorkiyName = SatNames.at(SatType::ZORKIY);

struct Interval
{
    SatName sat_name;
    SatType sat_type;
    ObsName obs_name;
    State state = State::FREE;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    double duration;
};

using Schedule = std::vector<Interval>;

struct Satellite
{
    SatName name;
    SatType type;
    Schedule ints_in_area;
    Schedule ints_observatories;
    Schedule full_schedule;
};

typedef std::unordered_map<SatName, Satellite> Satellites;



struct Observatory
{
    std::string name;
    Schedule ints_satellite;
};

typedef std::unordered_map<ObsName, Observatory> Observatories;

#endif