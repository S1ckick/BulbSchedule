#ifndef SCHEDULE_STRUCTS
#define SCHEDULE_STRUCTS

#include "date.h"

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


#endif 