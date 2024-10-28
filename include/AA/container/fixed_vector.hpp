#pragma once

#include "../metaprogramming/general.hpp"
#include "../container/fixed_array.hpp"



namespace aa {

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0843r14.html
	// https://en.wikipedia.org/wiki/Dynamic_array
	// Negali būti tuple like, nes kinta kiek masyvo elementų yra "naudojama".
	template<class T, ref_invocable<T *> D = std::default_delete<T[]>>
	struct fixed_vector : fixed_array<T, D> {
		// Member types
		using base_type = fixed_array<T, D>;
		using typename base_type::value_type, typename base_type::size_type, typename base_type::difference_type,
			typename base_type::reference, typename base_type::const_reference,
			typename base_type::pointer, typename base_type::const_pointer,
			typename base_type::iterator, typename base_type::const_iterator,
			typename base_type::deleter_type;



		// Element access
		template<class S>
		constexpr std::add_pointer_t<forward_like_t<S, value_type>> back_data(this S &&self) {
			return self._back_data;
		}



		// Anksčiau buvo, bet dabar nebėra elemento įdėjimo metodų overload'ų, kurie priimtų
		// value_type&&, nes move semantics neturi prasmės trivially copyable tipams.
		// Modifiers
		constexpr iterator clear() { return resize(const_cast<iterator>(this->rend())); }

		constexpr iterator resize(const size_type count) { return resize(const_cast<iterator>(this->rend()) + count); }
		constexpr iterator resize(const iterator pos) { return _back_data = pos; }

		constexpr iterator pop_back() { return --_back_data; }
		constexpr iterator push_back() { return ++_back_data; }
		constexpr iterator pop_back_alt() { return _back_data--; }
		constexpr iterator push_back_alt() { return _back_data++; }

		constexpr iterator pop_back(const size_type count) { return _back_data -= count; }
		constexpr iterator push_back(const size_type count) { return _back_data += count; }

		// Neišeina emplace_back ir push_back apjungti, nes įsivaizduokime tokį scenarijų, visi masyvo elementai
		// pradžioje sukonstruojami ir mes norime tiesiog rodyklę pastumti, emplace_back iš naujo sukonstruotų elementą.
		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace_back(A&&... args) {
			return std::ranges::construct_at(push_back(), std::forward<A>(args)...);
		}

		template<assignable_to<reference> V>
		constexpr iterator insert_back(V &&value) {
			return std::addressof(*push_back() = std::forward<V>(value));
		}

		constexpr iterator push(const iterator pos) {
			push_back();
			std::ranges::copy_backward(pos, _back_data, const_cast<iterator>(this->end()));
			return pos;
		}

		// Neteisinga būtų grąžinti ir paduoti const_iterator, nes sumažintume pateisinamą prieinamumą.
		// Nors funkcija teisingai veiktų ir padavus end(), būtų neteisinga end() paduoti todėl ir tai uždraudžiame.
		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace(const iterator pos, A&&... args) {
			return std::ranges::construct_at(push(pos), std::forward<A>(args)...);
		}

		template<assignable_to<reference> V>
		constexpr iterator insert(const iterator pos, V &&value) {
			return std::addressof(*push(pos) = std::forward<V>(value));
		}

		constexpr iterator erase(const iterator pos) {
			std::ranges::copy(pos + 1, this->end(), pos);
			pop_back();
			return pos;
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator fast_emplace(const iterator pos, A&&... args) {
			insert_back(*pos);
			return std::ranges::construct_at(pos, std::forward<A>(args)...);
		}

		template<assignable_to<reference> V>
		constexpr iterator fast_insert(const iterator pos, V &&value) {
			insert_back(*pos);
			return std::addressof(*pos = std::forward<V>(value));
		}

		constexpr iterator fast_erase(const iterator pos) {
			return std::addressof(*pos = *pop_back_alt());
		}

		template<std::ranges::input_range R>
		constexpr iterator append_range(R &&r) {
			return resize(std::ranges::copy(r, const_cast<iterator>(this->end())).out - 1);
		}

		template<std::ranges::input_range R>
		constexpr iterator assign_range(R &&r) {
			return resize(std::ranges::copy(r, this->data()).out - 1);
		}

		template<std::ranges::input_range R>
		constexpr fixed_vector &operator=(R &&r) {
			return (assign_range(r), *this);
		}

		constexpr fixed_vector &operator=(fixed_vector &&a) {
			cast<base_type &>(*this) = std::move(a);
			_back_data = std::exchange(a._back_data, nullptr);
			return *this;
		}



		// Special member functions
		constexpr fixed_vector(fixed_vector &&a)
			: base_type{std::move(a)}, _back_data{std::exchange(a._back_data, nullptr)} {}

		// Nedarome = default, nes konstruktorius vis tiek nebus trivial,
		// nes klasė turi kintamųjų su numatytais inicializatoriais.
		constexpr fixed_vector(base_type &&a = default_value)
			: base_type{std::move(a)} { clear(); }

		// Negalime turėti konstruktoriaus, kuris priimtų rodyklę, nes
		// tik po konstruktoriaus įvykdymo galima gauti rodykles.
		constexpr fixed_vector(base_type &&a, const const_t<std::placeholders::_1>)
			: base_type{std::move(a)} { resize(this->data()); }

		constexpr fixed_vector(base_type &&a, const const_t<std::placeholders::_2>)
			: base_type{std::move(a)} { resize(this->max_data()); }

		constexpr fixed_vector(base_type &&a, const size_type count)
			: base_type{std::move(a)} { resize(count); }

		// Nereikia konstruktoriaus, kuriame būtų naudojama fill, nes šį funkcionalumą
		// galima simuliuoti naudojant šį konstruktorių pavyzdžiui su repeat_view.
		template<std::ranges::input_range R>
		constexpr fixed_vector(base_type &&a, R &&r)
			: base_type{std::move(a)} { assign_range(r); }



		// Member objects
	protected:
		pointer _back_data;
	};

	namespace pmr {
		template<class T>
		using fixed_vector = aa::fixed_vector<T, std::identity>;
	}

}
