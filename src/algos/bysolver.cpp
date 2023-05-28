#include "algos.h"


#include "ortools/base/logging.h"
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model.pb.h"
#include "ortools/sat/cp_model_solver.h"
#include "ortools/util/sorted_interval_list.h"


extern timepoint START_MODELLING;

using namespace operations_research;
using namespace operations_research::sat;

void algos::bysolver (Satellites &sats, Observatories &obs) {
    std::cout << "Starting scheduler\n";
    auto plan = great_plan(sats);
    std::cout << "Intervals: " + std::to_string(plan.size()) + "\n";

    int cnt = 0;
    
    double transmitted = 0;
        
    
    for (auto &inter: plan)
    {
        cnt++;
        printf("%6d/%d ", cnt, plan.size());
        
        auto infos = inter->info;

        CpModelBuilder cp_model;

        std::map<std::string, BoolVar> vars;
        
        LinearExpr optimized;
        std::map<SatName, LinearExpr> underflow_conditions;
        std::map<SatName, LinearExpr> uniqueness_conditions_sat;
        std::map<ObsName, LinearExpr> uniqueness_conditions_obs;
        
        std::map<SatName, bool> can_record;
        
        std::map<std::string, std::shared_ptr<IntervalInfo>> id_to_info;
        
        for (auto & sat : sats)
        {
            SatName name = sat.second.name;
            underflow_conditions[name] = LinearExpr();
            underflow_conditions[name] -= (int)(1000 * sats.at(sat.second.name).capacity);
            uniqueness_conditions_sat[name] = LinearExpr();
            can_record[name] = false;
        }
        
        for (auto & ob : obs)
            uniqueness_conditions_obs[ob.second.name] = LinearExpr();
        
        
        for (auto &info : infos)
        {
            if (info->state == State::RECORDING) 
            {
                BoolVar v = cp_model.NewBoolVar();
                std::string id = std::to_string(info->sat_name) + "_0";
                Satellite &sat = sats.at(info->sat_name);
                vars.insert({id, v});
                id_to_info[id] = info;
                underflow_conditions[info->sat_name] -= v * (int)(1000 * inter->duration * sat.recording_speed);
                uniqueness_conditions_sat[info->sat_name] += v;
                can_record[info->sat_name] = true;
                
                optimized += v * (int)(1000 * inter->duration * sat.recording_speed *
                                              (sat.max_capacity * 0.8 - sat.capacity) / sat.max_capacity);
            }
            else if (info->state == State::TRANSMISSION)
            {
                BoolVar v = cp_model.NewBoolVar();
                std::string id = std::to_string(info->sat_name) + "_" + info->obs_name;
                Satellite &sat = sats.at(info->sat_name);
                
                vars.insert({id, v}); 
                id_to_info[id] = info;
                underflow_conditions[info->sat_name] += v * (int)(1000 * inter->duration * sat.broadcasting_speed);
                uniqueness_conditions_obs[info->obs_name] += v;
                uniqueness_conditions_sat[info->sat_name] += v;
                optimized += v * (int)(1000 * inter->duration * sat.broadcasting_speed);
            }
        }
        
        int nconstraints = 0;
        for (auto & sat : sats)
        {
            if (can_record[sat.second.name])
                cp_model.AddEquality(uniqueness_conditions_sat[sat.second.name], 1);
            else
                cp_model.AddLessOrEqual(uniqueness_conditions_sat[sat.second.name], 1);
            nconstraints++;
            cp_model.AddLessOrEqual(underflow_conditions[sat.second.name], 0);
            nconstraints++;
        }
        
        for (auto &ob : obs)
        {
            cp_model.AddLessOrEqual(uniqueness_conditions_obs[ob.second.name], 1);
            nconstraints++;
        }
        
        printf("%d v, %d c ", vars.size(), nconstraints);
        cp_model.Maximize(optimized);
        
        const CpSolverResponse response = Solve(cp_model.Build());
        
        if (response.status() == CpSolverStatus::OPTIMAL ||
            response.status() == CpSolverStatus::FEASIBLE) {
            if (response.status() == CpSolverStatus::OPTIMAL)
                printf("o ");
            if (response.status() == CpSolverStatus::FEASIBLE)
                printf("f ");
            int r = 0, b = 0;
            for (const auto &v : vars)
                if (SolutionBooleanValue(response, v.second))
                {
                    if (v.first[7] == '0') // recording
                    {
                        int satname = std::atoi(v.first.substr(0, 6).c_str());
                        Interval ii(inter->start, inter->end, {id_to_info[v.first]});
                        auto new_inter = std::make_shared<Interval>(ii);
                        algos::add2schedule(new_inter, sats.at(satname));
                        r++;
                    }
                    else
                    {
                        int satname = std::atoi(v.first.substr(0, 6).c_str());
                        std::string obsname = v.first.substr(7);
                        Interval ii(inter->start, inter->end, {id_to_info[v.first]});
                        auto new_inter = std::make_shared<Interval>(ii);
                        algos::add2schedule(new_inter, sats.at(satname), obs.at(obsname));
                        transmitted += inter->duration * sats.at(satname).broadcasting_speed;
                        b++;
                    }
                }
            printf("%2d r, %2d b; total transmitted %lf", r, b, transmitted);
        } else {
           LOG(INFO) << "No solution found.";
        }
         
        printf("\n");
        //break;
    }
    
}

