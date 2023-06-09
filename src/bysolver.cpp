#include "algos.h"

#include "ortools/base/logging.h"
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model.pb.h"
#include "ortools/sat/cp_model_solver.h"
#include "ortools/util/sorted_interval_list.h"

extern timepoint START_MODELLING;

using namespace operations_research;
using namespace operations_research::sat;

#define CONTINUITY

void algos::bysolver(Satellites &sats)
{
    std::cout << "Starting scheduler\n";
    auto plan = great_plan(sats);

    int cnt = 0;

    double transmitted = 0;

    struct VarWithID
    {
        SatName sat_name;
        ObsName obs_name;
        BoolVar var;
        IntervalInfo *info;
    };
    
    bool can_record[SAT_NUM + 1];
    //LinearExpr underflow_conditions[SAT_NUM + 1];
    LinearExpr uniqueness_conditions_sat[SAT_NUM + 1];
    bool    sat_keep_station[SAT_NUM + 1];
    BoolVar sat_keep_station_var[SAT_NUM + 1];
    BoolVar sat_rec[SAT_NUM + 1];
    
#ifdef CONTINUITY        
    int station_receiving[OBS_NUM + 1]; // -1 if idle, i>= 0 if receiving from satellite #i
    for (int i = 0; i <= OBS_NUM; i++)
    {
        station_receiving[i] = -1;
    }
#endif
    

    std::vector<VarWithID> vars;
    
    for (auto &inter : plan)
    {
        vars.clear();
        
        cnt++;

        auto inter_dur = DURATION(inter.start, inter.end);
        
        //printf("[dur = %lf]", inter_dur);

        auto infos = inter.info;

        CpModelBuilder cp_model;

        LinearExpr optimized;
        LinearExpr uniqueness_conditions_obs[OBS_NUM + 1];

        for (int isat = 1; isat <= SAT_NUM; isat++)
        {
            can_record[isat] = false;
            sat_keep_station[isat] = false;
            //underflow_conditions[isat] = LinearExpr();
            //underflow_conditions[isat] -= (int)(1000 * sats[isat].capacity);
            uniqueness_conditions_sat[isat] = LinearExpr();
        }

#ifdef CONTINUITY        
        bool still_visible[OBS_NUM + 1]; 
        for (int i = 0; i <= OBS_NUM; i++)
            still_visible[i] = false;
#endif
        
        for (auto &info : infos)
        {
            if (info.state == State::RECORDING)
            {
                BoolVar v = cp_model.NewBoolVar();
                Satellite &sat = sats.at(info.sat_name);
                vars.push_back({info.sat_name, 0, v, &info});
                //underflow_conditions[info.sat_name] -= v * (int)(1000 * inter_dur * sat.recording_speed);
                can_record[info.sat_name] = true;
                sat_rec[info.sat_name] = v;
                
                optimized += v * (int)(20000 * inter_dur * sat.recording_speed *
                                       (sat.max_capacity * 0.8 - sat.capacity) / sat.max_capacity);
            }
            else if (info.state == State::TRANSMISSION)
            {
                double capacity = sats.at(info.sat_name).capacity;
                
                if (capacity > 1e-3) // not zero because a minuscule amount could have remained because of rounding error
                {
                    BoolVar v = cp_model.NewBoolVar();
                    double rate = sats.at(info.sat_name).broadcasting_speed;

                    vars.push_back({info.sat_name, info.obs_name, v, &info});
                    optimized += v * (int)(1000 * std::min(inter_dur * rate, capacity));

                    //underflow_conditions[info.sat_name] += v * (int)(1000 * );
    #ifdef CONTINUITY                
                    if (station_receiving[info.obs_name] == info.sat_name)
                    {
                        uniqueness_conditions_obs[info.obs_name] += 1;
                        uniqueness_conditions_sat[info.sat_name] += 1;
                        still_visible[info.obs_name] = true; 
                        sat_keep_station_var[info.sat_name] = v;
                        sat_keep_station[info.sat_name] = true;
                    }
                    else
                    {
                        uniqueness_conditions_obs[info.obs_name] += v;
                        uniqueness_conditions_sat[info.sat_name] += v;
                    }
    #else
                    uniqueness_conditions_obs[info.obs_name] += v;
                    uniqueness_conditions_sat[info.sat_name] += v;
    #endif
                }
            }
        }
        
#ifdef CONTINUITY        
        for (int i = 1; i <= OBS_NUM; i++)
            if (!still_visible[i])
            {
                station_receiving[i] = -1;
            }
#endif        
        
        int nconstraints = 0;
        for (int isat = 1; isat <= SAT_NUM; isat++)
        {
            if (can_record[isat])
            {
#ifdef CONTINUITY               
                if (sat_keep_station[isat])
                {
                    cp_model.AddEquality(uniqueness_conditions_sat[isat], 1);
                    LinearExpr e;
                    e += sat_keep_station_var[isat];
                    e += sat_rec[isat];
                    cp_model.AddEquality(e, 1);
                }
                else
                {
                    uniqueness_conditions_sat[isat] += sat_rec[isat];
                    cp_model.AddEquality(uniqueness_conditions_sat[isat], 1);
                }
#else
                uniqueness_conditions_sat[sat.second.name] += sat_rec[sat.second.name];
                cp_model.AddEquality(uniqueness_conditions_sat[sat.second.name], 1);
#endif                
            }
            else
            {
                cp_model.AddLessOrEqual(uniqueness_conditions_sat[isat], 1);
            }
            nconstraints++;
            //cp_model.AddLessOrEqual(underflow_conditions[isat], 0);
            //nconstraints++;
        }

        for (int obs = OBS_FIRST; obs <= OBS_NUM; obs++)
        {
            cp_model.AddLessOrEqual(uniqueness_conditions_obs[obs], 1);
            nconstraints++;
        }

        cp_model.Maximize(optimized);
        
        const CpSolverResponse response = Solve(cp_model.Build());

        if (response.status() == CpSolverStatus::OPTIMAL ||
            response.status() == CpSolverStatus::FEASIBLE)
        {
            int r = 0, b = 0;
            for (const auto &v : vars)
                if (SolutionBooleanValue(response, v.var))
                {
                    if (v.obs_name == 0) // recording
                    {
                        algos::add2schedule(inter.start, inter.end, *(v.info), sats.at(v.sat_name));
                        //if (v.sat_name == 157 && v.obs_name == 0)
                        //    printf("#");
                        r++;
                    }
                    else
                    {
                        Satellite &sat = sats[v.sat_name];
                        
                        if (sat.capacity >= inter_dur * sat.broadcasting_speed)
                            algos::add2schedule(inter.start, inter.end, *(v.info), sat);
                        else
                        {
                            auto transmit_dur = (sat.capacity / sat.broadcasting_speed);
                            
                            auto transmit_end = inter.start + std::chrono::nanoseconds(uint64_t(transmit_dur * 1e9));
                            
                            //std::cout << "inter_dur = " << inter_dur << ", transmit_dur = " << transmit_dur <<
                            //            ", start = " << date::format("%e %b %Y %T", inter.start) << ", tend = " << date::format("%e %b %Y %T", transmit_end) <<
                            //        ", end = " << date::format("%e %b %Y %T", inter.end) << "\n";
                            //printf("BEFORE TRANSMISSION %lf\n", sat.capacity);
                            
                            algos::add2schedule(inter.start, transmit_end, *(v.info), sat);
                            
                            // Record in the remaining time, if possible
                            if (can_record[v.sat_name])
                                algos::add2schedule(transmit_end, inter.end, IntervalInfo(v.sat_name, State::RECORDING), sat);
                            
                            //printf("AFTER TRANSMISSION %lf\n", sat.capacity);
                            //algos::add2schedule(inter.start, inter.end, *(v.info), sat);
                        }
                        
                        //if (v.sat_name == 157 && v.obs_name == 8)                        
                        //    printf("*");
                        transmitted += inter_dur * sats.at(v.sat_name).broadcasting_speed;
                        b++;
                        
#ifdef CONTINUITY                        
                        if (station_receiving[v.obs_name] != -1 && station_receiving[v.obs_name] != v.sat_name)
                        {
                            printf("?");
                        }
                        if (station_receiving[v.obs_name] == v.sat_name)
                        {
                            //printf("!");
                        }
                        station_receiving[v.obs_name] = v.sat_name;
#endif
                    }
                }
            if ((cnt % 10) == 0 || cnt == plan.size())
            {
                printf("Interval %6d/%ld : ", cnt, plan.size());
                printf("%2d recording, %2d transmitting; total transmitted %lf\n", r, b, transmitted);
            }
        }
        else
        {
            LOG(INFO) << "No solution found.";
        }
    }
}