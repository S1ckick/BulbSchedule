
#include <stack>

#include "algos.h"

using namespace date;

struct Event {
	timepoint tp;
	std::shared_ptr<IntervalInfo> info;
	int direction;
};

Schedule algos::great_plan(const Satellites &sats) {
	std::vector<Event> events_grid; // vector bcs timepoints are not unique :(

	for (auto &sat: sats) {
		events_grid.reserve(events_grid.size() + sat.second.ints_in_area.size());
		for (auto &int_area: sat.second.ints_in_area) {
			events_grid.push_back({int_area->start, int_area->info[0], 1});
			events_grid.push_back({int_area->end, int_area->info[0], -1});
		}

		events_grid.reserve(events_grid.size() + sat.second.ints_observatories.size());
		for (auto &int_sat: sat.second.ints_observatories) {
			events_grid.push_back({int_sat->start, int_sat->info[0], 1});
			events_grid.push_back({int_sat->end, int_sat->info[0], -1});
		}
	}

	auto tp_cmp = [](const Event &a, const Event &b) {
		if (a.tp == b.tp)
			return a.direction < b.direction; // handle end points before start points
		return a.tp < b.tp; 
	};
	std::sort(events_grid.begin(), events_grid.end(), tp_cmp);

	std::vector<std::shared_ptr<IntervalInfo>> info_stack;

	Schedule great_plan;

	timepoint last_trigger;
	for (auto &event: events_grid) {
		auto &event_tp = event.tp;
		auto &event_info = event.info;
		auto &event_dir = event.direction;

		//std::cout << event_tp << "\n";

		if (event_dir == 1) { // start
			if (!info_stack.empty()) {
				if (last_trigger != event_tp) {
					Interval i(last_trigger, event_tp, info_stack);
					great_plan.insert(std::make_shared<Interval>(i));
				}
			}
			info_stack.push_back(event_info);
		}
		else { // end
			if (last_trigger != event_tp)
				great_plan.insert(std::make_shared<Interval>(Interval(last_trigger, event_tp, info_stack)));
			info_stack.erase(std::find(info_stack.begin(), info_stack.end(), event.info));
		}

		last_trigger = event_tp;
	}

	return std::move(great_plan);
}

Observatory empty_obs({"", {}, {}});

void algos::add2schedule(std::shared_ptr<Interval> &interval, Satellite &cur_sat, Observatory &cur_obs) {
	if (interval->info[0]->state == State::RECORDING)
		interval->capacity_change = cur_sat.record(interval->duration);
	else if (interval->info[0]->state == State::TRANSMISSION)
		interval->capacity_change = cur_sat.transmission(interval->duration);

	// its pointer so it affects observatory interval too
	if (!cur_sat.full_schedule.empty())
	{
		auto last_sat = cur_sat.full_schedule[cur_sat.full_schedule.size() - 1];
		if (interval->start == last_sat->end && interval->info[0] == last_sat->info[0]) {
			(*last_sat) += (*interval);
			return;
		}
	}

	else if (interval->info[0]->state == State::TRANSMISSION) {
		if (cur_obs.name.empty())
			throw std::logic_error("Pass observatoty in add2schedule to add broadcsting interval");
		cur_obs.full_schedule.push_back(interval);
	}
	cur_sat.full_schedule.push_back(interval);
}