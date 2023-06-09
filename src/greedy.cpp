#include "algos.h"

#include <algorithm>

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
