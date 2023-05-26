#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <iomanip>

#include "validation.h"

using namespace date;

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

timepoint TP_START;

int main()
{

    std::istringstream start_date("1/Jun/2027 00:00:00.000");
    start_date >> date::parse("%d/%b/%Y %T", TP_START);

    Satellites sats;
    const char russia_location[] = "../../DATA_Files/Russia2Constellation2/";
    int res_parse_russia = parse_russia_to_satellites(russia_location, sats);

    Observatories obs;
    const char facility_location[] = "../../DATA_Files/Facility2Constellation/";
    int res_parse_obs = parse_observatory(facility_location, obs, sats);
    algos::greedy_exhaustive(sats, obs);

    std::ofstream out("sats.txt", std::ofstream::out);
    std::ofstream out_schedule("sats_schedule.txt", std::ofstream::out);
    std::ofstream sats_obs_out("sats_obs.txt", std::ofstream::out);

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
    for (auto &item : sats){
        for (auto &interval : item.second.ints_in_area){
            out << std::fixed << satName_to_num[interval->info[0]->sat_name] 
                << " " << interval->info[0]->sat_name << " "
                << (DURATION(TP_START, interval->start) * 1000) //milliseconds
                << " " << (DURATION(TP_START, interval->end) * 1000) //milliseconds
                << " " 
                << std::endl;
        }

        for (auto &interval : item.second.ints_observatories){
            sats_obs_out << std::fixed << satName_to_num[interval->info[0]->sat_name] 
                << " " << interval->info[0]->sat_name << " "
                << (DURATION(TP_START, interval->start) * 1000) //milliseconds
                << " " << (DURATION(TP_START, interval->end) * 1000) //milliseconds
                << " " << interval->info[0]->obs_name 
                << " " << obs_to_hex[interval->info[0]->obs_name]
                << " " << obs_to_int[interval->info[0]->obs_name]
                << std::endl;
        }

        for (auto &interval : item.second.full_schedule){
            out_schedule << std::fixed << satName_to_num[interval->info[0]->sat_name] 
                << " " << interval->info[0]->sat_name
                << " " << interval->info[0]->sat_type
                << " "
                << (DURATION(TP_START, interval->start) * 1000) //milliseconds
                << " " << (DURATION(TP_START, interval->end) * 1000)  //milliseconds
                << " " << interval->info[0]->state
                << " " << interval->capacity_change
                << " " << obs_to_hex[interval->info[0]->obs_name]
                << " " << obs_to_int[interval->info[0]->obs_name]
                << " " << interval->info[0]->obs_name
                << std::endl;

            if (interval->info[0]->state == State::BROADCAST)
                sum_data += interval->capacity_change;
        }

    }
    std::cout << "Total data transmitted: " << std::fixed << std::setprecision(18) << sum_data << " Gb \n";

    out.close();
    out_schedule.close();
    sats_obs_out.close();

    double obs_total_length = countObsTotalLength(sats);
    std::cout << std::fixed << "stations can receive max: " << obs_total_length << " sec" << std::endl;


    VecSchedule sats_to_check;
    std::string filename_sats_to_check = "sats_schedule.txt";
    parse_schedule(sats_to_check, filename_sats_to_check);
    std::sort(sats_to_check.begin(), sats_to_check.end(), sort_obs);

    std::cout << sats_to_check.size() << std::endl;
    std::string err_check_str;

    if(checkForIntervalsIntersection(sats_to_check, err_check_str) == -1){
        std::cout << "Intervals intersection " << err_check_str;
    } else {
        std::cout << "No intervals intersections." << std::endl;
    }

    if(checkZeroIntervals(sats_to_check, err_check_str) == -1){
        std::cout << "Zero interval " << err_check_str;
    } else {
        std::cout << "No zero intervals." << std::endl;
    }
    
    if(checkValidity(sats_to_check, err_check_str) == -1) {
        std::cout << "Error while checking obs: " << err_check_str;
    } else {
        std::cout << "obs are fine!" << std::endl;
    }

    if(checkRecordingInRightArea(sats_to_check, sats, err_check_str) == -1) {
       std::cout << "Error while checking recording in right area: " << err_check_str;
    } else {
        std::cout << "all satellites record in right area" << std::endl;
    }


    return 0;
}
