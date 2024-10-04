#pragma once
#include <chrono>
#include <deque>

namespace utils
{
	struct time_measure
	{
		time_measure(int32_t average_count);

		void start();
		double finish();

		double get_average();

	private:
		const int32_t average_count;
		std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
		std::deque<double> values;
		double values_sum = 0.0;
	};
}
