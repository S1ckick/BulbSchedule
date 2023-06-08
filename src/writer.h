#ifndef WRITER
#define WRITER
#include <algos.h>
#include <parser.h>

int write_res_obs(const Satellites &sats, const std::string &path, const std::unordered_map<std::string, uint32_t> &obs_to_int);
int write_res_sats(const Satellites &sats, const std::string &path);
#endif