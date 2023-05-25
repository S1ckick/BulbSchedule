#ifndef SCHEDULE_STRUCTS
#define SCHEDULE_STRUCTS

#include <map>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <algorithm>

#define PLANES_NUM 20

#define DURATION(start, end) ((std::chrono::duration<double, std::milli>((end) - (start)) *                     \
                               std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den) \
                                  .count())

using SatName = uint32_t;
using ObsName = std::string;
using timepoint = std::chrono::system_clock::time_point;

enum class SatType
{
    KINOSAT,
    ZORKIY
};

enum class State
{
    IDLE,
    BROADCAST,
    RECORDING // for satellite
};

std::ostream& operator << (std::ostream& os, const State& obj)
{

   if(obj == State::IDLE) {
    os << "IDLE";
   }
   if(obj == State::BROADCAST){
    os << "BROADCAST";
   }
   if(obj == State::RECORDING) {
    os << "RECORDING";
   }
   return os;
}

const std::map<SatType, std::string> SatNames = {{SatType::KINOSAT, "KinoSat"}, {SatType::ZORKIY, "Zorkiy"}};
const std::string KinosatName = SatNames.at(SatType::KINOSAT);
const std::string ZorkiyName = SatNames.at(SatType::ZORKIY);

struct IntervalInfo {
    SatName sat_name;
    SatType sat_type;
    State state = State::IDLE;
    ObsName obs_name;

    IntervalInfo() = default;
    IntervalInfo(const IntervalInfo &base_interval) = default;
    IntervalInfo(IntervalInfo &base_interval) = default;

    IntervalInfo(
        const SatName &sat, const SatType &type,
        const ObsName &obs = {}) : sat_name(sat), sat_type(type), obs_name(obs)
    {
        
    }

    // Constructor for scheduling algorithm
    // RECORDING -> only satelitte info
    // BROADCAST -> satelitte and observartory info
    IntervalInfo(
        const SatName &sat, const SatType &type,
        const State &new_state, const ObsName &obs = {}) : IntervalInfo(sat, type, obs)
    {
        state = new_state;

        if (new_state == State::BROADCAST)
        {
            if (obs.empty()) {
                std::cout << "Broadcast interval should contain observatory\n";
                throw std::runtime_error("No observatory info");
            }
            obs_name = obs;
        }
    }
};

struct Interval
{
    timepoint start;
    timepoint end;
    double duration;
    double capacity_change = 0;

    std::vector<std::shared_ptr<IntervalInfo>> info;

    Interval(const Interval &base_interval) = default;
    Interval(Interval &base_interval) = default;

    // Constructor for parser
    Interval(
        const timepoint &tp_start,
        const timepoint &tp_end,
        const SatName &sat, const SatType &type,
        const ObsName &obs = {}) : start(tp_start), end(tp_end)
    {
        duration = DURATION(start, end);
        info.push_back(std::make_shared<IntervalInfo>(IntervalInfo(sat, type, obs)));
    }

    // Constructor for scheduling algorithm
    // RECORDING -> only satelitte info
    // BROADCAST -> satelitte and observartory info
    Interval(
        const timepoint &tp_start,
        const timepoint &tp_end,
        const SatName &sat, const SatType &type,
        const State &new_state, const ObsName &obs = {}) : start(tp_start), end(tp_end)
    {
        duration = DURATION(start, end);
        info.push_back(std::make_shared<IntervalInfo>(IntervalInfo(sat, type, new_state, obs)));
    }

    void add_info(const std::vector<std::shared_ptr<IntervalInfo>> &new_info)
    {
        auto start_size = info.size();

        info.resize(start_size + new_info.size());
        for (int i = 0; i < new_info.size(); ++i)
            info[start_size + i] = new_info[i];
    }

    // Constructor for plan
    Interval(
        const timepoint &tp_start,
        const timepoint &tp_end,
        const std::vector<std::shared_ptr<IntervalInfo>> &new_info) : start(tp_start), end(tp_end)
    {
        duration = DURATION(start, end);
        info.resize(new_info.size());
        std::copy(new_info.begin(), new_info.end(), info.begin());
    }
};

struct sort_schedule
{
    bool operator()(const std::shared_ptr<Interval> &a, const std::shared_ptr<Interval> &b) const
    {
        if (a->start == b->start)
            return a->duration > b->duration;
        return a->start < b->start;
    }
};


using Schedule = std::set<std::shared_ptr<Interval>, sort_schedule>;
using VecSchedule = std::vector<std::shared_ptr<Interval>>;

struct Satellite
{
    SatName name;
    SatType type;
    Schedule ints_in_area;
    Schedule ints_observatories;
    VecSchedule full_schedule;

    // in Gbit
    double capacity;
    double max_capacity;
    double recording_speed;
    double broadcasting_speed;

    Satellite(const SatName &sat_name, const SatType &sat_type) : name(sat_name), type(sat_type)
    {
        capacity = 0;
        if (sat_type == SatType::KINOSAT)
        {
            max_capacity = 8192;
            broadcasting_speed = 1;
            recording_speed = 4;
        }
        else
        { // ZORKIY
            max_capacity = 4096;
            broadcasting_speed = 0.25;
            recording_speed = 4;
        }
    }

    // change capacity, return recorded amount
    double record(const double &duration)
    {
        double recorded = duration * recording_speed;
        (capacity + recorded) < max_capacity ? capacity += recorded : (recorded = max_capacity - capacity, capacity = max_capacity);

        return recorded;
    }

    // change capacity, return transmitted amount
    double broadcast(const double &duration)
    {
        if (capacity == 0)
            return 0;

        double transmitted = duration * broadcasting_speed;
        (capacity - transmitted) > 0 ? capacity -= transmitted : (transmitted = capacity, capacity = 0);

        return transmitted;
    }
};

typedef std::unordered_map<SatName, Satellite> Satellites;

struct Observatory
{
    std::string name;
    Schedule ints_satellite;
    VecSchedule full_schedule;
};

typedef std::unordered_map<ObsName, Observatory> Observatories;

std::unordered_map<std::string, int> obs_to_int = {
    {"Anadyr1",1},
    {"Anadyr2", 2},
    {"CapeTown", 3},
    {"Delhi", 4},
    {"Irkutsk", 5},
    {"Magadan1", 6},
    {"Magadan2", 7},
    {"Moscow", 8},
    {"Murmansk1", 9},
    {"Murmansk2", 10},
    {"Norilsk", 11},
    {"Novosib", 12},
    {"RioGallegos", 13},
    {"Sumatra", 14}
};

#endif
