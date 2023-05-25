#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#include <date.h>
#include <parser/parser.h>
#include <algos/algos.h>
#include <validation.h>

using namespace date;

int main()
{
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

    int sum_data = 0;
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
    std::cout << "Total data transmitted: " << sum_data << "\n";

    std::string err_check_str;
    int err_check_int = checkValidity(sats, err_check_str);
    if(err_check_int == -1) {
        std::cout << "Error while checking obs: " << err_check_str;
    } else {
        std::cout << "obs are fine!" << std::endl;
    }


    // Schedule check;

    Schedule check;

    for(auto &item : sats) {
        for(auto & interval : item.second.full_schedule) {
            check.insert(interval);
        }
    }

    int check_counter = 0;
    std::ofstream check_out("check.txt", std::ofstream::out);
    for(auto &interval : check) {
            if(interval->start == interval->end){
                check_counter++;
            }
            check_out << std::fixed << satName_to_num[interval->info[0]->sat_name] 
                << " " << interval->info[0]->sat_name
                << " "
                << (std::chrono::duration<double, std::milli>(interval->start - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << " " << (std::chrono::duration<double, std::milli>(interval->end - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << " " << interval->info[0]->state
                << " " << interval->capacity_change
                << " " << obs_to_int[interval->info[0]->obs_name]
                << " " << interval->info[0]->obs_name
                << '\n';
    }
    std::cout << "check_counter: " << check_counter << std::endl;
    check_out.close();
    out.close();
    out_schedule.close();
    sats_obs_out.close();





    return 0;
}
