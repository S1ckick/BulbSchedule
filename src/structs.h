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
using ObsName = uint32_t;
using timepoint = std::chrono::system_clock::time_point;

enum class State
{
    IDLE,
    TRANSMISSION,
    RECORDING // for satellite
};

struct IntervalInfo {
    SatName sat_name;
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
        const SatName &sat, const ObsName &obs = 0) : sat_name(sat), obs_name(obs)
    {
        
    }

    // Constructor for scheduling algorithm
    // RECORDING -> only satelitte info
    // TRANSMISSION -> satelitte and observartory info
    IntervalInfo(
        const SatName &sat,
        const State &new_state, const ObsName &obs = 0) : IntervalInfo(sat, obs)
    {
        state = new_state;

        if (new_state == State::TRANSMISSION)
        {
            if (obs == 0) {
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
    // double duration;
    // double capacity_change = 0;

    IntervalInfo info;

    Interval(const Interval &base_interval) = default;
    //Interval(Interval &base_interval) = default;

    // Constructor for parser
    Interval(
        const timepoint &tp_start,
        const timepoint &tp_end,
        const SatName &sat,
        const ObsName &obs = 0) : start(tp_start), end(tp_end), info(sat, obs)
    {
    }

    // Constructor for scheduling algorithm
    // RECORDING -> only satelitte info
    // TRANSMISSION -> satelitte and observartory info
    Interval(
        const timepoint &tp_start,
        const timepoint &tp_end,
        const SatName &sat,
        const State &new_state, const ObsName &obs = 0) : start(tp_start), end(tp_end), info(sat, new_state, obs)
    {
    }

    Interval(
        const timepoint &tp_start,
        const timepoint &tp_end,
        const IntervalInfo &new_info) : start(tp_start), end(tp_end), info(new_info)
    {
    }

    Interval &operator+=(const Interval &r)
    {
        end = r.end;
        
        return *this;
    }
};

struct sort_schedule
{
    bool operator()(const Interval &a, const Interval &b) const
    {
        if (a.start == b.start) {
            if (a.end == b.end)
                return a.info.obs_name < b.info.obs_name;
            return DURATION(a.start, a.end) > DURATION(b.start, b.end);
        }
        return a.start < b.start;
    }
};

struct sort_schedule_link
{
    bool operator()(const std::unique_ptr<Interval> &a, const std::unique_ptr<Interval> &b) const
    {
        if (a->start == b->start) {
            if (a->end == b->end)
                return a->info.obs_name < b->info.obs_name;
            return DURATION(a->start, a->end) > DURATION(b->start, b->end);
        }
        return a->start < b->start;
    }
};

struct Segment
{
    timepoint start;
    timepoint end;

    std::vector<IntervalInfo> info;
};

// origin
using Schedule = std::set<Interval, sort_schedule>;
using LinkSchedule = std::set<std::unique_ptr<Interval>, sort_schedule_link>;
// algos
using VecSegment = std::vector<Segment>;
// result
using VecSchedule = std::vector<Interval>;
using LinkVecSchedule = std::vector<std::unique_ptr<Interval>>;

struct Satellite
{
    SatName name;
    Schedule ints_in_area;
    Schedule ints_observatories;
    VecSchedule full_schedule;

    // in Gbit
    double capacity;
    double max_capacity;
    double recording_speed;
    double broadcasting_speed;

    Satellite(const SatName &sat_name) : name(sat_name)
    {
        capacity = 0;
        if (sat_name < 110600)
        { // KINOSAT
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

// links for intervals grouped by observatories
struct Observatory
{
    ObsName name;
    LinkSchedule ints_satellite;
    LinkVecSchedule full_schedule;
};

typedef std::unordered_map<ObsName, Observatory> Observatories;

#endif
