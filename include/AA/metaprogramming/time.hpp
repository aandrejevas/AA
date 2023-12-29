#pragma once

#include "general.hpp"
#include <chrono> // is_clock_v



namespace aa {

	template<class T>
	concept clock_like = std::chrono::is_clock_v<T>;

	template<class T>
	concept uses_rep = requires { typename std::remove_reference_t<T>::rep; };

	template<uses_rep T>
	using rep_in_use_t = typename std::remove_reference_t<T>::rep;

	template<class T>
	concept uses_period = requires { typename std::remove_reference_t<T>::period; };

	template<uses_period T>
	using period_in_use_t = typename std::remove_reference_t<T>::period;

	template<class T>
	concept uses_duration = requires { typename std::remove_reference_t<T>::duration; };

	template<uses_duration T>
	using duration_in_use_t = typename std::remove_reference_t<T>::duration;

	template<class T>
	concept uses_time_point = requires { typename std::remove_reference_t<T>::time_point; };

	template<uses_time_point T>
	using time_point_in_use_t = typename std::remove_reference_t<T>::time_point;

}
