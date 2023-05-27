#ifndef VALIDATION
#define VALIDATION

#include <algos.h>
#include <parser/parser.h>

extern std::unordered_map<std::string, int> obs_to_int;
extern std::unordered_map<std::string, std::string> obs_to_hex;

int checkValidity(VecSchedule &schedule_to_check, std::string &res);
int checkZeroIntervals(VecSchedule &schedule_to_check, std::string &res);
int checkForIntervalsIntersection(VecSchedule &schedule_to_check, std::string &res);
double countObsTotalLength(Satellites &sats);
int checkRecordingInRightArea(VecSchedule &schedule_to_check, Satellites &sats, std::string &res);
int checkBroadcastInRightArea(VecSchedule &schedule_to_check, Observatories &obs, std::string &res);
#endif