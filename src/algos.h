#ifndef SCHEDULE_GREEDY
#define SCHEDULE_GREEDY

#include <structs.h>

namespace algos
{
    VecSegment great_plan(const Satellites &sats);
    void add2schedule(const timepoint &start, const timepoint &end, const IntervalInfo &info, Satellite &cur_sat);

    void greedy_random(Satellites &sats);
    void greedy_capacity(Satellites &sats);
    void greedy_exhaustive(Satellites &sats);
    
    void bysolver(Satellites &sats);
    void bysolver2(Satellites &sats);
}

#endif // SCHEDULE_GREEDY