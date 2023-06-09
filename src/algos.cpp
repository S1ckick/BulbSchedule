
#include <stack>

#include "algos.h"

using namespace date;

struct Event {
	timepoint tp;
	IntervalInfo info;
	int direction;
};

VecSegment algos::great_plan(const Satellites &sats) {
	std::vector<Event> events_grid; // vector bcs timepoints are not unique :(

        for (int isat = 1; isat <= SAT_NUM; isat++)
        {
            const Satellite &sat = sats[isat];
            
            events_grid.reserve(events_grid.size() + sat.ints_in_area.size());
            for (auto &int_area: sat.ints_in_area) {
                    events_grid.push_back({int_area.start, int_area.info, 1});
                    events_grid.push_back({int_area.end, int_area.info, -1});
            }

            events_grid.reserve(events_grid.size() + sat.ints_observatories.size());
            for (auto &int_sat: sat.ints_observatories) {
                    events_grid.push_back({int_sat.start, int_sat.info, 1});
                    events_grid.push_back({int_sat.end, int_sat.info, -1});
            }
	}

	auto tp_cmp = [](const Event &a, const Event &b) {
		if (a.tp == b.tp)
			return a.direction < b.direction; // handle end points before start points
		return a.tp < b.tp; 
	};
	std::sort(events_grid.begin(), events_grid.end(), tp_cmp);

	std::vector<IntervalInfo> info_stack;

	VecSegment great_plan;

	timepoint last_trigger;
	for (auto &event: events_grid) {
		auto &event_tp = event.tp;
		auto &event_info = event.info;
		auto &event_dir = event.direction;

		//std::cout << event_tp << "\n";

		if (event_dir == 1) { // start
			if (!info_stack.empty()) {
				if (last_trigger != event_tp) {
					Segment i({last_trigger, event_tp, info_stack});
					great_plan.push_back(i);
				}
			}
			info_stack.push_back(event_info);
		}
		else { // end
			if (last_trigger != event_tp) {
				Segment i({last_trigger, event_tp, info_stack});
				great_plan.push_back(i);
			}
			info_stack.erase(std::find(info_stack.begin(), info_stack.end(), event.info));
		}

		last_trigger = event_tp;
	}

	return std::move(great_plan);
}

void algos::add2schedule(const timepoint &start, const timepoint &end, const IntervalInfo &info, Satellite &cur_sat) 
{
	auto int_dur = DURATION(start, end);
	Interval new_interval(start, end, info);
	if (info.state == State::RECORDING)
		cur_sat.record(int_dur);
	else if (info.state == State::TRANSMISSION)
		cur_sat.transmission(int_dur);

	// its pointer so it affects observatory interval too
	if (!cur_sat.full_schedule.empty())
	{
		auto &last_sat = cur_sat.full_schedule[cur_sat.full_schedule.size() - 1];
		if (new_interval.start == last_sat.end && new_interval.info == last_sat.info) {
			last_sat += new_interval;
			return;
		}
	}

	cur_sat.full_schedule.push_back(new_interval);
	if (new_interval.info.state == State::TRANSMISSION) {
		if (info.obs_name == 0)
			throw std::logic_error("Pass an observatory to add2schedule to add broadcasting interval");
	}
}