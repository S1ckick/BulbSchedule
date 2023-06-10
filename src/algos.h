#ifndef SCHEDULE_GREEDY
#define SCHEDULE_GREEDY

#include <structs.h>

namespace algos
{
    VecSegment great_plan(const Satellites &sats);
    void add2schedule(const timepoint &start, const timepoint &end, const IntervalInfo &info, Satellite &cur_sat);
    timepoint end_of_current_interval(const Satellite &sat, const StationID &station, const timepoint &cur_start);

    void greedy_capacity(Satellites &sats);
    
    void bysolver(Satellites &sats, double F, double W);
}

#endif // SCHEDULE_GREEDY