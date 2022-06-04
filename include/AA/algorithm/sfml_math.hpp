#pragma once

#include "../metaprogramming/general.hpp"
#include "arithmetic.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath> // hypot



namespace aa {

	template<arithmetic T>
	inline constexpr bool intersects(const sf::Rect<T> &a, const sf::Rect<T> &b) {
		return !((b.left + b.width <= a.left) || (a.left + a.width <= b.left) || (b.top + b.height <= a.top) || (a.top + a.height <= b.top));
	}

	template<arithmetic T>
	inline constexpr bool contains(const sf::Rect<T> &a, const sf::Rect<T> &b) {
		return (a.left <= b.left) && (b.left + b.width <= a.left + a.width) && (a.top <= b.top) && (b.top + b.height <= a.top + a.height);
	}

	template<arithmetic T>
	inline constexpr bool contains(const sf::Rect<T> &a, const sf::Vector2<T> &b) {
		return (a.left <= b.x) && (b.x < a.left + a.width) && (a.top <= b.y) && (b.y < a.top + a.height);
	}



	template<arithmetic T>
	inline constexpr T dist_sq(const sf::Vector2<T> &a) {
		return sq(a.x) + sq(a.y);
	}

	template<arithmetic T>
	inline constexpr T dist_sq(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
		return sq(a.x - b.x) + sq(a.y - b.y);
	}



	template<arithmetic T>
	inline T dist(const sf::Vector2<T> &a) {
		return std::hypot(a.x, a.y);
	}

	template<arithmetic T>
	inline T dist(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
		return std::hypot(a.x - b.x, a.y - b.y);
	}



	template<arithmetic T>
	inline constexpr T dot(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
		return (a.x * b.x) + (a.y * b.y);
	}

}
