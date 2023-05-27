#include "writer.h"
#include <fstream>
#include <iomanip>


int write_res_obs(Satellites &sats, std::unordered_map<std::string, int> &obs_to_int) {
    std::unordered_map<ObsName, std::pair<std::ofstream, long int>> obs_files_to_save;

    for(auto &item : obs_to_int) {
        if(item.first == "")
            continue;
        std::ofstream obs_out_f(item.first + "_result.txt");
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
    return 0;
}