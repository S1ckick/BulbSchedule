#include "algos.h"

void print_time(const timepoint &a, const timepoint &b) {
    std::cout << (std::chrono::duration<double, std::milli>(a - TP_START) * std::chrono::milliseconds::period::num /
        std::chrono::milliseconds::period::den).count()
                << " " << (std::chrono::duration<double, std::milli>(b - TP_START) * std::chrono::milliseconds::period::num /
        std::chrono::milliseconds::period::den).count()
                << '\n';
}

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
                double a_visibility = visible_obs.at(a.first).size() / obs_actors.size();
                double b_visibility = visible_obs.at(b.first).size() / obs_actors.size();
                return a.second * a_visibility > b.second * b_visibility;
            }
        );

        // sort observatories by visibility
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

#if 0
void algos::build_schedule(Satellites &sats)
{
    for (auto &pair : sats)
    {
        auto &sat = pair.second;
        auto &sat_ints = sat.ints_in_area;       // intervals when satellite in recording area
        auto &obs_ints = sat.ints_observatories; // observatories intervals for current satellite only
        auto &schedule = sat.full_schedule;      // array for all intervals of operations for current satellite
        int ints_num = sat_ints.size();

        auto obs_it = obs_ints.begin();                   // current observatory interval to be handled
        auto last_trigger = (*(sat_ints.begin()))->start; // here will be stored end time of last operation

        // Each iteration handle time from start of current interval to start of next interval
        auto it = sat_ints.begin();

        for (;it != sat_ints.end(); ++it) {
            auto cur_int = *it;
            auto begin_cur = cur_int->start;
            auto end_cur = cur_int->end;
            // auto start_capacity = sat.capacity; // for debug
            // std::cout << "\n\nSat interval:";
            // print_time(begin_cur, end_cur);

            auto begin_next = end_cur; // start of next interval
            it++;
            if (it != sat_ints.end())
                begin_next = (*it)->start;
            it--;

            // Handle all observatory intervals that starts between begin_cur and begin_next
            while (obs_it != obs_ints.end() && (*obs_it)->start < begin_next) {
                // state of observatory at currrent interval
                auto &obs_start = (*obs_it)->start;
                auto &obs_end = (*obs_it)->end;
                auto &obs_state = (*obs_it)->state;
                auto &obs_name = (*obs_it)->obs_name;

                // std::cout << "\nObs interval:";
                // print_time(obs_start, obs_end);

                // Move to beginning of interval
                if (last_trigger < begin_cur) {
                    // schedule.push_back(Interval(last_trigger, begin_cur, sat.name, sat.type, State::IDLE));
                    last_trigger = begin_cur;
                }

                // Observatory was occupied
                if (obs_state == State::BROADCAST) {
                    obs_it++;
                    continue;
                }
                
                // std::cout << "Latst trigger to obs_start:";
                // print_time(last_trigger, obs_start);

                // Handle broadcast
                if (last_trigger < obs_start) {
                    if (last_trigger < end_cur) {
                        // Record before transmission than idle until obs_start
                        auto end_record = obs_start < end_cur ? obs_start : end_cur;

                        auto record_time = std::max(DURATION(last_trigger, end_record), 0.);
                        if (record_time > 0) {
                            auto recorded = sat.record(record_time);
                            schedule.insert(std::make_shared<Interval>(Interval(last_trigger, end_record, sat.name, sat.type, State::RECORDING, recorded)));
                        }
                        // std::cout << "Record before transmission:";
                        // print_time(last_trigger, end_record);
                    }

                    if (sat.capacity > 0) {
                        last_trigger = obs_start;
                        // Transmission whole interval
                        auto broadcast_time = DURATION(last_trigger, obs_end);
                        auto transmitted = sat.broadcast(broadcast_time);
                        schedule.insert(
                            std::make_shared<Interval>(
                                Interval(last_trigger, obs_end,
                                    sat.name, sat.type, State::BROADCAST,
                                    transmitted, obs_name)
                            )
                        );
                        obs_state = State::BROADCAST;
                        // std::cout << "Transmission:";
                        // print_time(last_trigger, obs_end);
                        last_trigger = obs_end;
                    }
                }
                else if (sat.capacity == 0 || last_trigger > obs_end) { // do nothing
                    // std::cout << "Skip\n";
                    obs_it++;
                    continue;
                }
                else { // Begin transmission after start of observatory interval, use only part of it
                    auto broadcast_time = DURATION(last_trigger, obs_end);
                    auto transmitted = sat.broadcast(broadcast_time);
                    schedule.insert(
                        std::make_shared<Interval>(
                            Interval(last_trigger, obs_end,
                                sat.name, sat.type, State::BROADCAST,
                                transmitted, obs_name)
                        )
                    );
                    // std::cout << "Transmission after transmission:";
                    // print_time(last_trigger, obs_end);
                    auto swp = last_trigger;
                    last_trigger = obs_end;
                    // save unused time of interval in smaller interval
                    obs_end = swp;
                }
                
                obs_it++;
            }

            // record last part of in area interval
            if (last_trigger < end_cur) {
                auto start_record = begin_cur > last_trigger ? begin_cur : last_trigger;

                // std::cout << "Record:";
                // print_time(start_record, end_cur);
                auto record_time = std::max(DURATION(start_record, end_cur), 0.);
                if (record_time > 0)
                    schedule.insert(std::make_shared<Interval>(Interval(start_record, end_cur, sat.name, sat.type, State::RECORDING, sat.record(record_time))));
                last_trigger = end_cur;
            }

            // if (sat.capacity - start_capacity > DURATION(begin_cur, end_cur) * sat.recording_speed + 0.1) {
            //     throw std::logic_error("Recorded too much");
            // }
        }
    }
}
#endif