#ifndef VALIDATION
#define VALIDATION

#include <algos.h>
#include <parser.h>

int checkValidity(VecSchedule &schedule_to_check, std::string &res);
int checkZeroIntervals(VecSchedule &schedule_to_check, std::string &res);
int checkForIntervalsIntersection(VecSchedule &schedule_to_check, std::string &res);
double countObsTotalLength(Satellites &sats);
int checkRecordingInRightArea(VecSchedule &schedule_to_check, Satellites &sats, std::string &res);
int checkBroadcastInRightArea(VecSchedule &schedule_to_check, Observatories &obs, std::string &res);
#endif