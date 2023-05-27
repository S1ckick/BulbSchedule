#include "validation.h"

std::ostream& operator<<(std::ostream& os, const State& obj)
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

int checkRecordingInRightArea(VecSchedule &schedule_to_check, Satellites &sats, std::string &res)
{
    auto one_ms = std::chrono::milliseconds(1);
    for (auto &interval : schedule_to_check)
    {
        if (interval->info[0]->state == State::RECORDING) {
            auto &sat = sats.at(interval->info[0]->sat_name);
            bool found = false;
            for (auto &area : sat.ints_in_area)
            {
                if (interval->start >= area->start - one_ms && interval->end <= area->end + one_ms)
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
                    << (interval->info[0]->sat_name) << " " << (interval->capacity_change) << std::endl;
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
        if (schedule_to_check[i]->start == schedule_to_check[i + 1]->start &&
            schedule_to_check[i]->end == schedule_to_check[i + 1]->end)
            continue;

        if (schedule_to_check[i]->end > schedule_to_check[i + 1]->start)
        {
            std::stringstream res_ss;
            res_ss << std::fixed << "Error: \n"
                   << schedule_to_check[i]->info[0]->sat_name << schedule_to_check[i]->info[0]->state << std::endl
                   << schedule_to_check[i + 1]->info[0]->sat_name << schedule_to_check[i + 1]->info[0]->state << std::endl;
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
        if (interval->start == interval->end)
        {
            std::stringstream res_ss;
            res_ss << std::fixed << "Error: \n"
                   << interval.get() << std::endl;
            res = res_ss.str();
            return -1;
        }
    }
    return 0;
}

int checkValidity(VecSchedule &schedule_to_check, std::string &res)
{
    std::unordered_map<std::string, std::vector<std::shared_ptr<Interval>>> obs_to_check;

    for (auto &item : schedule_to_check)
    {
        if (item->info[0]->obs_name.empty())
            obs_to_check[item->info[0]->obs_name].push_back(item);
    }

    for (auto &item : obs_to_check)
    {
        std::sort(item.second.begin(), item.second.end(), sort_obs);
        for (int i = 0; i < item.second.size() - 1; i++)
        {
            if (!(item.second[i]->start < item.second[i + 1]->start
                  && item.second[i]->start < item.second[i + 1]->end
                  && item.second[i]->end <= item.second[i + 1]->start
                  && item.second[i]->end < item.second[i + 1]->end))
            {
                // not all is fine
                std::stringstream res_ss;
                res_ss << std::fixed << "Error: \n"
                       << item.second[i].get() << std::endl
                       << item.second[i + 1].get() << std::endl;
                res = res_ss.str();
                return -1;
            }
        }
    }
    return 0;
}

double countObsTotalLength(Satellites &sats)
{
    Observatories init_obs;
    for (auto &item : sats)
    {
        for (auto &interval : item.second.ints_observatories)
        {
            if (init_obs.find(interval->info[0]->obs_name) == init_obs.end())
            {
                Observatory ob;
                ob.full_schedule.push_back(interval);
                ob.name = interval->info[0]->obs_name;
                init_obs[interval->info[0]->obs_name] = ob;
            }
            else
            {
                init_obs[interval->info[0]->obs_name].full_schedule.push_back(interval);
            }
        }
    }

    double obs_total_length = 0.0;
    for (auto &ob : init_obs)
    {
        std::sort(ob.second.full_schedule.begin(), ob.second.full_schedule.end(), sort_obs);

        for (int i = 0; i < ob.second.full_schedule.size(); i++)
        {
            timepoint start = ob.second.full_schedule[i]->start;
            timepoint end = ob.second.full_schedule[i]->end;
            while (i + 1 < ob.second.full_schedule.size() && end >= ob.second.full_schedule[i + 1]->start)
            {
                // (S)--------------E
                //    S------E          S--------(E)
                //                S--------E            S-----E
                if (end < ob.second.full_schedule[i + 1]->end)
                    end = ob.second.full_schedule[i + 1]->end;
                i++;
            }
            obs_total_length += DURATION(start, end);
        }
    }
    return obs_total_length;
}