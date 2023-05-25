
#include <stack>
#include <date.h>

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
		events_grid.reserve(sat.second.ints_in_area.size());
		for (auto &int_area: sat.second.ints_in_area) {
			events_grid.push_back({int_area->start, int_area->info[0], 1});
			events_grid.push_back({int_area->end, int_area->info[0], -1});
		}

		events_grid.reserve(sat.second.ints_observatories.size());
		for (auto &int_sat: sat.second.ints_observatories) {
			events_grid.push_back({int_sat->start, int_sat->info[0], 1});
			events_grid.push_back({int_sat->end, int_sat->info[0], -1});
		}
	}

	auto tp_cmp = [](const Event &a, const Event &b) { return a.tp < b.tp; };
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
			info_stack.push_back(event_info);
			if (info_stack.size() > 1) {
				Interval i(last_trigger, event_tp, info_stack);
				great_plan.insert(std::make_shared<Interval>(i));
			}
		}
		else { // end
			great_plan.insert(std::make_shared<Interval>(Interval(last_trigger, event_tp, info_stack)));
			info_stack.erase(std::find(info_stack.begin(), info_stack.end(), event.info));
		}

		last_trigger = event_tp;
	}

	return std::move(great_plan);
}