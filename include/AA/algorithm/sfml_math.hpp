#pragma once

#include "../metaprogramming/general.hpp"
#include "arithmetic.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef> // size_t
#include <cmath> // hypot
#include <utility> // tuple_size, tuple_size_v, tuple_element, tuple_element_t
#include <type_traits> // integral_constant, type_identity, remove_reference_t



namespace std {

	template<aa::arithmetic T>
	struct tuple_size<sf::Vector2<T>> : std::integral_constant<size_t, 2> {};

	template<aa::arithmetic T>
	struct tuple_element<0, sf::Vector2<T>> : std::type_identity<T> {};

	template<aa::arithmetic T>
	struct tuple_element<1, sf::Vector2<T>> : std::type_identity<T> {};

}



namespace sf {

	template<size_t I, aa::instantiation_of<sf::Vector2> V>
	AA_CONSTEXPR std::tuple_element_t<I, std::remove_reference_t<V>> &get(V &&v) {
		if constexpr (!I)	return v.x;
		else				return v.y;
	}



	template<aa::arithmetic T>
	AA_CONSTEXPR sf::Vector2<T> operator-(const sf::Vector2<T> &l, const T r) { return {l.x - r, l.y - r}; }

	template<aa::arithmetic T>
	AA_CONSTEXPR sf::Vector2<T> operator+(const sf::Vector2<T> &l, const T r) { return {l.x + r, l.y + r}; }



	template<aa::arithmetic T>
	AA_CONSTEXPR bool intersects(const sf::Rect<T> &a, const sf::Rect<T> &b) {
		return !((b.left + b.width <= a.left) || (a.left + a.width <= b.left) || (b.top + b.height <= a.top) || (a.top + a.height <= b.top));
	}

	template<aa::arithmetic T>
	AA_CONSTEXPR bool contains(const sf::Rect<T> &a, const sf::Rect<T> &b) {
		return (a.left <= b.left) && (b.left + b.width <= a.left + a.width) && (a.top <= b.top) && (b.top + b.height <= a.top + a.height);
	}

	template<aa::arithmetic T>
	AA_CONSTEXPR bool contains(const sf::Rect<T> &a, const sf::Vector2<T> &b) {
		return (a.left <= b.x) && (b.x < a.left + a.width) && (a.top <= b.y) && (b.y < a.top + a.height);
	}



	template<aa::arithmetic T>
	AA_CONSTEXPR T dist_sq(const sf::Vector2<T> &a) {
		return aa::sq(a.x) + aa::sq(a.y);
	}

	template<aa::arithmetic T>
	AA_CONSTEXPR T dist_sq(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
		return aa::sq(a.x - b.x) + aa::sq(a.y - b.y);
	}



	template<aa::arithmetic T>
	AA_CONSTEXPR T dist(const sf::Vector2<T> &a) {
		return std::hypot(a.x, a.y);
	}

	template<aa::arithmetic T>
	AA_CONSTEXPR T dist(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
		return std::hypot(a.x - b.x, a.y - b.y);
	}



	template<aa::arithmetic T>
	AA_CONSTEXPR T dot(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
		return (a.x * b.x) + (a.y * b.y);
	}

}
