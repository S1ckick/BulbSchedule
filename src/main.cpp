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

void countDailySum(const Interval &interval, double capacity_change, std::vector<double> & daily_sums) {
    for(int i = 0; i < 14; i++){
        if(interval.start < START_MODELLING + std::chrono::hours((i+1) * 24)){
            if(interval.end < START_MODELLING + std::chrono::hours((i+1) * 24))
                daily_sums[i] += capacity_change;
            else {
                double speed = (interval.info.sat_name <= 50) ? 1 : 0.25;
                double eval_change = DURATION(interval.start, START_MODELLING + std::chrono::hours((i+1) * 24)) * speed;
                if(eval_change < capacity_change){
                    daily_sums[i] += eval_change;
                    daily_sums[i+1] += (capacity_change - eval_change);
                } else {
                    daily_sums[i] += capacity_change;
                }
            }
            return;
        }
    }
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
        int res_parse_obs = parse_observatory(path2, sats);

        auto start_algo = std::chrono::high_resolution_clock::now();
        //algos::greedy_capacity(sats);
        algos::bysolver(sats);
        auto end_algo = std::chrono::high_resolution_clock::now();

        std::cout << "Schedule built in " << std::chrono::duration_cast<std::chrono::seconds>(end_algo - start_algo) << std::endl;

        fs::current_path(fs::current_path());

        std::string res_dir = "Results/";

        fs::create_directories(res_dir);
        std::ofstream out_schedule(res_dir + "all_schedule.txt");
        std::ofstream sats_obs_out("sats_obs.txt");
        std::ofstream out("sats.txt");
        std::ofstream sat_full(res_dir + "time_sat_full.txt");

        if (!out || !out_schedule || !sats_obs_out) {
            std::cout << "Can't create files\n";
        }

        std::vector<double> daily_sums(14,0.0);
        int cnt_sat = 1;

        for (auto &item : sats){
            //std::cout << "Writing schedule: " << cnt_sat++ << "/" << sats.size() << "\n";
            for (auto &interval : item.second.ints_in_area){
                out << std::fixed 
                    << interval.info.sat_name << " "
                    COUT_SATNAME(interval.info.sat_name) << " "
                    << (DURATION(START_MODELLING, interval.start) * 1000) //milliseconds
                    << " " << (DURATION(START_MODELLING, interval.end) * 1000) //milliseconds
                    << " " 
                    << std::endl;
            }
        }
        out.close();

        for (auto &item : sats){
            for (auto &interval : item.second.ints_observatories){
                sats_obs_out << std::fixed
                    << interval.info.sat_name << " "
                    COUT_SATNAME(interval.info.sat_name) << " "
                    << (DURATION(START_MODELLING, interval.start) * 1000) //milliseconds
                    << " " << (DURATION(START_MODELLING, interval.end) * 1000) //milliseconds
                    << " " << interval.info.obs_name 
                    << " " << obs_to_hex[interval.info.obs_name]
                    << " " << interval.info.obs_name
                    << std::endl;
            }
        }
        sats_obs_out.close();

        double sum_data = 0;

        for (auto &item : sats){
            item.second.capacity = 0; // simulate satellite work from beginning

            double time_full = 0;
            timepoint became_full = item.second.full_schedule[0].start;

            for (auto &interval : item.second.full_schedule){
                auto &cur_info = interval.info;
                double capacity_change = 0;
                if (interval.info.state == State::RECORDING) {
                    capacity_change = item.second.record(DURATION(interval.start, interval.end));
                    if (item.second.capacity >= item.second.max_capacity - 1e-8) {
                        time_full += DURATION(interval.start + std::chrono::milliseconds((uint64_t)(capacity_change / item.second.recording_speed * 1000)), interval.end);
                    }
                }
                else if (interval.info.state == State::TRANSMISSION) {
                    // if (item.second.capacity >= item.second.max_capacity - 1e-8) {
                    //     time_full += DURATION(became_full, interval.start) * 1000;
                    // }
                    capacity_change = item.second.transmission(DURATION(interval.start, interval.end));
                    sum_data += capacity_change;
                    countDailySum(interval, capacity_change, daily_sums);
                }

                out_schedule << std::fixed << cur_info.sat_name
                    << " " << COUT_SATNAME(cur_info.sat_name)
                    << " "
                    << (DURATION(START_MODELLING, interval.start) * 1000) //milliseconds
                    << " " << (DURATION(START_MODELLING, interval.end) * 1000)  //milliseconds
                    << " " << interval.info.state
                    << " " << capacity_change
                    << " " << obs_to_hex[cur_info.obs_name]
                    << " " << cur_info.obs_name
                    << std::endl;                    
            }
                sat_full << std::fixed << item.first << ": " << int(time_full) << " ms\n";
        }

        double daily_checksum = 0.0;
        std::cout << std::fixed << std::setprecision(18) << "Data transmitted daily: \n";
        for(int i = 0; i < daily_sums.size(); i++){
            std::cout << i+1 << " day: " << daily_sums[i] << " Gbit \n";
            daily_checksum += daily_sums[i];
        }
        std::cout << "Checksum: " << daily_checksum << std::endl;
        sat_full.close();
        out_schedule.close();

        std::cout << "Total data transmitted: " << std::fixed << std::setprecision(18) << sum_data << " Gbit \n";
        std::cout << "The schedule is saved in a folder: " << res_dir << std::endl;

        // double obs_total_length = countObsTotalLength(sats);
        // std::cout << std::fixed << "stations can receive max: " << obs_total_length << " sec" << std::endl;


        // write result

        std::string path_ground = res_dir + "Ground/";
        std::string path_camera = res_dir + "Camera/";
        std::string path_drop = res_dir + "Drop/";
        writeResults(sats, path_ground, path_camera, path_drop, obs_to_int);


        // Validation

        // VecSchedule sats_to_check;
        // std::string filename_sats_to_check = res_dir + "all_schedule.txt";
        // parse_schedule(sats_to_check, filename_sats_to_check, START_MODELLING);
        // std::sort(sats_to_check.begin(), sats_to_check.end(), sort_for_parsed_schedule);

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

        // if(checkBroadcastInRightArea(sats_to_check, sats, err_check_str) == -1) {
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
