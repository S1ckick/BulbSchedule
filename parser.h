#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <unordered_map>

#include "date.h"
#include "structs.h"

using namespace date;

Interval parse_interval(const std::string &line, const SatType &sat_type, const SatName sat_name)
{
    std::string str_idx, str_start_day, str_start_month, str_start_year, str_start_time,
        str_end_day, str_end_month, str_end_year, str_end_time,
        str_duration;
    std::istringstream line_stream(line);
    line_stream >> str_idx >> str_start_day >> str_start_month >> str_start_year >>
        str_start_time >> str_end_day >> str_end_month >> str_end_year >>
        str_end_time >> str_duration;

    std::istringstream start_date(str_start_day + "/" + str_start_month + "/" + str_start_year + " " + str_start_time);
    std::chrono::system_clock::time_point tp_start;
    start_date >> date::parse("%d/%b/%Y %T", tp_start);

    std::istringstream end_date(str_end_day + "/" + str_end_month + "/" + str_end_year + " " + str_end_time);
    std::chrono::system_clock::time_point tp_end;
    end_date >> date::parse("%d/%b/%Y %T", tp_end);

    double duration = (std::chrono::duration<double, std::milli>(tp_end - tp_start) * std::chrono::milliseconds::period::num /
                       std::chrono::milliseconds::period::den)
                          .count();

    Interval interval;
    interval.duration = duration;
    interval.start = tp_start;
    interval.end = tp_end;
    interval.sat_type = sat_type;
    interval.sat_name = sat_name;

    return interval;
}

int parse_russia_to_satellites(const char *location, std::unordered_map<SatName, Satellite> &sats)
{
    std::cout << "\nStart parsing satellites\n";
    // Read Russia intervals for each satellite
    for (int plane_num = 1; plane_num <= PLANES_NUM; plane_num++)
    {
        char filename[90];
        sprintf(filename, "%sAreaTarget-Russia-To-Satellite-KinoSat_%02d_plane.txt", location, plane_num);
        std::cout << "Reading file " + std::string(filename) + "\n";

        FILE *fp = fopen(filename, "r");
        if (fp == NULL)
        {
            printf("Error with opening file!\n");
            return 1;
        }

        size_t len = 0;
        char *line = NULL;

        bool headerReaded = false;
        SatName cur_sat = 0;

        while ((getline(&line, &len, fp)) != -1)
        {
            if (headerReaded)
            {
                if (line[0] != ' ')
                {
                    headerReaded = false;
                    //sats_idx++;
                    continue;
                }

                auto interval = parse_interval(line, sats[cur_sat].type, sats[cur_sat].name);

                sats[cur_sat].ints.push_back(interval);
            }
            else
            {
                const char *pre = "Russia-To";
                if (strncmp(pre, line, strlen(pre)) == 0)
                {
                    char sat_name[20];

                    sscanf(line, "Russia-To-%s\n", sat_name);

                    int start_number = -1;
                    for (int i = 0; i < strlen(sat_name); i++)
                        if (sat_name[i] == '_') {
                            start_number = i + 1;
                            break;
                        }

                    cur_sat = std::stoi(&sat_name[start_number]);
                    if (!sats.count(cur_sat)) {
                        sats[cur_sat].ints.reserve(300);
                        sats[cur_sat] = Satellite();
                        sats[cur_sat].name = cur_sat;

                        if (strncmp(KinosatName.data(), sat_name, KinosatName.size()) == 0)
                            sats[cur_sat].type = SatType::KINOSAT;
                        else
                            sats[cur_sat].type = SatType::ZORKIY;
                    }

                    headerReaded = true;
                    // pass header lines
                    getline(&line, &len, fp);
                    getline(&line, &len, fp);
                    getline(&line, &len, fp);
                }
            }
        }

        fclose(fp);

        std::cout << "Current state:\nSattelites " + std::to_string(sats.size()) + "\n";
        for (auto &sat: sats) {
            std::cout << SatNames.at(sat.second.type) + "_" + std::to_string(sat.second.name) + ":" << sat.second.ints.size() << "\t";
        }
        std::cout << "\n";
    }
    return 0;
}

int parse_observatory(const char *location, std::unordered_map<std::string, Observatory> &obs)
{
    int int_idx = 0;
    SatName cur_sat_name;
    SatType cur_sat_type;

    // TODO: call OS (in)dependent function to get files list

    std::vector<std::string> obs_names = {"Anadyr1", "Anadyr2", "CapeTown",
                                          "Delhi", "Irkutsk", "Magadan1",
                                          "Magadan2", "Moscow", "Murmansk1",
                                          "Murmansk2", "Norilsk", "Novosib",
                                          "RioGallegos", "Sumatra"};

    int obs_num = obs_names.size();
    std::cout << "\nStart parsing observatories\n";
    // Read each observatory
    for (int i = 0; i < obs_num; i++)
    {
        char filename[70];
        std::string cur_obs = obs_names[i].data();
        obs[cur_obs] = Observatory{cur_obs, {}};
        obs[cur_obs].ints.reserve(30000);

        sprintf(filename, "%sFacility-%s.txt", location, obs_names[i].data());

        std::cout << "Reading file " + std::string(filename) + "\n";
        
        FILE *fp = fopen(filename, "r");
        if (fp == NULL)
        {
            printf("Error with opening file!\n");
            return 1;
        }

        size_t len = 0;
        char *line = NULL;

        bool headerReaded = false;

        while ((getline(&line, &len, fp)) != -1)
        {
            if (headerReaded)
            {
                if (line[0] != ' ')
                {
                    headerReaded = false;
                    int_idx++;
                    continue;
                }

                auto interval = parse_interval(line, cur_sat_type, cur_sat_name);

                obs[cur_obs].ints.push_back(interval);
            }
            else
            {
                std::string prefix = obs_names[i] + "-To-";

                if (std::strncmp(prefix.data(), line, prefix.size()) == 0)
                {
                    int prefix_size = prefix.size();

                    int start_number = -1;
                    for (int i = 0; i < strlen(line); i++)
                        if (line[i] == '_') {
                            start_number = i + 1;
                            break;
                        }

                    int cur_sat = std::stoi(&line[start_number]);

                    if (strncmp(&line[prefix_size], KinosatName.data(), KinosatName.size()) == 0)
                        cur_sat_type = SatType::KINOSAT;
                    else
                        cur_sat_type = SatType::ZORKIY;

                    headerReaded = true;
                    // pass header lines
                    getline(&line, &len, fp);
                    getline(&line, &len, fp);
                    getline(&line, &len, fp);
                }
            }
        }

        fclose(fp);
        std::cout << "Readed " + cur_obs + " with " + std::to_string(obs[cur_obs].ints.size()) + " intervals\n";
    }

    return 0;
}