#include "algos.h"

#include <algorithm>

void algos::greedy_random(Satellites &sats, Observatories &obs) {
    using namespace std::chrono;

    std::cout << "Starting greedy algorithm\n";
    auto plan = great_plan(sats);
    std::cout << "Intervals: " + std::to_string(plan.size()) + "\n";

    std::vector<std::pair<SatName, std::shared_ptr<IntervalInfo>>> init_sat_idle;
    std::vector<std::pair<ObsName, std::shared_ptr<IntervalInfo>>> init_obs_idle;

    std::shared_ptr<IntervalInfo> chill = std::make_shared<IntervalInfo>(IntervalInfo({}, SatType::KINOSAT, State::IDLE, {}));

    int cnt = 0;
    int cur_step = 0;

    for (auto &sat: sats) {
        sat.second.full_schedule.reserve(55000);
        init_sat_idle.push_back({sat.first, chill});
    }
    for (auto &observ: obs) {
        observ.second.full_schedule.reserve(40000);
        init_obs_idle.push_back({observ.first, chill});
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

        for (auto &action: inter->info) {
            if (broadcasting_count < obs.size() && action->state == State::BROADCAST)
            {
                if (sat_state.at(action->sat_name)->state != State::BROADCAST && 
                    obs_state.at(action->obs_name)->state != State::BROADCAST) 
                {
                    broadcasting_count++;
                    sat_state.at(action->sat_name) = action;
                    obs_state.at(action->obs_name) = action;
                }
            }

            if (action->state == State::RECORDING && sat_state.at(action->sat_name)->state != State::BROADCAST)
            {
                if (sat_state.at(action->sat_name)->state == State::IDLE) {
                    sat_state.at(action->sat_name) = action;
                }
            }
        }

        //auto b = std::chrono::high_resolution_clock::now();

        // save states in shedule intervals
        for (auto &pair: sat_state) {
            if (pair.second->state != State::IDLE) {
                Interval ii(inter->start, inter->end, {pair.second});
                std::shared_ptr<Interval> new_inter = std::make_shared<Interval>(ii);

                if (pair.second->state == State::RECORDING && sats.at(pair.first).capacity < sats.at(pair.first).max_capacity) {
                    new_inter->capacity_change = sats.at(pair.first).record(ii.duration);
                }
                else {
                    if (pair.second->obs_name.empty()) {
                        throw std::logic_error("Obs and sat state dont coresspond");
                    }
                    new_inter->capacity_change = sats.at(pair.first).broadcast(ii.duration);
                    obs.at(pair.second->obs_name).full_schedule.push_back(new_inter);
                }
                sats.at(pair.first).full_schedule.push_back(new_inter);
            }
        }

        //auto c = std::chrono::high_resolution_clock::now();

        // mean_big1 += duration_cast<microseconds>(b - a).count();
        // mean_big2 += duration_cast<microseconds>(c - b).count();
    }

    // std::cout << mean_big1 / plan.size() << "\n";
    // std::cout << mean_big2 / plan.size() << "\n";
}

void algos::greedy_capacity(Satellites &sats, Observatories &obs) {
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
        for (auto &action: inter->info) {
            sat_actors.insert(action->sat_name);
            if (action->state == State::BROADCAST) {
                if (!visible_obs.count(action->sat_name)) {
                    visible_obs[action->sat_name] = {};
                }
                if (!visible_sat.count(action->obs_name)) {
                    visible_sat[action->obs_name] = {};
                }
                visible_obs[action->sat_name].push_back(action);
                visible_sat[action->obs_name].push_back(action);
                obs_actors.insert(action->obs_name);
            }
            else {
                can_record[action->sat_name] = action;
            }
        }

        // sort all satellites by current capacity
        std::vector<std::pair<SatName, double>> sat_cap(sat_actors.size());
        int cnt = 0;
        for (auto &sa: sat_actors) {
            sat_cap[cnt] = {sa, sats.at(sa).capacity / sats.at(sa).max_capacity};
            cnt++;
        }

        std::sort(sat_cap.begin(), sat_cap.end(),
            [&](const std::pair<SatName, double> &a, const std::pair<SatName, double> &b){
                double a_visibility = 1.0 * visible_obs.at(a.first).size() / obs_actors.size();
                double b_visibility = 1.0 * visible_obs.at(b.first).size() / obs_actors.size();
                return a.second * a_visibility > b.second * b_visibility;
            }
        );

        // sort observatories for satellite by visibility
        for (auto &vis_obs: visible_obs) {
            std::sort(vis_obs.second.begin(), vis_obs.second.end(), 
                [&](const std::shared_ptr<IntervalInfo> &a, const std::shared_ptr<IntervalInfo> &b) {
                    return visible_sat.at(a->obs_name).size() > visible_sat.at(b->obs_name).size();
                }
            );
        }

        // fill observatories with capacity priority
        for (auto &pair: sat_cap) {
            SatName satellite = pair.first;
            bool pair_found = false;

            // check if some obs available, sat can broadcast and its not empty
            if (!obs_actors.empty() && visible_obs.count(satellite) && pair.second != 0)
            {
                // choose observatory
                for (auto &visible: visible_obs.at(satellite)) {
                    auto &cur_obs = visible->obs_name;
                    if (obs_actors.count(cur_obs)) {
                        pair_found = true;
                        // fill interval
                        Interval ii(inter->start, inter->end, {visible});
                        auto new_inter = std::make_shared<Interval>(ii);
                        new_inter->capacity_change = sats.at(satellite).broadcast(ii.duration);

                        sats.at(satellite).full_schedule.push_back(new_inter);
                        obs.at(cur_obs).full_schedule.push_back(new_inter);

                        // this observatory busy now
                        obs_actors.erase(cur_obs);

                        break;
                    }
                }
            }
            if (!pair_found && can_record.count(satellite) && sats.at(satellite).capacity < sats.at(satellite).max_capacity) {
                // satellite can't broadcast but can record
                Interval ii(inter->start, inter->end, {can_record.at(satellite)});
                auto new_inter = std::make_shared<Interval>(ii);
                new_inter->capacity_change = sats.at(pair.first).record(ii.duration);
                sats.at(satellite).full_schedule.push_back(new_inter);
            }
        }
    }
}

