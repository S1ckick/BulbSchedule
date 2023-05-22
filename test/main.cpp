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
    const char russia_location[] = "../../DATA_Files/Russia2Constellation/";
    int res_parse_russia = parse_russia_to_satellites(russia_location, sats);

    std::unordered_map<std::string, Observatory> obs;
    const char facility_location[] = "../../DATA_Files/Facility2Constellation/";
    int res_parse_obs = parse_observatory(facility_location, obs, sats);
    algos::build_schedule(sats);
    
    std::ofstream out("sats.txt", std::ofstream::out);
    std::ofstream out_schedule("sats_schedule.txt", std::ofstream::out);

    std::istringstream start_date("1/Jun/2027 00:00:00.000");
    std::chrono::system_clock::time_point tp_start;
    start_date >> date::parse("%d/%b/%Y %T", tp_start);

    for (auto &item : sats){
        for (auto &interval : item.second.ints_in_area){
            out << std::fixed << interval->sat_name << " "
                << (std::chrono::duration<double, std::milli>(interval->start - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << " " << (std::chrono::duration<double, std::milli>(interval->end - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << '\n';
        }

        for (auto &interval : item.second.full_schedule){
            out_schedule << std::fixed << interval->sat_name << " "
                << (std::chrono::duration<double, std::milli>(interval->start - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << " " << (std::chrono::duration<double, std::milli>(interval->end - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den).count()
                << " " << interval->state
                << " " << interval->capacity_change
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