#ifndef SCHEDULE_STRUCTS
#define SCHEDULE_STRUCTS

#include "date.h"

#define KINOSAT 0
#define ZORKIY 1

struct Interval
{
    char sat_name[20];
    int sat_type;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    double duration;
};


struct Satellite
{
    char name[20];
    int type;
    std::vector<Interval> ints;
};

struct Observatory
{
    char name[20];
    std::vector<Interval> ints;
};



#endif 