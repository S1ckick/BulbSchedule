#include "parser.h"

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

extern std::unordered_map<std::string, uint32_t> stn_to_int;

using namespace date;

Interval parse_interval(const std::string &line, const SatID &sat_id, const StationID &station_id = 0)
{
    std::string str_idx, str_start_day, str_start_month, str_start_year, str_start_time,
        str_end_day, str_end_month, str_end_year, str_end_time,
        str_duration;
    std::istringstream line_stream(line);
    line_stream >> str_idx >> str_start_day >> str_start_month >> str_start_year >>
        str_start_time >> str_end_day >> str_end_month >> str_end_year >>
        str_end_time >> str_duration;

    std::istringstream start_date(str_start_day + "/" + str_start_month + "/" + str_start_year + " " + str_start_time);
    timepoint tp_start;
    start_date >> date::parse("%d/%b/%Y %T", tp_start);

    std::istringstream end_date(str_end_day + "/" + str_end_month + "/" + str_end_year + " " + str_end_time);
    timepoint tp_end;
    end_date >> date::parse("%d/%b/%Y %T", tp_end);

    State new_state;
    if (station_id != 0)
        new_state = State::TRANSMISSION;
    else
        new_state = State::RECORDING;

    Interval interval(tp_start, tp_end, sat_id, new_state, station_id);

    return interval;
}

int parse_russia_to_satellites(std::string &location, Satellites &sats)
{
    std::cout << "Parsing satellites...\n";
    sats.resize(SAT_NUM + 1);
    
    for (int isat = 1; isat <= SAT_NUM; isat++)
        sats[isat].init(isat);
    
    for (const auto & entry : fs::directory_iterator(location))
    {
        std::string name = entry.path().filename().string();
        //check filename
        if(name.rfind("Russia-To", 0) != 0)
            continue;
        // Read Russia intervals for each satellite
        std::string filename = entry.path().string();
        std::cout << "Reading file " + filename + "\n";

        std::ifstream fp(filename);
        if (!fp)
        {
            printf("Error with opening file!\n");
            return 1;
        }

        size_t len = 0;
        std::string line;

        bool headerRead = false;
        SatID cur_sat = 0;
        int cnt = 0;

        while (std::getline(fp, line))
        {
            if (headerRead)
            {
                if (line[0] != ' ')
                {
                    headerRead = false;
                    //sats_idx++;
                    continue;
                }

                auto interval = parse_interval(line, cur_sat);

                cnt++;
                
                sats.at(cur_sat).ints_in_area.insert(interval);
            }
            else
            {
                std::string pre = "Russia-To";
                if (strncmp(pre.data(), line.data(), pre.size()) == 0)
                {
                    char sat_id[30];

                    sscanf(line.data(), "Russia-To-%s\n", sat_id);

                    std::string sat_name_str(sat_id);
                    int start_number = -1;
                    for (int i = 0; i < sat_name_str.length(); i++)
                        if (sat_name_str[i] == '_') {
                            start_number = i + 1;
                            break;
                        }
                    if (start_number == -1)
                        throw "parse russia to satellites error";

                    //cur_sat = std::stoi(&sat_name_str[start_number]);
                    cur_sat = 10 * (std::stoi(sat_name_str.substr(start_number+2,2)) - 1) + std::stoi(sat_name_str.substr(start_number+4,2));

                    headerRead = true;
                    // pass header lines
                    std::getline(fp, line);
                    std::getline(fp, line);
                    std::getline(fp, line);
                }
            }
        }

        fp.close();

        std::cout << "Current state:\nSattelites " + std::to_string(sats.size()) + "\n";
        for (int isat = 1; isat <= SAT_NUM; isat++) {
            std::cout << std::to_string(isat) + ":" << sats[isat].ints_in_area.size() << "\t";
        }
        std::cout << "\n";
    }

    return 0;
}

