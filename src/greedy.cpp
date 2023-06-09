#include "algos.h"

#include <algorithm>

void algos::greedy_random(Satellites &sats) {
    using namespace std::chrono;

    std::cout << "Starting greedy algorithm\n";
    auto plan = great_plan(sats);
    std::cout << "Intervals: " + std::to_string(plan.size()) + "\n";

    std::vector<std::pair<SatName, std::shared_ptr<IntervalInfo>>> init_sat_idle;
    std::vector<std::pair<ObsName, std::shared_ptr<IntervalInfo>>> init_obs_idle;

    std::shared_ptr<IntervalInfo> chill = std::make_shared<IntervalInfo>(IntervalInfo({}, State::IDLE, {}));

    int cnt = 0;
    int cur_step = 0;

    for (int isat = 1; isat <= SAT_NUM; isat++) {
        sats[isat].full_schedule.reserve(55000);
        init_sat_idle.push_back({isat, chill});
    }
    for (int obs = OBS_FIRST; obs <= OBS_NUM; obs++) {
        init_obs_idle.push_back({obs, chill});
    }

    float mean_big1 = 0;
    float mean_big2 = 0;

    std::unordered_map<SatName, std::shared_ptr<IntervalInfo>> sat_state(init_sat_idle.begin(), init_sat_idle.end());
    std::unordered_map<ObsName, std::shared_ptr<IntervalInfo>> obs_state(init_obs_idle.begin(), init_obs_idle.end());

    for (auto &inter: plan) {
        if (cnt / (plan.size() / 100) > cur_step) {
            std::cout << cur_step++ << "/100\n";
        }
        cnt++;

        //auto a = std::chrono::high_resolution_clock::now();

        int broadcasting_count = 0;

        // init all objects with IDLE state
        for (auto &ss: sat_state) {
            ss.second = chill;
        }
        for (auto &os: obs_state) {
            os.second = chill;
        }

        for (auto &action: inter.info) {
            auto action_ptr = std::make_shared<IntervalInfo>(action);
            if (broadcasting_count < OBS_NUM && action.state == State::TRANSMISSION)
            {
                if (sat_state.at(action.sat_name)->state != State::TRANSMISSION && 
                    obs_state.at(action.obs_name)->state != State::TRANSMISSION && sats.at(action.sat_name).capacity > 0) 
                {
                    broadcasting_count++;
                    sat_state.at(action.sat_name) = action_ptr;
                    obs_state.at(action.obs_name) = action_ptr;
                }
            }

            if (action.state == State::RECORDING && sat_state.at(action.sat_name)->state != State::TRANSMISSION)
            {
                if (sat_state.at(action.sat_name)->state == State::IDLE) {
                    sat_state.at(action.sat_name) = action_ptr;
                }
            }
        }

        //auto b = std::chrono::high_resolution_clock::now();

        // save states in shedule intervals
        for (auto &pair: sat_state) {
            if (pair.second->state != State::IDLE) {
                algos::add2schedule(inter.start, inter.end, *(pair.second.get()), sats.at(pair.first));
            }
        }

        //auto c = std::chrono::high_resolution_clock::now();

        // mean_big1 += duration_cast<microseconds>(b - a).count();
        // mean_big2 += duration_cast<microseconds>(c - b).count();
    }

    // std::cout << mean_big1 / plan.size() << "\n";
    // std::cout << mean_big2 / plan.size() << "\n";
}

