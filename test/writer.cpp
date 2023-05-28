#include "writer.h"
#include <fstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

inline std::ostream& operator<<(std::ostream& os, const State& obj)
{
   if(obj == State::IDLE) {
    os << "IDLE";
   }
   if(obj == State::TRANSMISSION){
    os << "TRANSMISSION";
   }
   if(obj == State::RECORDING) {
    os << "RECORDING";
   }
   return os;
}

int write_res_obs(Satellites &sats, std::string &path, std::unordered_map<std::string, int> &obs_to_int) {
    std::unordered_map<ObsName, std::pair<std::ofstream, long int>> obs_files_to_save;
    fs::current_path(fs::current_path());
    fs::create_directories(path);
    for(auto &item : obs_to_int) {
        if(item.first == "")
            continue;
        std::ofstream obs_out_f(path + item.first + "_result.txt");
        obs_out_f << item.first << std::endl 
                  << "Access StartTime(UTCG) StopTime(UTCG) Duration(sec) Satname Data(Mbytes)"
                  << std::endl;
        obs_files_to_save[item.first] = std::make_pair<std::ofstream, long int>(std::move(obs_out_f), 1);
    }

    for (auto &item : sats){
        for (auto &interval : item.second.full_schedule){
            auto &cur_info = interval->info[0];
            std::ofstream & out_f = obs_files_to_save[cur_info->obs_name].first;
            out_f << std::fixed << std::setprecision(9) << obs_files_to_save[cur_info->obs_name].second++ 
                  << " " << date::format("%e %b %Y %T", interval->start)
                  << " " << date::format("%e %b %Y %T", interval->end)
                  << " " << DURATION(interval->start, interval->end)
                  << " " << cur_info->sat_name 
                  << " " << interval->capacity_change / 1000.0 << std::endl;
        }
    }

    for(auto &item : obs_files_to_save) {
        item.second.first.close();
    }

    return 0;
}

int write_res_sats(Satellites &sats, std::string &path) {
    fs::current_path(fs::current_path());
    fs::create_directories(path);

    for (auto &item : sats){
        std::ofstream sat_out_f(path + std::to_string(item.first) + "_result.txt");
        sat_out_f << item.first << std::endl 
          << "Access StartTime(UTCG) StopTime(UTCG) Duration(sec) Data(Mbytes) Activity Obsname"
          << std::endl;
        long int i = 1;
        for (auto &interval : item.second.full_schedule){
            auto &cur_info = interval->info[0];
            sat_out_f << std::fixed << std::setprecision(9) << i++
                  << " " << date::format("%e %b %Y %T", interval->start)
                  << " " << date::format("%e %b %Y %T", interval->end)
                  << " " << DURATION(interval->start, interval->end)
                  << " " << interval->capacity_change / 1000.0 
                  << " " << interval->info[0]->state 
                  << " " << interval->info[0]->obs_name
                  << std::endl;
        }
        sat_out_f.close();
    }

    return 0;
}