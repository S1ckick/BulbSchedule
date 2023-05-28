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

#include <date.h>

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

inline std::ostream& operator << (std::ostream& os, const SatType& obj)
{

   if(obj == SatType::KINOSAT) {
    os << "KINOSAT";
   }
   if(obj == SatType::ZORKIY){
    os << "ZORKIY";
   }
   return os;
}

enum class State
{
    IDLE,
    TRANSMISSION,
    RECORDING // for satellite
};

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
    //IntervalInfo(IntervalInfo &base_interval) = default;

    bool operator==(const IntervalInfo &r)
    {        
        return sat_name == r.sat_name && state == r.state && obs_name == r.obs_name;
    }

    IntervalInfo(
        const SatName &sat, const SatType &type,
        const ObsName &obs = {}) : sat_name(sat), sat_type(type), obs_name(obs)
    {
        
    }

    // Constructor for scheduling algorithm
    // RECORDING -> only satelitte info
    // TRANSMISSION -> satelitte and observartory info
    IntervalInfo(
        const SatName &sat, const SatType &type,
        const State &new_state, const ObsName &obs = {}) : IntervalInfo(sat, type, obs)
    {
        state = new_state;

        if (new_state == State::TRANSMISSION)
        {
            if (obs.empty()) {
                std::cout << "Transmission interval should contain observatory\n";
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
    //Interval(Interval &base_interval) = default;

    // use only for same interval info and consecutive intervals
    Interval &operator+=(const Interval &r)
    {
        duration += r.duration;
        capacity_change += r.capacity_change;
        end = r.end;
        
        return *this;
    }

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
    // TRANSMISSION -> satelitte and observartory info
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

inline bool sort_obs(std::shared_ptr<Interval> a, std::shared_ptr<Interval> b) {
    if (a->start == b->start)
        return a->duration > b->duration;
    return a->start < b->start;
}

struct sort_schedule
{
    bool operator()(const std::shared_ptr<Interval> &a, const std::shared_ptr<Interval> &b) const
    {
        if (a->start == b->start) {
            if (a->end == b->end)
                return a->info[0]->obs_name < b->info[0]->obs_name;
            return a->duration > b->duration;
        }
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

    double can_record(const double &duration)
    {
        if (capacity == max_capacity)
            return 0;

        double recorded = duration * recording_speed;
        if (capacity + recorded > max_capacity) {
            recorded = max_capacity - capacity;
        }

        return recorded;
    }

    // change capacity, return recorded amount
    double record(const double &duration)
    {
        double recorded = duration * recording_speed;
        (capacity + recorded) < max_capacity ? capacity += recorded : (recorded = max_capacity - capacity, capacity = max_capacity);

        return recorded;
    }

    double can_broadcast(const double &duration)
    {
        if (capacity == 0)
            return 0;

        double transmitted = duration * broadcasting_speed;
        if (capacity - transmitted < 0) {
            transmitted = capacity;
        }

        return transmitted;
    }

    // change capacity, return transmitted amount
    double transmission(const double &duration)
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

#endif
