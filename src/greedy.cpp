#include "algos.h"

#include <algorithm>

void algos::greedy_capacity(Satellites &sats) {
    using namespace std::chrono;

    std::cout << "Starting greedy algorithm\n";
    auto plan = great_plan(sats);
    std::cout << "Intervals: " + std::to_string(plan.size()) + "\n";

    int cnt = 0;
    int cur_step = 0;

    std::vector<std::pair<SatID, timepoint>> station_busy_until(STN_NUM);
    auto zero_tp = timepoint();
    for (int i = 0; i < STN_NUM; i++) {
        station_busy_until[i] = {0, zero_tp};
    }
    auto one_ms = std::chrono::milliseconds(1);

    for (auto &inter: plan) {
        if (cnt / (plan.size() / 100) > cur_step) {
            std::cout << cur_step++ << "/100\n";
        }
        cnt++;

        auto dur = DURATION(inter.start, inter.end);

        // unique actors
        std::set<SatID> sat_actors;
        std::set<StationID> stn_actors;
        // maps to group actions by actors
        std::unordered_map<SatID, std::vector<std::shared_ptr<IntervalInfo>>> visible_stations;
        std::unordered_map<StationID, std::vector<std::shared_ptr<IntervalInfo>>> visible_sat;
        std::unordered_map<SatID, std::shared_ptr<IntervalInfo>> can_record;
        // get all actors in interval
        for (auto &action: inter.info) {
            auto action_ptr = std::make_shared<IntervalInfo>(action);
            sat_actors.insert(action.sat_id);
            if (action.state == State::TRANSMISSION) 
            {
                if (!visible_stations.count(action.sat_id)) {
                    visible_stations[action.sat_id] = {};
                }
                if (!visible_sat.count(action.station_id)) {
                    visible_sat[action.station_id] = {};
                }
                visible_stations[action.sat_id].push_back(action_ptr);
                visible_sat[action.station_id].push_back(action_ptr);
                stn_actors.insert(action.station_id);
            }
            else {
                if (!visible_stations.count(action.sat_id)) {
                    visible_stations[action.sat_id] = {};
                }
                can_record[action.sat_id] = action_ptr;
            }
        }

        std::vector<std::pair<SatID, double>> sat_cap(visible_stations.size());
        int cnt = 0;
        for (auto &sa: visible_stations) {
            sat_cap[cnt] = {sa.first, sats.at(sa.first).volume / sats.at(sa.first).capacity};
            cnt++;
        }

        std::vector<std::pair<bool, StationID>> is_sat_connected(200, {0, false});
        for (auto &pair: station_busy_until) {
            if (pair.first != 0 && pair.second > inter.start) {
                is_sat_connected[pair.first - 1] = {pair.first, true};
            }
        }

        // sort all satellites by current priority estimation
        std::sort(sat_cap.begin(), sat_cap.end(),
            [&](const std::pair<SatID, double> &a, const std::pair<SatID, double> &b){
                double is_connected_a = is_sat_connected[a.first - 1].second;
                double is_connected_b = is_sat_connected[b.first - 1].second;

                double is_kinosat_a = a.first <= 50;
                double is_kinosat_b = b.first <= 50;

                double a_visibility = (1 - 1.0 * visible_stations.at(a.first).size() / stn_actors.size());
                double b_visibility = (1 - 1.0 * visible_stations.at(b.first).size() / stn_actors.size());

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
        for (auto &vis_stn: visible_stations) {
            std::sort(vis_stn.second.begin(), vis_stn.second.end(), 
                [&](const std::shared_ptr<IntervalInfo> &a, const std::shared_ptr<IntervalInfo> &b) {
                    bool a_connected = (station_busy_until[a->station_id - 1].first == vis_stn.first) && 
                                       (station_busy_until[a->station_id - 1].second > inter.start);
                    bool b_connected = (station_busy_until[b->station_id - 1].first == vis_stn.first) && 
                                       (station_busy_until[b->station_id - 1].second > inter.start);
                    if (a_connected && b_connected) {
                        if (visible_sat.at(a->station_id).size() == visible_sat.at(b->station_id).size())
                            return a->station_id < b->station_id; // always order double city-station same way
                        return visible_sat.at(a->station_id).size() < visible_sat.at(b->station_id).size();
                    }
                    return a_connected > b_connected;

                }
            );
        }

        // fill stations with volume priority
        for (auto &pair: sat_cap) {
            SatID satellite = pair.first;
            bool pair_found = false;

            // check if some stn available, sat can transmission and its not empty
            if (!stn_actors.empty() && visible_stations.count(satellite) && pair.second != 0)
            {
                // choose station
                for (auto &visible: visible_stations.at(satellite)) {
                    auto &cur_stn = visible->station_id;
                    if (stn_actors.count(cur_stn) &&
                        inter.start - one_ms > station_busy_until[cur_stn - 1].second ||
                        (station_busy_until[cur_stn - 1].first == satellite &&
                        inter.start - one_ms <= station_busy_until[cur_stn - 1].second))
                    {
                        pair_found = true;

                        // station busy until this sat is visible
                        station_busy_until[cur_stn - 1] = {
                            satellite, 
                            end_of_current_interval(sats[satellite], cur_stn, inter.start)
                        };

                        // fill interval
                        algos::add2schedule(inter.start, inter.end, *(visible.get()), sats.at(satellite));

                        // this station is busy now
                        stn_actors.erase(cur_stn);

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
