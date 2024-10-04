#include "utils.h"

namespace utils
{
	time_measure::time_measure(int32_t average_count)
		: average_count(average_count)
	{
	}

	void time_measure::start()
	{
		start_time = std::chrono::high_resolution_clock::now();
	}

	double time_measure::finish()
	{
		auto new_value = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();

		if (values.size() >= average_count)
		{
			values_sum -= values.front();
			values.pop_front();
		}
		values.push_back(new_value);
		values_sum += new_value;

		return new_value;
	}

	double time_measure::get_average()
	{
		if (values.size() == 0)
			return 0.0;

		return values_sum / (double)values.size();
	}
}
