#include "validation.h"

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

std::unordered_map<std::string, std::string> obs_to_hex = {
    {"Anadyr1","#FF2D00"},
    {"Anadyr2", "#FF8700"},
    {"CapeTown", "#FAFF00"},
    {"Delhi", "#05FFAF"},
    {"Irkutsk", "#05FFF7"},
    {"Magadan1", "#05C5FF"},
    {"Magadan2", "#0599FF"},
    {"Moscow", "#0546FF"},
    {"Murmansk1", "#9705FF"},
    {"Murmansk2", "#D905FF"},
    {"Norilsk", "#FF05ED"},
    {"Novosib", "#FF059A"},
    {"RioGallegos", "#5890A7"},
    {"Sumatra", "#388357"}
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

int checkZeroIntervals(Schedule &schedule_to_check, std::string &res) {
    for(auto &interval : schedule_to_check) {
        if(interval->start == interval->end) {
            std::stringstream res_ss;
            std::istringstream start_date("1/Jun/2027 00:00:00.000");
            timepoint tp_start;
            start_date >> date::parse("%d/%b/%Y %T", tp_start);
            res_ss << std::fixed << "Error: \n" << interval->info[0]->sat_name <<
                          " " << (DURATION(tp_start, interval->start) * 1000) <<
                          " " << (DURATION(tp_start, interval->end) * 1000) <<
                          " " << interval->info[0]->state <<
                          " " << interval->capacity_change <<
                          " " << obs_to_hex[interval->info[0]->obs_name] <<
                          " " << obs_to_int[interval->info[0]->obs_name] <<
                          " " << interval->info[0]->obs_name << std::endl;
            res = res_ss.str();
            return -1;
        }
    }
    return 0;
}

int checkValidity(Schedule &schedule_to_check, std::string &res) {
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
                    res_ss << std::fixed << "Error: \n" << item.second[i]->info[0]->sat_name <<
                          " " << (DURATION(tp_start, item.second[i]->start) * 1000) <<
                          " " << (DURATION(tp_start, item.second[i]->end) * 1000) <<
                          " " << item.second[i]->info[0]->state <<
                          " " << item.second[i]->capacity_change <<
                          " " << obs_to_hex[item.second[i]->info[0]->obs_name] <<
                          " " << obs_to_int[item.second[i]->info[0]->obs_name] <<
                          " " << item.second[i]->info[0]->obs_name << 
                          std::endl << 
                                item.second[i+1]->info[0]->sat_name <<
                          " " << (DURATION(tp_start, item.second[i+1]->start) * 1000) <<
                          " " << (DURATION(tp_start, item.second[i+1]->end) * 1000) <<
                          " " << item.second[i+1]->info[0]->state <<
                          " " << item.second[i+1]->capacity_change <<
                          " " << obs_to_hex[item.second[i+1]->info[0]->obs_name] <<
                          " " << obs_to_int[item.second[i+1]->info[0]->obs_name] <<
                          " " << item.second[i+1]->info[0]->obs_name <<
                          std::endl;
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