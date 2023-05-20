#include<string>
#include<iostream>
#include<fstream>
#include<cstring>
#include<vector>
#include "date.h"


using namespace date;
#define KINOSAT 0
#define ZORKIY 1


struct Interval
{
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    double duration;
};


struct Satellite
{
    char * name;
    int type;
    std::vector<Interval> ints;
};



int main(){
    std::vector<Satellite> sats(200);

    int sats_idx = 0;

    // Read Russia intervals for each satellite
    for(int i = 1; i <= 20; i++){

        char filename[90];
        sprintf(filename,"DATA_Files/Russia2Constellation/AreaTarget-Russia-To-Satellite-KinoSat_%02d_plane.txt", i);
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
                printf("%s", line);
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
                sats[sats_idx].ints.push_back(interval);
            } else {
                const char *pre = "Russia-To";
                if(strncmp(pre, line, strlen(pre)) == 0) {
                    char sat_name[14];
                    sscanf(line, "Russia-To-%s\n", sat_name);
                    sats[sats_idx].name = sat_name;
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


        for(int i = 0; i < sats[0].ints.size(); i++){
            std::cout << sats[0].ints[i].start << " " << sats[0].ints[i].end << " " << sats[0].ints[i].duration << std::endl;
        }


    }

    return 0;
}