void algos::greedy_exhaustive(Satellites &sats, Observatories &obs) {
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

        // maps to group actions by actors
        std::unordered_map<SatName, std::vector<std::shared_ptr<IntervalInfo>>> visible_obs;
        std::unordered_map<ObsName, std::vector<std::shared_ptr<IntervalInfo>>> visible_sat;
        std::unordered_map<SatName, std::shared_ptr<IntervalInfo>> can_record;
        // get all actors in interval
        for (auto &action: inter->info) {
            // sat_actors.insert(action->sat_name);
            if (action->state == State::BROADCAST) {
                if (sats.at(action->sat_name).capacity > 0) {
                    if (!visible_obs.count(action->sat_name)) {
                        visible_obs[action->sat_name] = {};
                    }
                    if (!visible_sat.count(action->obs_name)) {
                        visible_sat[action->obs_name] = {};
                    }
                    visible_obs[action->sat_name].push_back(action);
                    visible_sat[action->obs_name].push_back(action);
                }
                // obs_actors.insert(action->obs_name);
            }
            else {
                can_record[action->sat_name] = action;
            }
        }

        // mb sort stations before

        std::vector<std::pair<ObsName, std::vector<std::shared_ptr<IntervalInfo>>>> obs_sat(visible_sat.begin(), visible_sat.end());

        // sort satellite for observatories by capacity
        for (auto &vis_sat: obs_sat) {
            std::sort(vis_sat.second.begin(), vis_sat.second.end(), 
                [&](const std::shared_ptr<IntervalInfo> &a, const std::shared_ptr<IntervalInfo> &b) {
                    return sats.at(a->sat_name).capacity < sats.at(b->sat_name).capacity;
                }
            );
        }

        int obs_num = obs_sat.size();

        // sats < 200 so string is acceptable and easier to manipulate
        std::string max_case(obs_num, char(0));
        std::string cur_case(obs_num, char(0));
        std::string opt_case(obs_num, char(0));
        double opt_data = 0;

        int obs_cnt = 0;
        for (int cur_obs = 0; cur_obs < obs_num; cur_obs++) {
            max_case[cur_obs] = obs_sat[cur_obs].second.size() - 1;
        }

        // check all possible cases
        while (cur_case < max_case) {
            std::set<SatName> used;
            double cur_data = 0;

            // fill observatories
            for (int cur_obs = 0; cur_obs < obs_num; cur_obs++) {
                auto &cur_sat = obs_sat[cur_obs].second[cur_case[cur_obs]]->sat_name;
                if (!used.count(cur_sat)) {
                    used.insert(cur_sat);
                    cur_data += sats.at(cur_sat).can_broadcast(inter->duration);
                }
            }

            // save new optimal solution
            if (opt_data <= cur_data) {
                opt_data = cur_data;
                opt_case = cur_case;
            }

            // increase case number
            for (int cur_obs = obs_num - 1; cur_obs >= 0; cur_obs--) {
                if (cur_case[cur_obs] < max_case[cur_obs]) {
                    cur_case[cur_obs]++;
                    break;
                }
            }
        }

        // restore optimal case
        std::set<SatName> used;
        for (int cur_obs = 0; cur_obs < obs_num; cur_obs++) {
            auto cur_action = obs_sat[cur_obs].second[cur_case[cur_obs]];
            if (!used.count(cur_action->sat_name)) {
                Interval ii(inter->start, inter->end, {cur_action});
                auto new_inter = std::make_shared<Interval>(ii);
                new_inter->capacity_change = sats.at(cur_action->sat_name).broadcast(ii.duration);

                sats.at(cur_action->sat_name).full_schedule.push_back(new_inter);
                obs.at(obs_sat[cur_obs].first).full_schedule.push_back(new_inter);
            }
        }

        for (auto &sat_info: can_record) {
            if (!used.count(sat_info.first)) {
                Interval ii(inter->start, inter->end, {can_record.at(sat_info.first)});
                auto new_inter = std::make_shared<Interval>(ii);
                new_inter->capacity_change = sats.at(sat_info.first).record(ii.duration);
                sats.at(sat_info.first).full_schedule.push_back(new_inter);
            }
        }
   }
}