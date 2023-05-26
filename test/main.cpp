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
                << DURATION(tp_start, interval->start)
                << " " << DURATION(tp_start, interval->end)
                << " " 
                << '\n';
        }

        for (auto &interval : item.second.ints_observatories){
            sats_obs_out << std::fixed << satName_to_num[interval->info[0]->sat_name] 
                << " " << interval->info[0]->sat_name << " "
                << DURATION(tp_start, interval->start)
                << " " << DURATION(tp_start, interval->end)
                << " " << interval->info[0]->obs_name 
                << " " << obs_to_hex[interval->info[0]->obs_name]
                << " " << obs_to_int[interval->info[0]->obs_name]
                << '\n';
        }

        for (auto &interval : item.second.full_schedule){
            out_schedule << std::fixed << satName_to_num[interval->info[0]->sat_name] 
                << " " << interval->info[0]->sat_name
                << " " << interval->info[0]->sat_type
                << " "
                << DURATION(tp_start, interval->start)
                << " " << DURATION(tp_start, interval->end)
                << " " << interval->info[0]->state
                << " " << interval->capacity_change
                << " " << obs_to_hex[interval->info[0]->obs_name]
                << " " << obs_to_int[interval->info[0]->obs_name]
                << " " << interval->info[0]->obs_name
                << '\n';

            if (interval->info[0]->state == State::BROADCAST)
                sum_data += interval->capacity_change;
        }

    }

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
    std::cout << "are we here?" << std::endl;
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

    std::cout << std::fixed << "stations can receive max: " << obs_total_length << std::endl;

    std::cout << "Total data transmitted: " << std::fixed << std::setprecision(18) << sum_data << "\n";

    std::string err_check_str;
    int err_check_int = checkValidity(sats, err_check_str);
    if(err_check_int == -1) {
        std::cout << "Error while checking obs: " << err_check_str;
    } else {
        std::cout << "obs are fine!" << std::endl;
    }


    VecSchedule sats_to_check;
    parse_schedule(sats_to_check);
    std::ofstream check_file("check.txt", std::ofstream::out);
    for(auto &interval : sats_to_check) {
        check_file << std::fixed << satName_to_num[interval->info[0]->sat_name] 
        << " " << interval->info[0]->sat_name
        << " " << interval->info[0]->sat_type
        << " "
        << DURATION(tp_start, interval->start)
        << " " << DURATION(tp_start, interval->end)
        << " " << interval->info[0]->state
        << " " << interval->capacity_change
        << " " << obs_to_hex[interval->info[0]->obs_name]
        << " " << obs_to_int[interval->info[0]->obs_name]
        << " " << interval->info[0]->obs_name
        << '\n';
    }





    check_file.close();
    out.close();
    out_schedule.close();
    sats_obs_out.close();

    return 0;
}
