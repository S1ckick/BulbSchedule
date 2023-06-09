#include "validation.h"

extern char * stn_to_hex[];
extern std::unordered_map<std::string, int> stn_to_int;
extern timepoint START_MODELLING;

inline std::ostream& operator << (std::ostream& o, const Interval& a)
{
    o << a.info.sat_id <<
    " " << (DURATION(START_MODELLING, a.start) * 1000) <<
    " " << (DURATION(START_MODELLING, a.end) * 1000) <<
    // " " << a.capacity_change <<
    " " << stn_to_hex[a.info.station_id] <<
    " " << a.info.station_id;
    return o;
}

int checkBroadcastInRightArea(VecSchedule &schedule_to_check, Satellites &sats, std::string &res) {
    for(auto &interval : schedule_to_check) {
        if(interval.info.state != State::TRANSMISSION)
            continue;
        auto &sat = sats.at(interval.info.sat_id);
        int found = 0;
        for(auto &area : sat.ints_stations) {
            if(interval.start >= area.start && interval.end <= area.end) {
                found = 1;
                break;
            }
        }
    
        if(found == 0) {
            std::stringstream res_ss;
            res_ss << std::fixed << "Error: \n" << interval << std::endl;
            res = res_ss.str();
            return -1;
        }
    }
    return 0;
}

int checkRecordingInRightArea(VecSchedule &schedule_to_check, Satellites &sats, std::string &res)
{
    auto one_ms = std::chrono::milliseconds(1);
    for (auto &interval : schedule_to_check)
    {
        if (interval.info.state == State::RECORDING) {
            auto &sat = sats.at(interval.info.sat_id);
            bool found = false;
            for (auto &area : sat.ints_in_area)
            {
                if (interval.start >= area.start - one_ms && interval.end <= area.end + one_ms)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::stringstream res_ss;
                res_ss << std::fixed
                    << "Error: \n"
                    << interval << std::endl;
                res = res_ss.str();
                return -1;
            }
        }
    }
    return 0;
}

int checkForIntervalsIntersection(VecSchedule &schedule_to_check, std::string &res)
{
    for (int i = 0; i < schedule_to_check.size() - 1; i++)
    {
        if (schedule_to_check[i].start == schedule_to_check[i + 1].start &&
            schedule_to_check[i].end == schedule_to_check[i + 1].end)
            continue;

        if (schedule_to_check[i].end > schedule_to_check[i + 1].start)
        {
            std::stringstream res_ss;
            res_ss << std::fixed << "Error: \n"
                   << schedule_to_check[i] << std::endl
                   << schedule_to_check[i + 1] << std::endl;
            res = res_ss.str();
            return -1;
        }
    }
    return 0;
}

int checkZeroIntervals(VecSchedule &schedule_to_check, std::string &res)
{
    for (auto &interval : schedule_to_check)
    {
        if (interval.start == interval.end)
        {
            std::stringstream res_ss;
            res_ss << std::fixed << "Error: \n"
                   << interval << std::endl;
            res = res_ss.str();
            return -1;
        }
    }
    return 0;
}

int checkTransmissionTillTheEndOfSession(VecSchedule &schedule_to_check, Satellites &sats, std::string &res)
{
    auto one_ms = std::chrono::milliseconds(1);
    for (auto &interval : schedule_to_check)
    {
        if (interval.info.state == State::TRANSMISSION) {
            auto &sat = sats.at(interval.info.sat_id);
            bool found = false;
            for (auto &area : sat.ints_stations)
            {
                if (interval.info.station_id == area.info.station_id && interval.start >= area.start - one_ms && interval.end <= area.end + one_ms)
                {
                    if(interval.end != area.end) {
                        std::stringstream res_ss;
                        res_ss << std::fixed
                            << "Error: \n"
                            << interval << std::endl
                            << area << std::endl;
                        res = res_ss.str();
                    return -1;
                    }
                }
            }
        }
    }
    return 0;
}

int checkValidity(VecSchedule &schedule_to_check, std::string &res)
{
    std::unordered_map<StationID, std::vector<Interval>> stn_to_check;

    for (auto &item : schedule_to_check)
    {
        if (item.info == 0)
            stn_to_check[item.info.station_id].push_back(item);
    }

    for (auto &item : stn_to_check)
    {
        //std::sort(item.second.begin(), item.second.end(), sort_stn);
        for (int i = 0; i < item.second.size() - 1; i++)
        {
            if (!(item.second[i].start < item.second[i + 1].start
                  && item.second[i].start < item.second[i + 1].end
                  && item.second[i].end <= item.second[i + 1].start
                  && item.second[i].end < item.second[i + 1].end))
            {
                // not all is fine
                std::stringstream res_ss;
                res_ss << std::fixed << "Error: \n"
                       << item.second[i] << std::endl
                       << item.second[i + 1] << std::endl;
                res = res_ss.str();
                return -1;
            }
        }
    }
    return 0;
}
