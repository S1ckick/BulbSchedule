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
    
    std::map<SatName, bool> can_record;
    
#ifdef CONTINUITY        
    int station_receiving[OBS_NUM + 1]; // -1 if idle, i>= 0 if receiving from satellite #i
    for (int i = 0; i <= OBS_NUM; i++)
    {
        station_receiving[i] = -1;
    }
#endif
    

    for (auto &inter : plan)
    {
        cnt++;

        auto inter_dur = DURATION(inter.start, inter.end);

        auto infos = inter.info;

        CpModelBuilder cp_model;

        std::vector<VarWithID> vars;

        LinearExpr optimized;
        std::map<SatName, LinearExpr> underflow_conditions;
        std::map<SatName, LinearExpr> uniqueness_conditions_sat;
        std::map<SatName, BoolVar> sat_keep_station;
        std::map<SatName, BoolVar> sat_rec;
        
        LinearExpr uniqueness_conditions_obs[OBS_NUM + 1];

        for (auto &sat : sats)
        {
            SatName name = sat.second.name;
            underflow_conditions[name] = LinearExpr();
            underflow_conditions[name] -= (int)(1000 * sats.at(sat.second.name).capacity);
            uniqueness_conditions_sat[name] = LinearExpr();
            can_record[name] = false;
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
                underflow_conditions[info.sat_name] -= v * (int)(1000 * inter_dur * sat.recording_speed);
                can_record[info.sat_name] = true;
                sat_rec[info.sat_name] = v;
                
                optimized += v * (int)(20000 * inter_dur * sat.recording_speed *
                                       (sat.max_capacity * 0.8 - sat.capacity) / sat.max_capacity);
            }
            else if (info.state == State::TRANSMISSION)
            {
                BoolVar v = cp_model.NewBoolVar();
                double rate = sats.at(info.sat_name).broadcasting_speed;

                vars.push_back({info.sat_name, info.obs_name, v, &info});
                optimized += v * (int)(1000 * inter_dur * rate);
                
                underflow_conditions[info.sat_name] += v * (int)(1000 * inter_dur * rate);
#ifdef CONTINUITY                
                if (station_receiving[info.obs_name] == info.sat_name)
                {
                    uniqueness_conditions_obs[info.obs_name] += 1;
                    uniqueness_conditions_sat[info.sat_name] += 1;
                    still_visible[info.obs_name] = true; 
                    sat_keep_station[info.sat_name] = v;
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
        
#ifdef CONTINUITY        
        for (int i = 1; i <= OBS_NUM; i++)
            if (!still_visible[i])
            {
                station_receiving[i] = -1;
            }
#endif        
        
        int nconstraints = 0;
        for (auto &sat : sats)
        {
            if (can_record[sat.second.name])
            {
#ifdef CONTINUITY               
                if (sat_keep_station.find(sat.second.name) != sat_keep_station.end())
                {
                    cp_model.AddEquality(uniqueness_conditions_sat[sat.second.name], 1);
                    LinearExpr e;
                    e += sat_keep_station[sat.second.name];
                    e += sat_rec[sat.second.name];
                    cp_model.AddEquality(e, 1);
                }
                else
                {
                    uniqueness_conditions_sat[sat.second.name] += sat_rec[sat.second.name];
                    cp_model.AddEquality(uniqueness_conditions_sat[sat.second.name], 1);
                }
#else
                uniqueness_conditions_sat[sat.second.name] += sat_rec[sat.second.name];
                cp_model.AddEquality(uniqueness_conditions_sat[sat.second.name], 1);
#endif                
            }
            else
            {
                cp_model.AddLessOrEqual(uniqueness_conditions_sat[sat.second.name], 1);
            }
            nconstraints++;
            cp_model.AddLessOrEqual(underflow_conditions[sat.second.name], 0);
            nconstraints++;
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
                    algos::add2schedule(inter.start, inter.end, *(v.info), sats.at(v.sat_name));
                    if (v.obs_name == 0) // recording
                    {
                        r++;
                    }
                    else
                    {
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

void algos::bysolver2(Satellites &sats)
{
    std::cout << "Starting scheduler\n";
    auto plan = great_plan(sats);
    // std::vector<Interval> plan(plan_.begin(), plan_.end());
    typedef std::tuple<int, SatName, ObsName> SegmentSatObs;

    std::cout << "Intervals: " + std::to_string(plan.size()) + "\n";

    double transmitted = 0;

    std::unique_ptr<CpModelBuilder> cp_model(new CpModelBuilder());
    std::unique_ptr<LinearExpr> optimized(new LinearExpr());
    std::unique_ptr<std::map<SatName, LinearExpr>> underflow_conditions(new std::map<SatName, LinearExpr>());

    std::map<SegmentSatObs, BoolVar> vars;
    std::map<SegmentSatObs, IntervalInfo> id_to_info;
    int nconstraints = 0;

    for (int cnt = 0; cnt < plan.size(); cnt++)
    {
        auto &inter = plan[cnt];

        auto inter_dur = DURATION(inter.start, inter.end);

        auto infos = inter.info;

        std::map<SatName, LinearExpr> uniqueness_conditions_sat;
        std::map<ObsName, LinearExpr> uniqueness_conditions_obs;

        std::map<SatName, bool> can_record;

        for (auto &sat : sats)
        {
            SatName name = sat.second.name;
            (*underflow_conditions)[name] = LinearExpr();
            (*underflow_conditions)[name] -= (int)(1000 * sats.at(sat.second.name).capacity);
            uniqueness_conditions_sat[name] = LinearExpr();
            can_record[name] = false;
        }

        for (int obs = OBS_FIRST; obs <= OBS_NUM; obs++)
            uniqueness_conditions_obs[obs] = LinearExpr();

        for (auto &info : infos)
        {
            if (info.state == State::RECORDING)
            {
                BoolVar v = cp_model->NewBoolVar();
                SegmentSatObs id(cnt, info.sat_name, 0);
                Satellite &sat = sats.at(info.sat_name);
                vars.insert({id, v});
                id_to_info[id] = info;
                (*underflow_conditions)[info.sat_name] -= v * (int)(1000 * inter_dur * sat.recording_speed);
                uniqueness_conditions_sat[info.sat_name] += v;
                can_record[info.sat_name] = true;

                (*optimized) += v * (int)(1000 * inter_dur * sat.recording_speed *
                                          (sat.max_capacity * 0.8 - sat.capacity) / sat.max_capacity);
            }
            else if (info.state == State::TRANSMISSION)
            {
                BoolVar v = cp_model->NewBoolVar();
                SegmentSatObs id(cnt, info.sat_name, info.obs_name);
                Satellite &sat = sats.at(info.sat_name);

                vars.insert({id, v});
                id_to_info[id] = info;
                (*underflow_conditions)[info.sat_name] += v * (int)(1000 * inter_dur * sat.broadcasting_speed);
                uniqueness_conditions_obs[info.obs_name] += v;
                uniqueness_conditions_sat[info.sat_name] += v;
                (*optimized) += v * (int)(1000 * inter_dur * sat.broadcasting_speed);
            }
        }

        for (auto &sat : sats)
        {
            if (can_record[sat.second.name])
                cp_model->AddEquality(uniqueness_conditions_sat[sat.second.name], 1);
            else
                cp_model->AddLessOrEqual(uniqueness_conditions_sat[sat.second.name], 1);
            nconstraints++;
            // I am not sure the copying is needed.
            // Anyway, the goal is to put this linear condition as it is,
            // and then continue to grow it for future segments
            LinearExpr underflow_copy((*underflow_conditions)[sat.second.name]);
            cp_model->AddLessOrEqual(underflow_copy, 0);
            nconstraints++;
        }

        for (int obs = OBS_FIRST; obs <= OBS_NUM; obs++)
        {
            cp_model->AddLessOrEqual(uniqueness_conditions_obs[obs], 1);
            nconstraints++;
        }

        if (cnt % 50 == 0 || cnt == plan.size() - 1)
        {
            printf("%6d/%ld ", cnt, plan.size());
            printf("%d v, %d c ", int(vars.size()), nconstraints);
            fflush(stdout);
            cp_model->Maximize(*optimized);

            const CpSolverResponse response = Solve(cp_model->Build());

            if (response.status() == CpSolverStatus::OPTIMAL ||
                response.status() == CpSolverStatus::FEASIBLE)
            {
                if (response.status() == CpSolverStatus::OPTIMAL)
                    printf("o ");
                if (response.status() == CpSolverStatus::FEASIBLE)
                    printf("f ");
                int r = 0, b = 0;
                for (const auto &v : vars)
                    if (SolutionBooleanValue(response, v.second))
                    {
                        int interval_idx = std::get<0>(v.first);
                        int satname = std::get<1>(v.first);
                        int obsname = std::get<2>(v.first);
                        auto &cur_inter = plan[interval_idx];

                        if (obsname == 0) // recording
                        {
                            algos::add2schedule(cur_inter.start, cur_inter.end, id_to_info[v.first], sats.at(satname));
                            r++;
                        }
                        else
                        {
                            algos::add2schedule(cur_inter.start, cur_inter.end, id_to_info[v.first], sats.at(satname));
                            transmitted += DURATION(cur_inter.start, cur_inter.end) * sats.at(satname).broadcasting_speed;
                            b++;
                        }
                    }
                printf("%2d r, %2d b; total transmitted %lf", r, b, transmitted);
            }
            else
            {
                LOG(INFO) << "No solution found.";
            }
            vars.clear();
            id_to_info.clear();
            cp_model.reset(new CpModelBuilder());
            optimized.reset(new LinearExpr());
            underflow_conditions.reset(new std::map<SatName, LinearExpr>());
            nconstraints = 0;
            printf("\n");
        }

        // break;
    }
}