void algos::greedy_capacity(Satellites &sats) {
    using namespace std::chrono;

    std::cout << "Starting greedy algorithm\n";
    auto plan = great_plan(sats);
    std::cout << "Intervals: " + std::to_string(plan.size()) + "\n";

    int cnt = 0;
    int cur_step = 0;

    for (auto &inter: plan) {
        if (cnt / (plan.size() / 100) > cur_step) {
            std::cout << cur_step++ << "/100\n";
        }
        cnt++;

        // unique actors
        std::set<SatName> sat_actors;
        std::set<ObsName> obs_actors;
        // maps to group actions by actors
        std::unordered_map<SatName, std::vector<std::shared_ptr<IntervalInfo>>> visible_obs;
        std::unordered_map<ObsName, std::vector<std::shared_ptr<IntervalInfo>>> visible_sat;
        std::unordered_map<SatName, std::shared_ptr<IntervalInfo>> can_record;
        // get all actors in interval
        for (auto &action: inter.info) {
            auto action_ptr = std::make_shared<IntervalInfo>(action);
            sat_actors.insert(action.sat_name);
            if (action.state == State::TRANSMISSION) {
                if (!visible_obs.count(action.sat_name)) {
                    visible_obs[action.sat_name] = {};
                }
                if (!visible_sat.count(action.obs_name)) {
                    visible_sat[action.obs_name] = {};
                }
                visible_obs[action.sat_name].push_back(action_ptr);
                visible_sat[action.obs_name].push_back(action_ptr);
                obs_actors.insert(action.obs_name);
            }
            else {
                can_record[action.sat_name] = action_ptr;
            }
        }

        std::vector<std::pair<SatName, double>> sat_cap(visible_obs.size());
        int cnt = 0;
        for (auto &sa: visible_obs) {
            sat_cap[cnt] = {sa.first, sats.at(sa.first).capacity / sats.at(sa.first).max_capacity};
            cnt++;
        }

        // sort all satellites by current priority estimation
        std::sort(sat_cap.begin(), sat_cap.end(),
            [&](const std::pair<SatName, double> &a, const std::pair<SatName, double> &b){
                double is_kinosat_a = a.first <= 50;
                double is_kinosat_b = b.first <= 50;

                double a_visibility = (1 - 1.0 * visible_obs.at(a.first).size() / obs_actors.size());
                double b_visibility = (1 - 1.0 * visible_obs.at(b.first).size() / obs_actors.size());

                double a_fullness = a.second;
                double b_fullness = b.second;

                auto dur = DURATION(inter.start, inter.end);

                double a_enough_data = (sats.at(a.first).capacity > dur * sats.at(a.first).broadcasting_speed);
                double b_enough_data = (sats.at(b.first).capacity > dur * sats.at(b.first).broadcasting_speed);
                //return (a_fullness) > (b_fullness);
                return (a_visibility * 0.6 + a_fullness * 0.2 + 0.1 * is_kinosat_a + 0.4 * a_enough_data) >
                    (b_visibility * 0.6 + b_fullness * 0.2 + 0.1 * is_kinosat_b + 0.4 * b_enough_data);
            }
        );

        // sort observatories for satellite by visibility
        for (auto &vis_obs: visible_obs) {
            std::sort(vis_obs.second.begin(), vis_obs.second.end(), 
                [&](const std::shared_ptr<IntervalInfo> &a, const std::shared_ptr<IntervalInfo> &b) {
                    if (visible_sat.at(a->obs_name).size() == visible_sat.at(b->obs_name).size())
                        return a->obs_name < b->obs_name; // always order double city-observatory same way
                    return visible_sat.at(a->obs_name).size() < visible_sat.at(b->obs_name).size();
                }
            );
        }

        // fill observatories with capacity priority
        for (auto &pair: sat_cap) {
            SatName satellite = pair.first;
            bool pair_found = false;

            // check if some obs available, sat can transmission and its not empty
            if (!obs_actors.empty() && visible_obs.count(satellite) && pair.second != 0)
            {
                // choose observatory
                for (auto &visible: visible_obs.at(satellite)) {
                    auto &cur_obs = visible->obs_name;
                    if (obs_actors.count(cur_obs)) {
                        pair_found = true;
                        // fill interval
                        algos::add2schedule(inter.start, inter.end, *(visible.get()), sats.at(satellite));

                        // this observatory busy now
                        obs_actors.erase(cur_obs);

                        break;
                    }
                }
            }
            if (!pair_found && can_record.count(satellite)) {
                // satellite can't transmission but can record
                algos::add2schedule(inter.start, inter.end, *(can_record.at(satellite).get()), sats.at(satellite));
            }
        }
    }
}