void algos::bysolver2 (Satellites &sats, Observatories &obs) {
    std::cout << "Starting scheduler\n";
    auto plan_ = great_plan(sats);
    VecSchedule plan(plan_.begin(), plan_.end());
    
    std::cout << "Intervals: " + std::to_string(plan.size()) + "\n";

    double transmitted = 0;
        
    std::unique_ptr<CpModelBuilder> cp_model(new CpModelBuilder());
    std::unique_ptr<LinearExpr> optimized(new LinearExpr());
    std::unique_ptr<std::map<SatName, LinearExpr>> underflow_conditions(new std::map<SatName, LinearExpr>());
    
    typedef std::tuple<int, SatName, ObsName> SegmentSatObs;
    
    std::map<SegmentSatObs, BoolVar> vars;
    std::map<SegmentSatObs, std::shared_ptr<IntervalInfo>> id_to_info;
    
    for (int cnt = 0; cnt < plan.size(); cnt++)
    {
        auto &inter = plan[cnt];
        
        printf("%6d/%d ", cnt, plan.size());
        
        auto infos = inter->info;
        
        std::map<SatName, LinearExpr> uniqueness_conditions_sat;
        std::map<ObsName, LinearExpr> uniqueness_conditions_obs;
        
        std::map<SatName, bool> can_record;
        
        for (auto & sat : sats)
        {
            SatName name = sat.second.name;
            underflow_conditions->at(name) = LinearExpr();
            underflow_conditions->at(name) -= (int)(1000 * sats.at(sat.second.name).capacity);
            uniqueness_conditions_sat[name] = LinearExpr();
            can_record[name] = false;
        }
        
        for (auto & ob : obs)
            uniqueness_conditions_obs[ob.second.name] = LinearExpr();
        
        
        for (auto &info : infos)
        {
            if (info->state == State::RECORDING) 
            {
                BoolVar v = cp_model->NewBoolVar();
                SegmentSatObs id(cnt, info->sat_name, "0");
                Satellite &sat = sats.at(info->sat_name);
                vars.insert({id, v});
                id_to_info[id] = info;
                underflow_conditions->at(info->sat_name) -= v * (int)(1000 * inter->duration * sat.recording_speed);
                uniqueness_conditions_sat[info->sat_name] += v;
                can_record[info->sat_name] = true;
                
                //optimized += v * (int)(1000 * inter->duration * sat.recording_speed *
                //                              (sat.max_capacity * 0.8 - sat.capacity) / sat.max_capacity);
            }
            else if (info->state == State::TRANSMISSION)
            {
                BoolVar v = cp_model->NewBoolVar();
                SegmentSatObs id(cnt, info->sat_name, info->obs_name);
                Satellite &sat = sats.at(info->sat_name);
                
                vars.insert({id, v}); 
                id_to_info[id] = info;
                underflow_conditions->at(info->sat_name) += v * (int)(1000 * inter->duration * sat.broadcasting_speed);
                uniqueness_conditions_obs[info->obs_name] += v;
                uniqueness_conditions_sat[info->sat_name] += v;
                (*optimized) += v * (int)(1000 * inter->duration * sat.broadcasting_speed);
            }
        }
        
        int nconstraints = 0;
        for (auto & sat : sats)
        {
            if (can_record[sat.second.name])
                cp_model->AddEquality(uniqueness_conditions_sat[sat.second.name], 1);
            else
                cp_model->AddLessOrEqual(uniqueness_conditions_sat[sat.second.name], 1);
            nconstraints++;
            cp_model->AddLessOrEqual(underflow_conditions->at(sat.second.name), 0);
            nconstraints++;
        }
        
        for (auto &ob : obs)
        {
            cp_model->AddLessOrEqual(uniqueness_conditions_obs[ob.second.name], 1);
            nconstraints++;
        }
        
        printf("%d v, %d c ", vars.size(), nconstraints);
        
        if (cnt % 10 == 0)
        {
            cp_model->Maximize(*optimized);

            const CpSolverResponse response = Solve(cp_model->Build());

            if (response.status() == CpSolverStatus::OPTIMAL ||
                response.status() == CpSolverStatus::FEASIBLE) {
                if (response.status() == CpSolverStatus::OPTIMAL)
                    printf("o ");
                if (response.status() == CpSolverStatus::FEASIBLE)
                    printf("f ");
                int r = 0, b = 0;
                for (const auto &v : vars)
                    if (SolutionBooleanValue(response, v.second))
                    {
                        int interval_idx =  std::get<0>(v.first);
                        int satname = std::get<1>(v.first);
                        std::string obsname = std::get<2>(v.first);
                        auto &cur_inter = plan[interval_idx];

                        if (obsname[0] == '0') // recording
                        {
                            
                            Interval ii(cur_inter->start, cur_inter->end, {id_to_info[v.first]});
                            auto new_inter = std::make_shared<Interval>(ii);
                            algos::add2schedule(new_inter, sats.at(satname));
                            r++;
                        }
                        else
                        {
                            Interval ii(cur_inter->start, cur_inter->end, {id_to_info[v.first]});
                            auto new_inter = std::make_shared<Interval>(ii);
                            algos::add2schedule(new_inter, sats.at(satname), obs.at(obsname));
                            transmitted += cur_inter->duration * sats.at(satname).broadcasting_speed;
                            b++;
                        }
                    }
                printf("%2d r, %2d b; total transmitted %lf", r, b, transmitted);
            } else {
               LOG(INFO) << "No solution found.";
            }
            vars.clear();
            id_to_info.clear();
            cp_model.reset(new CpModelBuilder());
            optimized.reset(new LinearExpr());
            underflow_conditions.reset(new std::map<SatName, LinearExpr>());
        }
         
        printf("\n");
        //break;
    }
    
}