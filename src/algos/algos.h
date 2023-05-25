#ifndef SCHEDULE_GREEDY
#define SCHEDULE_GREEDY

#include <structs.h>

namespace algos
{
    Schedule great_plan(const Satellites &sats);
    void greedy_random(Satellites &sats, Observatories &obs);
    void greedy_capacity(Satellites &sats, Observatories &obs);
}

#endif // SCHEDULE_GREEDY