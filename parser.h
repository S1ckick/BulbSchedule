#include "date.h"
#include "structs.h"
#include<string>
#include<iostream>
#include<fstream>
#include<cstring>
#include<vector>

using namespace date;

int parse_russia_to_satellites(const char *location, std::vector<Satellite> &sats){
    int sats_idx = 0;
    // Read Russia intervals for each satellite
    for(int i = 1; i <= 20; i++){

        char filename[90];
        sprintf(filename,"%sAreaTarget-Russia-To-Satellite-KinoSat_%02d_plane.txt", location, i);
        printf("%s\n", filename);

        FILE * fp = fopen(filename, "r");
        if(fp == NULL) {
            printf("Error with opening file!\n");
            return 1;
        }

        size_t len = 0;
        char *line = NULL;

        int curRead = 0;

        while((getline(&line, &len, fp)) != -1){
            if(curRead == 1) {
                if(line[0] != ' ') {
                    curRead = 0;
                    sats_idx++;
                    continue;
                }

                std::string str_idx,str_start_day,str_start_month,str_start_year,str_start_time,
                                str_end_day,str_end_month,str_end_year,str_end_time,
                            str_duration;
                std::istringstream some_stream(line);
                some_stream >> str_idx >> str_start_day >> str_start_month >> str_start_year >>
                            str_start_time >> str_end_day >> str_end_month >> str_end_year >>
                            str_end_time >> str_duration;

                std::istringstream start_date(str_start_day + "/" + str_start_month + "/" + str_start_year + " " + str_start_time);
                std::chrono::system_clock::time_point tp_start;
                start_date >> date::parse("%d/%b/%Y %T", tp_start);

                std::istringstream end_date(str_end_day + "/" + str_end_month + "/" + str_end_year + " " + str_end_time);
                std::chrono::system_clock::time_point tp_end;
                end_date >> date::parse("%d/%b/%Y %T", tp_end);              

                double duration = (std::chrono::duration<double, std::milli>(tp_end - tp_start)* std::chrono::milliseconds::period::num /
                               std::chrono::milliseconds::period::den).count();

                Interval interval;
                interval.duration = duration;
                interval.start = tp_start;
                interval.end = tp_end;
                interval.sat_type = sats[sats_idx].type; 
                strncpy(interval.sat_name, sats[sats_idx].name, strlen(sats[sats_idx].name));
                sats[sats_idx].ints.push_back(interval);
            } else {
                const char *pre = "Russia-To";
                if(strncmp(pre, line, strlen(pre)) == 0) {
                    char sat_name[20];
                    sscanf(line, "Russia-To-%s\n", sat_name);
                    strncpy(sats[sats_idx].name, sat_name, strlen(sat_name));
                    if(strncmp("KinoSat", sat_name, strlen("KinoSat")) == 0)
                        sats[sats_idx].type = KINOSAT;
                    else 
                        sats[sats_idx].type = ZORKIY;
                    curRead = 1;
                    // pass header lines
                    getline(&line, &len, fp);
                    getline(&line, &len, fp);
                    getline(&line, &len, fp);
                }
            }
        }
    }
    return 0;
}


int parse_observatory(const char *location, std::vector<Observatory> &obs){
    int obs_idx = 0;
    char cur_sat_name[20];
    int cur_sat_type = 0;

    // TODO: call OS (in)dependent function to get files list

    const char obs_names[14][15] = {"Anadyr1", "Anadyr2", "Cape_Town",
                       "Dehli", "Irkutsk", "Magadan1",
                       "Magadan2", "Moscow", "Murmansk1",
                       "Murmansk2", "Norilsk", "Novosib",
                       "RioGallegos", "Sumatra"};

    int obs_num = sizeof(obs_names) / sizeof(obs_names[0]);
    

    // Read each observatory
    for(int i = 0; i < obs_num; i++){
        char filename[70];
        sprintf(filename,"%sFacility-%s.txt", location, obs_names[i]);
        printf("%s\n", filename);
        FILE * fp = fopen(filename, "r");
        if(fp == NULL) {
            printf("Error with opening file!\n");
            return 1;
        }

        size_t len = 0;
        char *line = NULL;

        int curRead = 0;

        while((getline(&line, &len, fp)) != -1){
            if(curRead == 1) {
                if(line[0] != ' ') {
                    curRead = 0;
                    obs_idx++;
                    continue;
                }
                
                std::string str_idx,str_start_day,str_start_month,str_start_year,str_start_time,
                                str_end_day,str_end_month,str_end_year,str_end_time,
                            str_duration;
                std::istringstream some_stream(line);
                some_stream >> str_idx >> str_start_day >> str_start_month >> str_start_year >>
                            str_start_time >> str_end_day >> str_end_month >> str_end_year >>
                            str_end_time >> str_duration;

                std::istringstream start_date(str_start_day + "/" + str_start_month + "/" + str_start_year + " " + str_start_time);
                std::chrono::system_clock::time_point tp_start;
                start_date >> date::parse("%d/%b/%Y %T", tp_start);

                std::istringstream end_date(str_end_day + "/" + str_end_month + "/" + str_end_year + " " + str_end_time);
                std::chrono::system_clock::time_point tp_end;
                end_date >> date::parse("%d/%b/%Y %T", tp_end);              

                double duration = (std::chrono::duration<double, std::milli>(tp_end - tp_start)* std::chrono::milliseconds::period::num /
                               std::chrono::milliseconds::period::den).count();
                
                // Interval interval;
                // interval.duration = duration;
                // interval.start = tp_start;
                // interval.end = tp_end;
                // interval.sat_type = cur_sat_type;
                // strncpy(interval.sat_name, cur_sat_name, strlen(cur_sat_name));
                // printf("%ld\n", obs_idx);
                // obs[obs_idx].ints.push_back(interval);
            } else {
                char pre[20];
                sprintf(pre,"%s-To-", obs_names[i]);
                printf("%s\n", pre);
                if(strncmp(pre, line, strlen(pre)) == 0) {
                    char sat_name[20];
                    char obs_name[20];
                    sscanf(line, "%s-To-%s\n", obs_name, sat_name);
                    strncpy(obs[obs_idx].name, obs_name, strlen(obs_name));
                    strncpy(cur_sat_name, sat_name, strlen(sat_name));

                    if(strncmp("KinoSat", sat_name, strlen("KinoSat")) == 0)
                        cur_sat_type = KINOSAT;
                    else 
                        cur_sat_type = ZORKIY;
                    curRead = 1;
                    // pass header lines
                    getline(&line, &len, fp);
                    getline(&line, &len, fp);
                    getline(&line, &len, fp);
                }
            }
        }
    }
    return 0;
}