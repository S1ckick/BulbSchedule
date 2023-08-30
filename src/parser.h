#ifndef SCHEDULE_PARSER
#define SCHEDULE_PARSER

#include <structs.h>

int parse_station(std::string &location, Satellites &sats);
int parse_russia_to_satellites(std::string &location, Satellites &sats);

int parseCamera(VecSchedule & schedule, const std::string &resdir);
int parseDrop(VecSchedule & schedule, const std::string &resdir);
int parse_schedule(VecSchedule &schedule, const std::string &filename);


#endif // SCHEDULE_PARSER