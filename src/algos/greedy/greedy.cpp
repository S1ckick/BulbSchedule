#include "../algos.h"
#include <date.h>

void print_time(const timepoint &a, const timepoint &b) {
    std::istringstream start_date("1/Jun/2027 00:00:00.000");
    timepoint tp_start;
    start_date >> date::parse("%d/%b/%Y %T", tp_start);
    std::cout << (std::chrono::duration<double, std::milli>(a - tp_start) * std::chrono::milliseconds::period::num /
        std::chrono::milliseconds::period::den).count()
                << " " << (std::chrono::duration<double, std::milli>(b - tp_start) * std::chrono::milliseconds::period::num /
        std::chrono::milliseconds::period::den).count()
                << '\n';
}

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