void algos::greedy_exhaustive(Satellites &sats) {
    using namespace std::chrono;

    std::cout << "Starting greedy algorithm\n";
    auto plan = great_plan(sats);
    std::cout << "Intervals: " + std::to_string(plan.size()) + "\n";

    int cnt = 0;
    int cur_step = 0;

    for (auto &inter: plan) {
        if (cnt / (plan.size() / 100000) > cur_step) {
            std::cout << cur_step++ << "/100000\n";
        }
        cnt++;

        // maps to group actions by actors
        std::unordered_map<SatName, std::vector<std::shared_ptr<IntervalInfo>>> visible_obs;
        std::unordered_map<ObsName, std::vector<std::shared_ptr<IntervalInfo>>> visible_sat;
        std::unordered_map<SatName, std::shared_ptr<IntervalInfo>> can_record;
        // get all actors in interval
        for (auto &action: inter.info) {
            auto action_ptr = std::make_shared<IntervalInfo>(action);
            // sat_actors.insert(action->sat_name);
            if (action.state == State::TRANSMISSION) {
                if (sats.at(action.sat_name).capacity / sats.at(action.sat_name).max_capacity > 0.5) {
                    if (!visible_obs.count(action.sat_name)) {
                        visible_obs[action.sat_name] = {};
                    }
                    if (!visible_sat.count(action.obs_name)) {
                        visible_sat[action.obs_name] = {};
                    }
                    visible_obs[action.sat_name].push_back(action_ptr);
                    visible_sat[action.obs_name].push_back(action_ptr);
                }
                // obs_actors.insert(action->obs_name);
            }
            else {
                can_record[action.sat_name] = action_ptr;
            }
        }

        // mb sort stations before

        std::vector<std::pair<ObsName, std::vector<SatName>>> obs_sat;
        for (auto &vis_sat: visible_sat) {
            std::vector<SatName> obs_satels;
            for (auto &event: vis_sat.second) {
                obs_satels.push_back(event->sat_name);
            }
            obs_sat.push_back({vis_sat.first, obs_satels});
        }

        // sort satellite for observatories by capacity
        for (auto &vis_sat: obs_sat) {
            std::sort(vis_sat.second.begin(), vis_sat.second.end(), 
                [&](const SatName &a, const SatName &b) {
                    double a_visibility = 1.0 * visible_obs.at(a).size() / visible_sat.size();
                    double b_visibility = 1.0 * visible_obs.at(b).size() / visible_sat.size();
                    return 1.0 * sats.at(a).capacity / sats.at(a).max_capacity * a_visibility
                         > 1.0 * sats.at(b).capacity / sats.at(b).max_capacity * b_visibility;
                }
            );
        }

        int obs_num = obs_sat.size();
        std::set<SatName> used_sats;

        if (obs_num > 0) {
            // sats < 200 so string is acceptable and easier to manipulate
            std::string max_case(obs_num, char(0));
            std::string cur_case(obs_num, char(0));
            std::string opt_case(obs_num, char(0));
            double opt_data = 0;

            int obs_cnt = 0;
            unsigned int cases_num = 1;
            for (int cur_obs = 0; cur_obs < obs_num; cur_obs++) {
                //cases_space.push_back(obs_sat[cur_obs].second.begin(), obs_sat[cur_obs].second.end());
                max_case[cur_obs] = obs_sat[cur_obs].second.size() - 1;
                cases_num *= obs_sat[cur_obs].second.size();
            }
            //std::cout << cases_num << "\n";

            // alloc data for full search
            bool finish = false;
            std::unordered_map<SatName, bool> used;
            for (auto &satel: visible_obs) {
                used[satel.first] = false;
            }
            std::unordered_map<SatName, double> fullness;
            double mean_fullness = 0;
            for (auto &satel: visible_obs) {
                fullness[satel.first] = 1.0 * sats.at(satel.first).capacity / sats.at(satel.first).max_capacity;
                mean_fullness += fullness[satel.first];
            }
            mean_fullness /= fullness.size();

            int changed_last = 0;
            int changed_first = 0;
            bool is_overflow = false;

            // check all possible cases
            while (!finish) {
                //double cur_fullness = 0;
                int used_num = 0;

                // fill observatories
                int start_idx = changed_last;
                if (is_overflow)
                    start_idx = 0;
                for (int cur_obs = start_idx; cur_obs <= changed_last; ++cur_obs) {
                    auto &cur_sat = obs_sat[cur_obs].second[cur_case[cur_obs]];
                    if (!used[cur_sat]) {
                        used[cur_sat] = true;
                        used_num++;
                        //cur_fullness += fullness[cur_sat];//sats.at(cur_sat).can_broadcast(inter->duration);
                    }
                }

                // ok to stop
                //std::cout << used_num << " " << obs_num << "\n";
                if (used_num == obs_num) { //cur_fullness > mean_fullness * obs_num && 
                    opt_case = cur_case;
                    break;
                }

                // // save new optimal solution
                // if (opt_data < cur_fullness) {
                //     opt_data = cur_fullness;
                //     opt_case = cur_case;
                // }

                if (cur_case == max_case)
                    break;
                
                is_overflow = false;
                for (int cur_obs = start_idx; cur_obs < obs_num; ++cur_obs) {
                    used[obs_sat[cur_obs].second[cur_case[cur_obs]]] = false;
                    used_num--;
                    if (cur_case[cur_obs] < max_case[cur_obs]) {
                        cur_case[cur_obs]++;
                        changed_last = cur_obs;
                        break;
                    } else {
                        cur_case[cur_obs] = 0;
                        is_overflow = true;
                    }
                }
            }
            // restore optimal case
            for (int cur_obs = 0; cur_obs < obs_num; cur_obs++) {
                auto cur_action = visible_sat[obs_sat[cur_obs].first][opt_case[cur_obs]];
                if (!used_sats.count(cur_action->sat_name)) {
                    used_sats.insert(cur_action->sat_name);
                    algos::add2schedule(inter.start, inter.end, *(cur_action.get()), sats.at(cur_action->sat_name));
                }
            }
        }

        for (auto &sat_info: can_record) {
            if (!used_sats.count(sat_info.first)) {
                algos::add2schedule(inter.start, inter.end, *(can_record.at(sat_info.first).get()), sats.at(sat_info.first));
            }
        }
   }
}