#pragma once

#include "../metaprogramming/general.hpp"
#include "arithmetic.hpp"
#include <SFML/System/Vector2.hpp>
#include <cmath> // hypot



namespace aa {

	template<arithmetic T>
	inline T dist_sq(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
		return sq(a.x - b.x) + sq(a.y - b.y);
	}

	template<arithmetic T>
	inline T dist(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
		return std::hypot(a.x - b.x, a.y - b.y);
	}

	template<arithmetic T>
	inline T dot(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
		return (a.x * b.x) + (a.y * b.y);
	}

}
