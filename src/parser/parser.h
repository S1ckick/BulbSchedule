#ifndef SCHEDULE_PARSER
#define SCHEDULE_PARSER

#include <structs.h>

int parse_observatory(const char *location, Observatories &obs, Satellites &sats);
int parse_russia_to_satellites(const char *location, Satellites &sats);

int parse_schedule(Schedule &schedule, std::string & filename);

#endif // SCHEDULE_PARSER