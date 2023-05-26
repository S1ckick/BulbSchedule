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