#ifndef WRITER
#define WRITER
#include <algos.h>
#include <parser.h>

int write_res_obs(Satellites &sats, std::string &path, std::unordered_map<std::string, int> &obs_to_int);
int write_res_sats(Satellites &sats, std::string &path);
#endif