int parse_station(std::string &location, Satellites &sats)
{
    int int_idx = 0;
    SatID cur_sat_name;

    // TODO: call OS (in)dependent function to get files list

    std::cout << "Parsing stations...\n";
    // Read each station
    for (const auto & entry : fs::directory_iterator(location))
    {
        std::string name = entry.path().filename().string();
        //check filename
        if(name.rfind("Facility", 0) != 0)
            continue;
        // Read Russia intervals for each satellite
        std::string filename = entry.path().string();
        std::cout << "Reading file " + filename + "\n";
        
        std::ifstream fp(filename);
        if (!fp)
        {
            printf("Error with opening file!\n");
            return 1;
        }

        size_t len = 0;
        std::string line;

        bool headerRead = false;
        StationID cur_station;
        while ((std::getline(fp, line)))
        {
            if (headerRead)
            {
                if (line[0] != ' ')
                {
                    headerRead = false;
                    int_idx++;
                    continue;
                }

                auto interval = parse_interval(line, cur_sat_name, cur_station);

                auto inserted = sats.at(cur_sat_name).ints_stations.insert(interval);
                Interval &int_inserted = const_cast<Interval&>(*inserted.first);
            }
            else
            {
                std::string start_of_line = line.substr(0, line.find('-'));
                std::string prefix = start_of_line + "-To-";
                if (std::strncmp(prefix.data(), line.data(), prefix.size()) == 0)
                {
                    cur_station = stn_to_int[start_of_line];
                    size_t prefix_size = prefix.size();
                    int start_number = -1;
                    for (int i = 0; i < line.size(); i++)
                        if (line[i] == '_') {
                            start_number = i + 1;
                            break;
                        }

                    //cur_sat_name = std::stoi(&line[start_number]);
                    cur_sat_name = 10 * (std::stoi(line.substr(start_number+2,2)) - 1) + std::stoi(line.substr(start_number+4,2));

                    headerRead = true;
                    // pass header lines
                    std::getline(fp, line);
                    std::getline(fp, line);
                    std::getline(fp, line);
                }
            }
        }

        fp.close();

        std::cout << "Read " << cur_station  << "\n";
    }

    return 0;
}

int parse_schedule(VecSchedule &schedule, const std::string &resdir) {

    parseCamera(schedule, resdir);
    parseDrop(schedule, resdir);

    return 0;
}


Interval parse_intervalDrop(const std::string &line, const SatID &sat_id)
{
    std::string str_idx, str_start_day, str_start_month, str_start_year, str_start_time,
        str_end_day, str_end_month, str_end_year, str_end_time,
        str_duration, station_id;
    std::istringstream line_stream(line);
    line_stream >> str_idx >> str_start_day >> str_start_month >> str_start_year >>
        str_start_time >> str_end_day >> str_end_month >> str_end_year >>
        str_end_time >> str_duration >> station_id;

    std::istringstream start_date(str_start_day + "/" + str_start_month + "/" + str_start_year + " " + str_start_time);
    timepoint tp_start;
    start_date >> date::parse("%d/%b/%Y %T", tp_start);

    std::istringstream end_date(str_end_day + "/" + str_end_month + "/" + str_end_year + " " + str_end_time);
    timepoint tp_end;
    end_date >> date::parse("%d/%b/%Y %T", tp_end);

    State new_state;
    new_state = State::TRANSMISSION;

    std::cout << station_id << std::endl;

    Interval interval(tp_start, tp_end, sat_id, new_state, stn_to_int[station_id]);

    return interval;
}

int parseCamera(VecSchedule & schedule, const std::string &resdir) {
        std::string cameradir = resdir + "Camera";
        std::set<fs::path> sorted_dir;
    for (const auto & file: std::filesystem::directory_iterator(cameradir)) {
        sorted_dir.insert(file.path());
    }
    SatID sat_id = 0;
    for (auto &filename: sorted_dir){
        sat_id++;
        std::ifstream fp(filename.u8string());
        std::cout << filename.u8string() << std::endl;
        if (!fp)
        {
            printf("Error with opening file!\n");
            return 1;
        }

        std::string line;
        std::getline(fp, line);
        std::getline(fp, line);
        std::getline(fp, line);

        while ((std::getline(fp, line))) {
            std::istringstream line_stream(line);
            auto interval = parse_interval(line, sat_id);
            schedule.push_back(interval);
        }

        fp.close();
    }
    return 0;
}

int parseDrop(VecSchedule & schedule, const std::string &resdir) {
        std::string cameradir = resdir + "Drop";
        std::set<fs::path> sorted_dir;
    for (const auto & file: std::filesystem::directory_iterator(cameradir)) {
        sorted_dir.insert(file.path());
    }
    SatID sat_id = 0;
    StationID cur_station;
    int counter = 0;
    for (auto &filename: sorted_dir){
        sat_id++;
        std::ifstream fp(filename.u8string());
        std::cout << filename.u8string() << std::endl;

        if (!fp)
        {
            printf("Error with opening file!\n");
            return 1;
        }

        std::string line;
        std::getline(fp, line);
        std::getline(fp, line);
        std::getline(fp, line);

        while (std::getline(fp, line)) {
            std::istringstream line_stream(line);
            auto interval = parse_intervalDrop(line, sat_id);
            schedule.push_back(interval);
            counter++;
        }
        
        fp.close();
    }
    std::cout << "For drop: " << counter << std::endl;
    return 0;
}