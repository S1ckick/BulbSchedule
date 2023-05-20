#include<string>
#include<iostream>
#include<fstream>
#include<cstring>
#include<vector>
#include "date.h"
#include "structs.h"
#include "parser.h"

using namespace date;

int main(){
    std::vector<Satellite> sats(200);
    const char russia_location[] = "DATA_Files/Russia2Constellation/"; 
    int res_parse_russia = parse_russia_to_satellites(russia_location, sats);


    std::vector<Observatory> obs(14);
    const char facility_location[] = "DATA_Files/Facility2Constellation/";
    parse_observatory(facility_location, obs);
    
    for(int i = 0; i < sats[0].ints.size(); i++){
            std::cout << 
            sats[0].ints[i].sat_name << " " <<
            sats[0].ints[i].sat_type << " " << 
            sats[0].ints[i].start << " " << 
            sats[0].ints[i].end << " " << 
            sats[0].ints[i].duration << std::endl;
    }

    return 0;
}