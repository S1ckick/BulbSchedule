#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

#include "validation.h"
#include "writer.h"

using namespace date;

std::unordered_map<std::string, uint32_t> obs_to_int = {
    {"0", 0},
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

std::unordered_map<uint32_t, std::string> obs_to_hex = {
    {1,"#FF2D00"},
    {2, "#FF8700"},
    {3, "#FAFF00"},
    {4, "#05FFAF"},
    {5, "#05FFF7"},
    {6, "#05C5FF"},
    {7, "#0599FF"},
    {8, "#0546FF"},
    {9, "#9705FF"},
    {10, "#D905FF"},
    {11, "#FF05ED"},
    {12, "#FF059A"},
    {13, "#5890A7"},
    {14, "#388357"}
};

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

timepoint START_MODELLING{std::chrono::milliseconds{1811808000000}};

void help ()
{
    fprintf(stderr, "\nUsage: scheduler [-data1 path] [-data2 path] [-algo algorithm]\n"
            "  data1 is the path to Satellite-Russia visibility files. Default DATA_Files/Russia2Constellation2\n"
            "  data2 is the part to Satellite-station LOS files. Default DATA_Files/Facility2Constellation\n");
    
}

int main(int argc, char* argv[])
{
    Satellites sats;
    std::string path1 = "DATA_Files/Russia2Constellation2";
    std::string path2 = "DATA_Files/Facility2Constellation";

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-data1") == 0)
        {
            if (i < argc - 1)
                path1 = argv[++i];
            else
            {
                help();
                return -1;
            }
        }
        else if (strcmp(argv[i], "-data2") == 0)
        {
            if (i < argc - 1)
                path2 = argv[++i];
            else
            {
                help();
                return -1;
            }
        }
        else if (strcmp(argv[i], "-algo") == 0)
        {
            if (i < argc - 1)
                ++i;
            else
            {
                help();
                return -1;
            }
        }
        else
        {
            help();
            return -1;
        }
    }
    
    try
    {
        int res_parse_russia = parse_russia_to_satellites(path1, sats);
        Observatories obs;
        int res_parse_obs = parse_observatory(path2, obs, sats);

        auto start_algo = std::chrono::high_resolution_clock::now();
        algos::greedy_capacity(sats, obs);
        auto end_algo = std::chrono::high_resolution_clock::now();

        std::cout << "Schedule built in " << std::chrono::duration_cast<std::chrono::seconds>(end_algo - start_algo) << std::endl;


        fs::current_path(fs::current_path());

        std::string res_dir = "results/";

        fs::create_directories(res_dir);
        std::ofstream out_schedule(res_dir + "all_schedule.txt");

        std::ofstream sats_obs_out("sats_obs.txt");
        std::ofstream out("sats.txt");

        if (!out || !out_schedule || !sats_obs_out) {
            std::cout << "Can't create files\n";
        }

        std::unordered_map<int,int> satName_to_num;

        std::vector<int> all_sat_names(200);

        int sat_i = 0;
        for(auto &item : sats){
            all_sat_names[sat_i] = item.first;
            sat_i++;
        }
        std::sort(all_sat_names.begin(), all_sat_names.end());
        for(int ii = 0; ii < 200; ii++){
            satName_to_num[all_sat_names[ii]] = ii;
        }

        double sum_data = 0;
        int cnt_sat = 1;
        for (auto &item : sats){
            item.second.capacity = 0;
            //std::cout << "Writing schedule: " << cnt_sat++ << "/" << sats.size() << "\n";
            for (auto &interval : item.second.ints_in_area){
                // out << std::fixed << satName_to_num[interval.info[0].sat_name] 
                //     << " " << interval.info[0].sat_name << " "
                //     << (DURATION(START_MODELLING, interval.start) * 1000) //milliseconds
                //     << " " << (DURATION(START_MODELLING, interval.end) * 1000) //milliseconds
                //     << " " 
                //     << std::endl;
            }

            for (auto &interval : item.second.ints_observatories){
                // sats_obs_out << std::fixed << satName_to_num[interval.info[0].sat_name] 
                //     << " " << interval.info[0].sat_name << " "
                //     << (DURATION(START_MODELLING, interval.start) * 1000) //milliseconds
                //     << " " << (DURATION(START_MODELLING, interval.end) * 1000) //milliseconds
                //     << " " << interval.info[0].obs_name 
                //     << " " << obs_to_hex[interval.info[0].obs_name]
                //     << " " << interval.info[0].obs_name
                //     << std::endl;
            }

            for (auto &interval : item.second.full_schedule){
                auto &cur_info = interval.info;
                if (interval.info.state == State::RECORDING)
                    item.second.record(DURATION(interval.start, interval.end));
                else if (interval.info.state == State::TRANSMISSION)
                    sum_data += item.second.transmission(DURATION(interval.start, interval.end));
                // out_schedule << std::fixed << satName_to_num[cur_info.sat_name] 
                //     << " " << cur_info.sat_name
                //     << " "
                //     << (DURATION(START_MODELLING, interval.start) * 1000) //milliseconds
                //     << " " << (DURATION(START_MODELLING, interval.end) * 1000)  //milliseconds
                //     << " " << interval.info[0].state
                //     << " " << interval.capacity_change
                //     << " " << obs_to_hex[cur_info.obs_name]
                //     << " " << cur_info.obs_name
                //     << std::endl;

                // if (cur_info.state == State::TRANSMISSION)
                //     sum_data += interval.capacity_change;
            }
        }
        std::cout << "Total data transmitted: " << std::fixed << std::setprecision(18) << sum_data << " Gbit \n";
        std::cout << "The schedule is saved in a folder: " << res_dir << std::endl;

        out.close();
        out_schedule.close();
        sats_obs_out.close();

        // double obs_total_length = countObsTotalLength(sats);
        // std::cout << std::fixed << "stations can receive max: " << obs_total_length << " sec" << std::endl;


        // write result

        std::string path_to_res_obs = res_dir + "observatories/";
        write_res_obs(sats, path_to_res_obs, obs_to_int);
        std::string path_to_res_sats = res_dir + "satellites/";
        write_res_sats(sats, path_to_res_sats);

        // Validation

        // VecSchedule sats_to_check;
        // std::string filename_sats_to_check = res_dir + "all_schedule.txt";
        // parse_schedule(sats_to_check, filename_sats_to_check, START_MODELLING);
        // std::sort(sats_to_check.begin(), sats_to_check.end(), sort_obs);

        // std::cout << sats_to_check.size() << std::endl;
        // std::string err_check_str;

        // if(checkZeroIntervals(sats_to_check, err_check_str) == -1){
        //     std::cout << "Zero interval " << err_check_str;
        // } else {
        //     std::cout << "No zero intervals." << std::endl;
        // }

        // if(checkValidity(sats_to_check, err_check_str) == -1) {
        //     std::cout << "Error while checking obs: " << err_check_str;
        // } else {
        //     std::cout << "obs are fine!" << std::endl;
        // }

        // if(checkBroadcastInRightArea(sats_to_check, obs, err_check_str) == -1) {
        //    std::cout << "Error while checking transmission area: " << err_check_str;
        // } else {
        //     std::cout << "all satellites transmission in right area" << std::endl;
        // }

        // if(checkRecordingInRightArea(sats_to_check, sats, err_check_str) == -1) {
        //    std::cout << "Error while checking recording area: " << err_check_str;
        // } else {
        //     std::cout << "all satellites record in right area" << std::endl;
        // }
    }
    catch (std::filesystem::filesystem_error &err)
    {
        std::cerr << err.what() << std::endl;
        help();
        return -1;
    }
    catch (std::exception &err)
    {
        std::cerr << err.what() << std::endl;
        return -1;
    }

    return 0;
}
