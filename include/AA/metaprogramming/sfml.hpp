#pragma once

#include "general.hpp"
#include <SFML/System/Vector2.hpp>
#include <type_traits> // invoke_result_t
#include <concepts> // invocable, same_as



namespace aa {

	template<class T, class U>
	concept storable_locator = storable<T>
		&& std::invocable<const T &, const U &> && std::same_as<std::invoke_result_t<const T &, const U &>, const sf::Vector2f &>;

}
