#include "algos.h"

#include <algorithm>

void algos::greedy_capacity(Satellites &sats) {
    using namespace std::chrono;

    std::cout << "Starting greedy algorithm\n";
    auto plan = great_plan(sats);
    std::cout << "Intervals: " + std::to_string(plan.size()) + "\n";

    int cnt = 0;
    int cur_step = 0;

    std::vector<std::pair<SatName, timepoint>> obs_busy_until(OBS_NUM);
    auto zero_tp = timepoint();
    for (int i = 0; i < OBS_NUM; i++) {
        obs_busy_until[i] = {0, zero_tp};
    }
    auto one_ms = std::chrono::milliseconds(1);

    for (auto &inter: plan) {
        if (cnt / (plan.size() / 100) > cur_step) {
            std::cout << cur_step++ << "/100\n";
        }
        cnt++;

        auto dur = DURATION(inter.start, inter.end);

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
            if (action.state == State::TRANSMISSION) 
            {
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
                if (!visible_obs.count(action.sat_name)) {
                    visible_obs[action.sat_name] = {};
                }
                can_record[action.sat_name] = action_ptr;
            }
        }

        // если спутник прервал связь, обсерватория не занимается до конца передачи этого спутника


        // use info from previous segment
        // auto prev_state = get_prev_state(sats, inter.start);
        // for (auto sat_obs: prev_state) {
        //     auto &sat_prev = sat_obs.first;
        //     auto &obs_prev = sat_obs.second;

        //     if (sat_actors.count(sat_prev) && obs_actors.count(obs_prev) && visible_obs.count(sat_prev)) {
        //         for (auto &vis_obs: visible_obs[sat_prev]) {
        //             if (vis_obs->obs_name == obs_prev) {
        //                 algos::add2schedule(inter.start, inter.end, *(vis_obs.get()), sats.at(sat_prev));
        //                 visible_obs.erase(sat_prev); // this satellite dont pass further
        //                 obs_actors.erase(obs_prev);
        //                 break;
        //             }
        //         }
        //     }
        // }

        std::vector<std::pair<SatName, double>> sat_cap(visible_obs.size());
        int cnt = 0;
        for (auto &sa: visible_obs) {
            sat_cap[cnt] = {sa.first, sats.at(sa.first).volume / sats.at(sa.first).capacity};
            cnt++;
        }

        std::vector<std::pair<bool, ObsName>> is_sat_connected(200, {0, false});
        for (auto &pair: obs_busy_until) {
            if (pair.first != 0 && pair.second > inter.start) {
                is_sat_connected[pair.first - 1] = {pair.first, true};
            }
        }

        // sort all satellites by current priority estimation
        std::sort(sat_cap.begin(), sat_cap.end(),
            [&](const std::pair<SatName, double> &a, const std::pair<SatName, double> &b){
                double is_connected_a = is_sat_connected[a.first - 1].second;
                double is_connected_b = is_sat_connected[b.first - 1].second;

                double is_kinosat_a = a.first <= 50;
                double is_kinosat_b = b.first <= 50;

                double a_visibility = (1 - 1.0 * visible_obs.at(a.first).size() / obs_actors.size());
                double b_visibility = (1 - 1.0 * visible_obs.at(b.first).size() / obs_actors.size());

                double a_fullness = a.second;
                double b_fullness = b.second;

                auto dur_a = dur;
                auto dur_b = dur;

                double a_enough_data = (sats.at(a.first).volume > 700 * sats.at(a.first).transmission_speed);//dur * sats.at(a.first).broadcasting_speed);
                double b_enough_data = (sats.at(b.first).volume > 700 * sats.at(b.first).transmission_speed);//dur * sats.at(b.first).broadcasting_speed);

                if (is_connected_a == is_connected_b) {
                    if (a_enough_data == b_enough_data) {
                        return (a_visibility * 0.6 + a_fullness * 0.2 + 0.1 * is_kinosat_a) >
                               (b_visibility * 0.6 + b_fullness * 0.2 + 0.1 * is_kinosat_b);
                    }
                    return a_enough_data > b_enough_data;
                }
                return is_connected_a > is_connected_b;
            }
        );

        // sort stations for satellite by visibility
        for (auto &vis_obs: visible_obs) {
            std::sort(vis_obs.second.begin(), vis_obs.second.end(), 
                [&](const std::shared_ptr<IntervalInfo> &a, const std::shared_ptr<IntervalInfo> &b) {
                    bool a_connected = (obs_busy_until[a->obs_name - 1].first == vis_obs.first) && 
                                       (obs_busy_until[a->obs_name - 1].second > inter.start);
                    bool b_connected = (obs_busy_until[b->obs_name - 1].first == vis_obs.first) && 
                                       (obs_busy_until[b->obs_name - 1].second > inter.start);
                    if (a_connected && b_connected) {
                        if (visible_sat.at(a->obs_name).size() == visible_sat.at(b->obs_name).size())
                            return a->obs_name < b->obs_name; // always order double city-station same way
                        return visible_sat.at(a->obs_name).size() < visible_sat.at(b->obs_name).size();
                    }
                    return a_connected > b_connected;

                }
            );
        }

        // fill stations with volume priority
        for (auto &pair: sat_cap) {
            SatName satellite = pair.first;
            bool pair_found = false;

            // check if some obs available, sat can transmission and its not empty
            if (!obs_actors.empty() && visible_obs.count(satellite) && pair.second != 0)
            {
                // choose station
                for (auto &visible: visible_obs.at(satellite)) {
                    auto &cur_obs = visible->obs_name;
                    if (obs_actors.count(cur_obs) &&
                        inter.start - one_ms > obs_busy_until[cur_obs - 1].second ||
                        (obs_busy_until[cur_obs - 1].first == satellite &&
                        inter.start - one_ms <= obs_busy_until[cur_obs - 1].second))
                    {
                        pair_found = true;

                        // obs busy until this sat is visible
                        obs_busy_until[cur_obs - 1] = {
                            satellite, 
                            end_of_current_interval(sats[satellite], cur_obs, inter.start)
                        };

                        // fill interval
                        algos::add2schedule(inter.start, inter.end, *(visible.get()), sats.at(satellite));

                        // this station is busy now
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
