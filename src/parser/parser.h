#ifndef SCHEDULE_PARSER
#define SCHEDULE_PARSER

#include <unordered_map>

#include "../structs.h"

int parse_observatory(const char *location, std::unordered_map<std::string, Observatory> &obs);
int parse_russia_to_satellites(const char *location, std::unordered_map<SatName, Satellite> &sats);

#endif // SCHEDULE_PARSER