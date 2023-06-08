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

std::unordered_map<int,std::string> int_to_obs = {
    {0,"0"},
    {1,"Anadyr1"},
    {2,"Anadyr2"},
    {3,"CapeTown"},
    {4,"Delhi"},
    {5,"Irkutsk"},
    {6,"Magadan1"},
    {7,"Magadan2"},
    {8,"Moscow"},
    {9,"Murmansk1"},
    {10,"Murmansk2"},
    {11,"Norilsk"},
    {12,"Novosib"},
    {13,"RioGallegos"},
    {14,"Sumatra"}
};

/********TODO*********/
// int write_res_obs(const Satellites &sats, const std::string &path, const std::unordered_map<std::string, uint32_t> &obs_to_int) {
//     std::unordered_map<ObsName, std::pair<std::ofstream, long int>> obs_files_to_save;
//     fs::current_path(fs::current_path());
//     fs::create_directories(path);
//     for(auto &item : obs_to_int) {
//         if(item.first == "" || item.first == "0")
//             continue;
//         std::ofstream obs_out_f(path + item.first + "_result.txt");
//         obs_out_f << item.first << std::endl 
//                   << "Access StartTime(UTCG) StopTime(UTCG) Duration(sec) Satname Data(Mbytes)"
//                   << std::endl;
//         obs_files_to_save[item.second] = std::make_pair<std::ofstream, long int>(std::move(obs_out_f), 1);
//     }

//     double total = 0.0;
//     for (auto &item : sats){
//         for (auto &interval : item.second.full_schedule){
//             auto &cur_info = interval.info;
//             if(cur_info.state != State::TRANSMISSION)
//                 continue;
//             std::ofstream & out_f = obs_files_to_save[cur_info.obs_name].first;
//             out_f << std::fixed << std::setprecision(9) << obs_files_to_save[cur_info.obs_name].second++ 
//                   << " " << date::format("%e %b %Y %T", interval.start)
//                   << " " << date::format("%e %b %Y %T", interval.end)
//                   << " " << DURATION(interval.start, interval.end)
//                   << " " << cur_info.sat_name 
//                   << " " << interval->capacity_change * 128.0 
//                   << std::endl;
//         }
//     }

//     for(auto &item : obs_files_to_save) {
//         item.second.first.close();
//     }

//     return 0;
// }

// int write_res_sats(const Satellites &sats, const std::string &path) {
//     fs::current_path(fs::current_path());
//     fs::create_directories(path);

//     for (auto &item : sats){
//         std::ofstream sat_out_f(path + std::to_string(item.first) + "_result.txt");
//         sat_out_f << item.first << std::endl 
//           << "Access StartTime(UTCG) StopTime(UTCG) Duration(sec) Data(Mbytes) Activity Obsname"
//           << std::endl;
//         long int i = 1;
//         for (auto &interval : item.second.full_schedule){
//             auto &cur_info = interval.info;
//             sat_out_f << std::fixed << std::setprecision(9) << i++
//                   << " " << date::format("%e %b %Y %T", interval.start)
//                   << " " << date::format("%e %b %Y %T", interval.end)
//                   << " " << DURATION(interval.start, interval.end)
//                 //   << " " << interval->capacity_change * 128.0
//                 //   << " " << interval->info[0]->state 
//                   << " " << int_to_obs[interval.info.obs_name]
//                   << std::endl;
//         }
//         sat_out_f.close();
//     }

//     return 0;
// }