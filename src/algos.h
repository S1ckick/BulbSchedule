#ifndef SCHEDULE_GREEDY
#define SCHEDULE_GREEDY

#include <structs.h>

extern Observatory empty_obs;

namespace algos
{
    Schedule great_plan(const Satellites &sats);
    void add2schedule(std::shared_ptr<Interval> &interval, Satellite &cur_sat, Observatory &cur_obs = empty_obs);

    void greedy_random(Satellites &sats, Observatories &obs);
    void greedy_capacity(Satellites &sats, Observatories &obs);
    void greedy_exhaustive(Satellites &sats, Observatories &obs);
    
    void bysolver(Satellites &sats, Observatories &obs);
    void bysolver2(Satellites &sats, Observatories &obs);
}

#endif // SCHEDULE_GREEDY