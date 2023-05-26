#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <iomanip>

#include "validation.h"

using namespace date;

int main()
{
    std::unordered_map<SatName, Satellite> sats;
    const char russia_location[] = "../../DATA_Files/Russia2Constellation2/";
    int res_parse_russia = parse_russia_to_satellites(russia_location, sats);

    std::unordered_map<std::string, Observatory> obs;
    const char facility_location[] = "../../DATA_Files/Facility2Constellation/";
    int res_parse_obs = parse_observatory(facility_location, obs, sats);
    algos::greedy_capacity(sats, obs);

    std::ofstream out("sats.txt", std::ofstream::out);
    std::ofstream out_schedule("sats_schedule.txt", std::ofstream::out);
    std::ofstream sats_obs_out("sats_obs.txt", std::ofstream::out);

    std::istringstream start_date("1/Jun/2027 00:00:00.000");
    timepoint tp_start;
    start_date >> date::parse("%d/%b/%Y %T", tp_start);


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
                << (DURATION(tp_start, interval->start) * 1000) //milliseconds
                << " " << (DURATION(tp_start, interval->end) * 1000) //milliseconds
                << " " 
                << std::endl;
        }

        for (auto &interval : item.second.ints_observatories){
            sats_obs_out << std::fixed << satName_to_num[interval->info[0]->sat_name] 
                << " " << interval->info[0]->sat_name << " "
                << (DURATION(tp_start, interval->start) * 1000) //milliseconds
                << " " << (DURATION(tp_start, interval->end) * 1000) //milliseconds
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
                << (DURATION(tp_start, interval->start) * 1000) //milliseconds
                << " " << (DURATION(tp_start, interval->end) * 1000)  //milliseconds
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

    out.close();
    out_schedule.close();
    sats_obs_out.close();

    double obs_total_length = countObsTotalLength(sats);
    std::cout << std::fixed << "stations can receive max: " << obs_total_length << " sec" << std::endl;

    std::cout << "Total data transmitted: " << std::fixed << std::setprecision(18) << sum_data << "\n";

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


    return 0;
}
