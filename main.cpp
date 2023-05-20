#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include "date.h"
#include "structs.h"
#include "parser.h"

using namespace date;

int main()
{
    std::unordered_map<SatName, Satellite> sats;
    const char russia_location[] = "DATA_Files/Russia2Constellation/";
    int res_parse_russia = parse_russia_to_satellites(russia_location, sats);

    std::unordered_map<std::string, Observatory> obs;
    const char facility_location[] = "DATA_Files/Facility2Constellation/";
    parse_observatory(facility_location, obs);
    
    // auto sat_first = sats.begin()->second;
    // for (int i = 0; i < sat_first.ints.size(); i++)
    // {
    //     std::cout << sat_first.name << " " << sat_first.ints[i].start << " " << sat_first.ints[i].end << " " << sat_first.ints[i].duration << std::endl;
    // }

    return 0;
}