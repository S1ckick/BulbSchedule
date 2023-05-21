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

#define PLANES_NUM 20

#define DURATION(start, end) ((std::chrono::duration<double, std::milli>((end) - (start)) *                     \
                               std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den) \
                                  .count())

using SatName = uint32_t;
using ObsName = std::string;

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

const std::map<SatType, std::string> SatNames = {{SatType::KINOSAT, "KinoSat"}, {SatType::ZORKIY, "Zorkiy"}};
const std::string KinosatName = SatNames.at(SatType::KINOSAT);
const std::string ZorkiyName = SatNames.at(SatType::ZORKIY);

struct Interval
{
    SatName sat_name;
    SatType sat_type;
    State state = State::IDLE;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    double duration;
    double capacity_change = 0; // for satellites

    ObsName obs_name;

    Interval(const Interval &base_interval) = default;
    Interval(Interval &base_interval) = default;

    // Constructor for parser
    Interval(
        const std::chrono::system_clock::time_point &tp_start,
        const std::chrono::system_clock::time_point &tp_end,
        const SatName &sat, const SatType &type,
        const ObsName &obs = {}) : start(tp_start), end(tp_end), sat_name(sat), sat_type(type), obs_name(obs)
    {
        duration = DURATION(start, end);
    }

    // Constructor for scheduling algorithm
    // RECORDING -> only satelitte info
    // BROADCAST -> satelitte and observartory info
    Interval(
        const std::chrono::system_clock::time_point &tp_start,
        const std::chrono::system_clock::time_point &tp_end,
        const SatName &sat, const SatType &type,
        const State &new_state, const double &change, const ObsName &obs = {}) : Interval(tp_start, tp_end, sat, type)
    {
        state = new_state;
        capacity_change = change;

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

struct Satellite
{
    SatName name;
    SatType type;
    Schedule ints_in_area;
    Schedule ints_observatories;
    Schedule full_schedule;

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
        (capacity + recorded) < max_capacity ? capacity += recorded : recorded = max_capacity - capacity, capacity = max_capacity;

        return recorded;
    }

    // change capacity, return transmitted amount
    double broadcast(const double &duration)
    {
        if (capacity == 0)
            return 0;

        double transmitted = duration * broadcasting_speed;
        (capacity - transmitted) > 0 ? capacity -= transmitted : transmitted = capacity, capacity = 0;

        return transmitted;
    }
};

typedef std::unordered_map<SatName, Satellite> Satellites;

struct Observatory
{
    std::string name;
    Schedule ints_satellite;
};

typedef std::unordered_map<ObsName, Observatory> Observatories;

#endif