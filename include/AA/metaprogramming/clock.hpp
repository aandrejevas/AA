#pragma once

#include <chrono> // is_clock_v



namespace aa {

	template<class T>
	concept clock = std::chrono::is_clock_v<T>;

}
