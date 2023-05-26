#include "validation.h"

int checkRecordingInRightArea(VecSchedule &schedule_to_check, Satellites &sats, std::string &res) {
    for(auto &interval : schedule_to_check) {
        auto &sat = sats.at(interval->info[0]->sat_name);
        int found = 0;
        for(auto &area : sat.ints_in_area) {
            if(interval->start >= area->start && interval->end <= area->end){
                found = 1;
                break;
            }
        }
        if(found == 0) {
            std::stringstream res_ss;
            res_ss << std::fixed 
                   << "Error: \n" 
                   << *interval << std::endl;
            res = res_ss.str();
            return -1;
        }
    }
    return 0;
}

int checkForIntervalsIntersection(VecSchedule &schedule_to_check, std::string &res) {
    for(int i = 0; i < schedule_to_check.size()-1; i++){
        if(schedule_to_check[i]->start == schedule_to_check[i+1]->start && 
           schedule_to_check[i]->end == schedule_to_check[i+1]->end)
                continue;

        if(schedule_to_check[i]->end > schedule_to_check[i+1]->start) {
            std::stringstream res_ss;
            res_ss << std::fixed << "Error: \n" 
                   << *schedule_to_check[i] << std::endl 
                   << *schedule_to_check[i+1] << std::endl;
            res = res_ss.str();
            return -1;
        }
    }
    return 0;
}

int checkZeroIntervals(VecSchedule &schedule_to_check, std::string &res) {
    for(auto &interval : schedule_to_check) {
        if(interval->start == interval->end) {
            std::stringstream res_ss;
            res_ss << std::fixed << "Error: \n" << *interval << std::endl;
            res = res_ss.str();
            return -1;
        }
    }
    return 0;
}

int checkValidity(VecSchedule &schedule_to_check, std::string &res) {
    std::unordered_map<std::string, std::vector<std::shared_ptr<Interval>>> obs_to_check;

    for(auto &item : schedule_to_check) {
        if(item->info[0]->obs_name != "0")
            obs_to_check[item->info[0]->obs_name].push_back(item);
    }

    for(auto &item : obs_to_check) {
        std::sort(item.second.begin(), item.second.end(), sort_obs);
        for(int i = 0; i < item.second.size() - 1; i++) {
            if(item.second[i]->start < item.second[i+1]->start
               && item.second[i]->start < item.second[i+1]->end
               && item.second[i]->end <= item.second[i+1]->start
               && item.second[i]->end < item.second[i+1]->end){
                // all is fine
               } else {
                // not all is fine
                std::stringstream res_ss;
                res_ss << std::fixed << 
                        "Error: \n" << *item.second[i] << 
                        std::endl << *item.second[i+1] << std::endl;
                res = res_ss.str();
                return -1;     
               }
        }
    }
    return 0;
}

double countObsTotalLength(Satellites &sats) {
    Observatories init_obs;
    for(auto &item : sats) {
        for (auto &interval : item.second.ints_observatories){
            if(init_obs.find(interval->info[0]->obs_name) == init_obs.end()){
                Observatory ob;
                ob.full_schedule.push_back(interval);
                ob.name = interval->info[0]->obs_name;
                init_obs[interval->info[0]->obs_name] = ob;
            } else {
                init_obs[interval->info[0]->obs_name].full_schedule.push_back(interval);
            }

        }
    }

    double obs_total_length = 0.0;
    for(auto &ob : init_obs) {
        std::sort(ob.second.full_schedule.begin(), ob.second.full_schedule.end(), sort_obs);

        for(int i = 0; i < ob.second.full_schedule.size(); i++){
            timepoint start = ob.second.full_schedule[i]->start;
            timepoint end = ob.second.full_schedule[i]->end;
            while(i+1 < ob.second.full_schedule.size() && end >= ob.second.full_schedule[i+1]->start) {
                // (S)--------------E
                //    S------E          S--------(E)
                //                S--------E            S-----E
                if(end < ob.second.full_schedule[i+1]->end)
                    end = ob.second.full_schedule[i+1]->end;
                i++;
            }
            obs_total_length += DURATION(start,end);
        }
    }
    return obs_total_length;
}