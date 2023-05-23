#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#include <date.h>
#include <parser/parser.h>
#include <algos/algos.h>

using namespace date;

int main()
{
    std::unordered_map<SatName, Satellite> sats;
    const char russia_location[] = "../../DATA_Files/Russia2Constellation2/";
    int res_parse_russia = parse_russia_to_satellites(russia_location, sats);

    std::unordered_map<std::string, Observatory> obs;
    const char facility_location[] = "../../DATA_Files/Facility2Constellation/";
    int res_parse_obs = parse_observatory(facility_location, obs, sats);
    algos::build_schedule(sats);
    
    std::ofstream out("sats.txt", std::ofstream::out);
    std::ofstream out_schedule("sats_schedule.txt", std::ofstream::out);

    std::istringstream start_date("1/Jun/2027 00:00:00.000");
    timepoint tp_start;
    start_date >> date::parse("%d/%b/%Y %T", tp_start);


    std::unordered_map<int,int> satName_to_num;

    int sat_i = 0;
    for(auto &item : sats)
        satName_to_num[item.first] = sat_i++;

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

    for (auto &item : sats){
        for (auto &interval : item.second.ints_in_area){
            out << std::fixed << satName_to_num[interval->sat_name] 
                << " " << interval->sat_name << " "
                << (std::chrono::duration<double, std::milli>(interval->start - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << " " << (std::chrono::duration<double, std::milli>(interval->end - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << " " 
                << '\n';
        }

        for (auto &interval : item.second.full_schedule){
            out_schedule << std::fixed << satName_to_num[interval->sat_name] 
                << " " << interval->sat_name
                << " "
                << (std::chrono::duration<double, std::milli>(interval->start - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << " " << (std::chrono::duration<double, std::milli>(interval->end - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << " " << interval->state
                << " " << interval->capacity_change
                << " " << obs_to_int[interval->obs_name]
                << " " << interval->obs_name
                << '\n';
        }

    }

    out.close();

    // auto sat_first = sats.begin()->second;
    // for (int i = 0; i < sat_first.ints.size(); i++)
    // {
    //     std::cout << sat_first.name << " " << sat_first.ints[i].start << " " << sat_first.ints[i].end << " " << sat_first.ints[i].duration << std::endl;
    // }

    return 0;
}
