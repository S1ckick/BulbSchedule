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


#define STN_NUM 14
#define SAT_NUM 200

#define DURATION(start, end) ((std::chrono::duration<double, std::milli>((end) - (start)) *                     \
                               std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den) \
                                  .count())

using SatID = uint32_t;
using StationID = uint32_t;
using timepoint = std::chrono::system_clock::time_point;

enum class State
{
    IDLE,
    TRANSMISSION,
    RECORDING // for satellite
};

struct IntervalInfo {
    SatID sat_id;
    State state = State::IDLE;
    StationID station_id;

    IntervalInfo() = default;
    IntervalInfo(const IntervalInfo &base_interval) = default;

    bool operator==(const IntervalInfo &r)
    {        
        return sat_id == r.sat_id && state == r.state && station_id == r.station_id;
    }

    IntervalInfo(
        const SatID &sat, const StationID &stn = 0) : sat_id(sat), station_id(stn)
    {
        
    }

    // Constructor for scheduling algorithm
    // RECORDING -> only satelitte info
    // TRANSMISSION -> satelitte and station info
    IntervalInfo(
        const SatID &sat,
        const State new_state, const StationID &stn = 0) : IntervalInfo(sat, stn)
    {
        state = new_state;

        if (new_state == State::TRANSMISSION)
        {
            if (stn == 0) {
                std::cout << "Transmission interval should contain station\n";
                throw std::runtime_error("No station info");
            }
            station_id = stn;
        }
    }
};

struct Interval
{
    timepoint start;
    timepoint end;

    IntervalInfo info;

    Interval(const Interval &base_interval) = default;

    // Constructor for parser
    Interval(
        const timepoint &tp_start,
        const timepoint &tp_end,
        const SatID &sat,
        const StationID &stn = 0) : start(tp_start), end(tp_end), info(sat, stn)
    {
    }

    // Constructor for scheduling algorithm
    // RECORDING -> only satelitte info
    // TRANSMISSION -> satelitte and station info
    Interval(
        const timepoint &tp_start,
        const timepoint &tp_end,
        const SatID &sat,
        const State &new_state, const StationID &stn = 0) : start(tp_start), end(tp_end), info(sat, new_state, stn)
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
                return a.info.station_id < b.info.station_id;
            return DURATION(a.start, a.end) > DURATION(b.start, b.end);
        }
        return a.start < b.start;
    }
};

inline bool sort_for_parsed_schedule(const Interval& a, const Interval& b) {
    if (a.start == b.start)
        return DURATION(a.start, a.end) > DURATION(b.start, b.end);
    return a.start < b.start;
}

struct Segment
{
    timepoint start;
    timepoint end;

    std::vector<IntervalInfo> info;
};

// origin
using Schedule = std::set<Interval, sort_schedule>;
// algos
using VecSegment = std::vector<Segment>;
// result
using VecSchedule = std::vector<Interval>;

struct Satellite
{
    Schedule ints_in_area;
    Schedule ints_stations;
    VecSchedule full_schedule;

    // in Gbit
    double volume;
    double capacity;
    double recording_speed;
    double transmission_speed;

    Satellite ()
    {
        volume = 0;
    }

    void init(int idx)
    {
        if (idx <= 50)
        { // KINOSAT
            capacity = 8192;
            transmission_speed = 1;
            recording_speed = 4;
        }
        else
        { // ZORKIY
            capacity = 4096;
            transmission_speed = 0.25;
            recording_speed = 4;
        }
    }
    
    double can_record(const double &duration)
    {
        if (volume == capacity)
            return 0;

        double recorded = duration * recording_speed;
        if (volume + recorded > capacity) {
            recorded = capacity - volume;
        }

        return recorded;
    }

    // change volume, return recorded amount
    double record(const double &duration)
    {
        double recorded = duration * recording_speed;
        (volume + recorded) < capacity ? volume += recorded : (recorded = capacity - volume, volume = capacity);

        return recorded;
    }

    double can_broadcast(const double &duration)
    {
        if (volume == 0)
            return 0;

        double transmitted = duration * transmission_speed;
        if (volume - transmitted < 0) {
            transmitted = volume;
        }

        return transmitted;
    }

    // change volume, return transmitted amount
    double transmission(const double &duration)
    {
        if (volume == 0)
            return 0;

        double transmitted = duration * transmission_speed;
        (volume - transmitted) > 0 ? volume -= transmitted : (transmitted = volume, volume = 0);

        return transmitted;
    }
};

typedef std::vector<Satellite> Satellites;

#endif
