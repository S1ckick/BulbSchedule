#include "validation.h"

extern std::unordered_map<int, std::string> obs_to_hex;
extern std::unordered_map<std::string, int> obs_to_int;
extern timepoint START_MODELLING;

inline std::ostream& operator << (std::ostream& o, const Interval& a)
{
    o << a.info.sat_name <<
    " " << (DURATION(START_MODELLING, a.start) * 1000) <<
    " " << (DURATION(START_MODELLING, a.end) * 1000) <<
    // " " << a.capacity_change <<
    " " << obs_to_hex[a.info.obs_name] <<
    " " << a.info.obs_name;
    return o;
}

// int checkBroadcastInRightArea(VecSchedule &schedule_to_check, Observatories &obs, std::string &res) {
//     for(auto &interval : schedule_to_check) {
//         if(interval.info.state != State::TRANSMISSION)
//             continue;
//         auto &ob = obs.at(interval.info.obs_name);
//         int found = 0;
//         for(auto &area : ob.ints_satellite) {
//             if(interval.start >= area->start && interval.end <= area->end) {
//                 found = 1;
//                 break;
//             }
//         }
    
//         if(found == 0) {
//             std::stringstream res_ss;
//             res_ss << std::fixed << "Error: \n" << interval << std::endl;
//             res = res_ss.str();
//             return -1;
//         }
//     }
//     return 0;
// }

int checkRecordingInRightArea(VecSchedule &schedule_to_check, Satellites &sats, std::string &res)
{
    auto one_ms = std::chrono::milliseconds(1);
    for (auto &interval : schedule_to_check)
    {
        if (interval.info.state == State::RECORDING) {
            auto &sat = sats.at(interval.info.sat_name);
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

int checkValidity(VecSchedule &schedule_to_check, std::string &res)
{
    std::unordered_map<ObsName, std::vector<Interval>> obs_to_check;

    for (auto &item : schedule_to_check)
    {
        if (item.info == 0)
            obs_to_check[item.info.obs_name].push_back(item);
    }

    for (auto &item : obs_to_check)
    {
        //std::sort(item.second.begin(), item.second.end(), sort_obs);
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

// double countObsTotalLength(Satellites &sats)
// {
//     Observatories init_obs;
//     for (auto &item : sats)
//     {
//         for (auto &interval : item.second.ints_observatories)
//         {
//             if (init_obs.find(interval.info.obs_name) == init_obs.end())
//             {
//                 Observatory ob;
//                 ob.full_schedule.push_back(std::move(std::make_unique<Interval>(interval)));
//                 ob.name = interval.info.obs_name;
//                 init_obs[interval.info.obs_name] = ob;
//             }
//             else
//             {
//                 init_obs[interval.info.obs_name].full_schedule.push_back(std::move(std::make_unique<Interval>(interval)));
//             }
//         }
//     }

//     double obs_total_length = 0.0;
//     for (auto &ob : init_obs)
//     {
//         //std::sort(ob.second.full_schedule.begin(), ob.second.full_schedule.end(), sort_obs);

//         for (int i = 0; i < ob.second.full_schedule.size(); i++)
//         {
//             timepoint start = ob.second.full_schedule[i]->start;
//             timepoint end = ob.second.full_schedule[i]->end;
//             while (i + 1 < ob.second.full_schedule.size() && end >= ob.second.full_schedule[i + 1]->start)
//             {
//                 // (S)--------------E
//                 //    S------E          S--------(E)
//                 //                S--------E            S-----E
//                 if (end < ob.second.full_schedule[i + 1]->end)
//                     end = ob.second.full_schedule[i + 1]->end;
//                 i++;
//             }
//             obs_total_length += DURATION(start, end);
//         }
//     }
//     return obs_total_length;
// }