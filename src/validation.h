#ifndef VALIDATION
#define VALIDATION

#include <date.h>
#include <parser/parser.h>
#include <algos/algos.h>

bool sort_obs(std::shared_ptr<Interval> a, std::shared_ptr<Interval> b) {
    if (a->start == b->start)
        return a->duration > b->duration;
    return a->start < b->start;
}

int checkValidity(Satellites &sats, std::string &res) {
    Schedule schedule_to_check;

    for(auto &item : sats) {
        for(auto & interval : item.second.full_schedule) {
            schedule_to_check.insert(interval);
        }
    }

    std::unordered_map<std::string, std::vector<std::shared_ptr<Interval>>> obs_to_check;

    for(auto &item : schedule_to_check) {
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
                std::istringstream start_date("1/Jun/2027 00:00:00.000");
                timepoint tp_start;
                start_date >> date::parse("%d/%b/%Y %T", tp_start);
                    std::stringstream res_ss;
                    res_ss << "Error: \n" << item.second[i]->info[0]->sat_name <<
                          " " << DURATION(tp_start, item.second[i]->start) <<
                          " " << DURATION(tp_start, item.second[i]->end) <<
                          " " << item.second[i]->info[0]->state <<
                          " " << item.second[i]->capacity_change <<
                          " " << obs_to_hex[item.second[i]->info[0]->obs_name] <<
                          " " << obs_to_int[item.second[i]->info[0]->obs_name] <<
                          " " << item.second[i]->info[0]->obs_name << 
                          "\n"<< 
                                item.second[i+1]->info[0]->sat_name <<
                          " " << DURATION(tp_start, item.second[i+1]->start) <<
                          " " << DURATION(tp_start, item.second[i+1]->end) <<
                          " " << item.second[i+1]->info[0]->state <<
                          " " << item.second[i+1]->capacity_change <<
                          " " << obs_to_hex[item.second[i+1]->info[0]->obs_name] <<
                          " " << obs_to_int[item.second[i+1]->info[0]->obs_name] <<
                          " " << item.second[i+1]->info[0]->obs_name <<
                          "\n";
                res = res_ss.str();
                return -1;     
               }
        }

    }
    return 0;
}




#endif