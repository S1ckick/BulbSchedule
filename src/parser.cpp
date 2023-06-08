#include "parser.h"

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

extern std::unordered_map<std::string, uint32_t> obs_to_int;

using namespace date;

Interval parse_interval(const std::string &line, const SatName &sat_name, const ObsName &obs_name = 0)
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
    if (obs_name != 0)
        new_state = State::TRANSMISSION;
    else
        new_state = State::RECORDING;

    Interval interval(tp_start, tp_end, sat_name, new_state, obs_name);

    return interval;
}

int parse_russia_to_satellites(std::string &location, Satellites &sats)
{
    std::cout << "Parsing satellites...\n";
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
        SatName cur_sat = 0;
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

                auto interval = parse_interval(line, sats.at(cur_sat).name);

                cnt++;
                
                sats.at(cur_sat).ints_in_area.insert(interval);
            }
            else
            {
                std::string pre = "Russia-To";
                if (strncmp(pre.data(), line.data(), pre.size()) == 0)
                {
                    char sat_name[30];

                    sscanf(line.data(), "Russia-To-%s\n", sat_name);

                    int start_number = -1;
                    for (int i = 0; i < strlen(sat_name); i++)
                        if (sat_name[i] == '_') {
                            start_number = i + 1;
                            break;
                        }
                    if (start_number == -1)
                        throw "darou";

                    cur_sat = std::stoi(&sat_name[start_number]);
                    if (!sats.count(cur_sat)) {
                        auto new_sat = Satellite(cur_sat);
                        sats.insert(std::make_pair(cur_sat, new_sat));
                    }

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
        for (auto &sat: sats) {
            std::cout << std::to_string(sat.second.name) + ":" << sat.second.ints_in_area.size() << "\t";
        }
        std::cout << "\n";
    }

    return 0;
}

int parse_observatory(std::string &location, Satellites &sats)
{
    int int_idx = 0;
    SatName cur_sat_name;

    // TODO: call OS (in)dependent function to get files list

    std::cout << "Parsing stations...\n";
    // Read each observatory
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
        ObsName cur_obs;
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

                auto interval = parse_interval(line, cur_sat_name, cur_obs);

                auto inserted = sats.at(cur_sat_name).ints_observatories.insert(interval);
                Interval &int_inserted = const_cast<Interval&>(*inserted.first);
            }
            else
            {
                std::string start_of_line = line.substr(0, line.find('-'));
                std::string prefix = start_of_line + "-To-";
                if (std::strncmp(prefix.data(), line.data(), prefix.size()) == 0)
                {
                    cur_obs = obs_to_int[start_of_line];
                    size_t prefix_size = prefix.size();
                    int start_number = -1;
                    for (int i = 0; i < line.size(); i++)
                        if (line[i] == '_') {
                            start_number = i + 1;
                            break;
                        }

                    cur_sat_name = std::stoi(&line[start_number]);

                    headerRead = true;
                    // pass header lines
                    std::getline(fp, line);
                    std::getline(fp, line);
                    std::getline(fp, line);
                }
            }
        }

        fp.close();

        std::cout << "Read " << cur_obs  << "\n";
    }

    return 0;
}

int parse_schedule(VecSchedule &schedule, const std::string &filename, const timepoint &tp_start) {
    std::ifstream fp(filename);
    if (!fp)
    {
        printf("Error with opening file!\n");
        return 1;
    }

    std::string line;
    std::unordered_map<std::string,State> const str_to_state = { {"TRANSMISSION", State::TRANSMISSION}, {"IDLE", State::IDLE}, {"RECORDING", State::RECORDING}};
    
    while ((std::getline(fp, line))) {
        std::istringstream line_stream(line);
        std::string sat_num, sat_name, start_str, end_str, state_str, capacity_change,
                    obs_hex, obs_name;
        line_stream >> sat_num >> sat_name >> start_str >> end_str >> state_str >> capacity_change >>
                    obs_hex;
        if(obs_hex != "0") {
            line_stream >> obs_name;
        }
        else {
            obs_name = "0";
        }
            
        long int start_int = std::stol(start_str);
        long int end_int = std::stol(end_str);
        const timepoint start = tp_start + std::chrono::milliseconds(start_int);
        const timepoint end = tp_start + std::chrono::milliseconds(end_int);
        Interval inter(start, end, std::stoi(sat_name), std::stoi(obs_name));
        inter.info.state = str_to_state.at(state_str);
        //inter.capacity_change = std::stod(capacity_change);

        schedule.push_back(inter);
    }

    fp.close();

    return 0;
}
