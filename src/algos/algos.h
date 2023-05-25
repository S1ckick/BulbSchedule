#ifndef SCHEDULE_GREEDY
#define SCHEDULE_GREEDY

#include <structs.h>

namespace algos
{
    Schedule great_plan(const Satellites &sats);
    void greedy(Satellites &sats, Observatories &obs);
}

#endif // SCHEDULE_GREEDY