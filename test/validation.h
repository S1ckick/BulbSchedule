#ifndef VALIDATION
#define VALIDATION

#include <algos.h>
#include <parser/parser.h>

extern std::unordered_map<std::string, int> obs_to_int;
extern std::unordered_map<std::string, std::string> obs_to_hex;

std::ostream& operator << (std::ostream& os, const State& obj);

int checkValidity(Schedule &schedule_to_check, std::string &res);
int checkZeroIntervals(Schedule &schedule_to_check, std::string &res);
double countObsTotalLength(Satellites &sats);

#endif