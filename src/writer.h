#ifndef WRITER
#define WRITER
#include <algos.h>
#include <parser.h>

#define COUT_SATNAME(sat_name) "11" << std::setfill('0') << std::setw(2) << (sat_name / 10 + (sat_name % 10 == 0 ? 0 : 1)) \
                    << std::setfill('0') << std::setw(2) << (sat_name % 10 == 0 ? 10 : sat_name % 10)

int writeResults(Satellites &sats, const std::string &path_ground, const std::string &path_camera, const std::string & path_drop, const std::unordered_map<std::string, uint32_t> &obs_to_int);
#endif