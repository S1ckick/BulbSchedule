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


int writeResults(Satellites &sats, const std::string &path_ground, const std::string &path_camera, const std::string & path_drop, const std::unordered_map<std::string, uint32_t> &obs_to_int) {
    std::unordered_map<ObsName, std::pair<std::ofstream, long int>> obs_files_to_save;
    fs::current_path(fs::current_path());
    fs::create_directories(path_ground);
    fs::create_directories(path_drop);
    fs::create_directories(path_camera);
    for(auto &item : obs_to_int) {
        if(item.first == "" || item.first == "0")
            continue;
        std::ofstream obs_out_f(path_ground + "Ground_" + item.first + ".txt");
        obs_out_f << item.first << " - satellite name" << std::endl 
                  << "-------------------------" << std::endl
                  << "Access * Start Time (UTCG) * Stop Time (UTCG) * Duration (sec) * Sat name * Data (Mbytes)"
                  << std::endl;
        obs_files_to_save[item.second] = std::make_pair<std::ofstream, long int>(std::move(obs_out_f), 1);
    }

    double total = 0.0;
    for (int isat = 1; isat <= SAT_NUM; isat++){
        std::stringstream satname;
        satname << (isat <= 50 ? "KinoSat_" : "Zorkiy_") << COUT_SATNAME(isat);

        std::ofstream out_drop(path_drop + "Drop_" + satname.str() + ".txt");
        std::ofstream out_camera(path_camera + "Camera_" + satname.str() + ".txt");

        out_drop << satname.str() << " - satellite name" << std::endl 
          << "-------------------------" << std::endl
          << "Access * Start Time (UTCG) * Stop Time (UTCG) * Duration (sec) * Station * Data (Mbytes)"
          << std::endl;

        out_camera << satname.str() << " - satellite name" << std::endl 
          << "-------------------------" << std::endl
          << "Access * Start Time (UTCG) * Stop Time (UTCG) * Duration (sec) * Data (Mbytes)"
          << std::endl;

        double capacity_change = 0;
        long int i_camera = 1;
        long int i_drop = 1;
        long int i_ground = 1;
        for (auto &interval : sats[isat].full_schedule){
            auto &cur_info = interval.info;
            if (cur_info.state == State::RECORDING) {
                capacity_change = sats[isat].record(DURATION(interval.start, interval.end));

                // write to Camera
                out_camera << std::fixed << std::setprecision(3) << i_camera++
                  << " " << date::format("%e %b %Y %T", interval.start)
                  << " " << date::format("%e %b %Y %T", interval.end)
                  << " " << DURATION(interval.start, interval.end)
                  << " " << capacity_change * 128.0
                  << std::endl;
            } else {
                capacity_change = sats[isat].transmission(DURATION(interval.start, interval.end));
                
                // write to Ground
                std::ofstream & out_f = obs_files_to_save[cur_info.obs_name].first;
                out_f << std::fixed << std::setprecision(3) << obs_files_to_save[cur_info.obs_name].second++ 
                  << " " << date::format("%e %b %Y %T", interval.start)
                  << " " << date::format("%e %b %Y %T", interval.end)
                  << " " << DURATION(interval.start, interval.end)
                  << " " << satname.str()
                  << " " << capacity_change * 128.0 
                  << std::endl;

                // write to Drop
                out_drop << std::fixed << std::setprecision(3) << i_drop++
                  << " " << date::format("%e %b %Y %T", interval.start)
                  << " " << date::format("%e %b %Y %T", interval.end)
                  << " " << DURATION(interval.start, interval.end)
                  << " " << int_to_obs[interval.info.obs_name]
                  << " " << capacity_change * 128.0
                  << std::endl;
            }


        }
        out_camera.close();
        out_drop.close();
    }

    for(auto &item : obs_files_to_save) {
        item.second.first.close();
    }

    return 0;
}
