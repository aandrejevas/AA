#pragma once

#include "../metaprogramming/general.hpp"
#include "../container/fixed_array.hpp"



namespace aa {

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0843r14.html
	// https://en.wikipedia.org/wiki/Dynamic_array
	// Negali būti tuple like, nes kinta kiek masyvo elementų yra "naudojama".
	template<std::destructible T, cref_invocable<T *> auto DELETER = default_v<std::default_delete<T[]>>>
	struct fixed_vector : fixed_array<T, DELETER> {
		// Member types
		using base_type = fixed_array<T, DELETER>;
		using typename base_type::value_type, typename base_type::size_type, typename base_type::difference_type,
			typename base_type::reference, typename base_type::const_reference,
			typename base_type::pointer, typename base_type::const_pointer,
			typename base_type::iterator, typename base_type::const_iterator,
			typename base_type::deleter_type;



		// Element access
		template<class S>
		constexpr auto && back(this S && self) {
			return std::forward_like<S>(*self.ptr_to_back);
		}



		// Neturime insert metodų, nes vietoje jų be problemų galima emplace metodus naudoti.
		// Modifiers
		constexpr iterator clear() { return resize(const_cast<iterator>(this->rend())); }

		constexpr iterator resize(const size_type count) { return resize(const_cast<iterator>(this->rend()) + count); }
		constexpr iterator resize(const iterator pos) { return ptr_to_back = pos; }

#pragma region // modify one
		// modify one back
		// Neišeina emplace_back ir push_back apjungti, nes įsivaizduokime tokį scenarijų, visi masyvo elementai
		// pradžioje sukonstruojami ir mes norime tiesiog rodyklę pastumti, emplace_back iš naujo sukonstruotų elementą.
		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace_back(A &&... args) {
			return std::ranges::construct_at(push_back(), std::forward<A>(args)...);
		}

		// modify one anywhere
		// Neteisinga būtų grąžinti ir paduoti const_iterator, nes sumažintume pateisinamą prieinamumą.
		// Nors funkcija teisingai veiktų ir padavus end(), būtų neteisinga end() paduoti todėl ir tai uždraudžiame.
		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace(const iterator pos, A &&... args) {
			return std::ranges::construct_at(push(pos), std::forward<A>(args)...);
		}

		// fast-modify one anywhere
		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator fast_emplace(const iterator pos, A &&... args) {
			emplace_back(std::move(*pos));
			return std::ranges::construct_at(pos, std::forward<A>(args)...);
		}
#pragma endregion

#pragma region // modify many back
		constexpr iterator pop_back(const size_type count = 1) { return ptr_to_back -= count; }
		constexpr iterator push_back(const size_type count = 1) { return ptr_to_back += count; }

		template<std::ranges::input_range R>
		constexpr iterator emplace_back_range(R && r) {
			return resize(std::ranges::copy(r, const_cast<iterator>(this->end())).out - 1);
		}
#pragma endregion

#pragma region // modify many anywhere
		// Patikrinau, toks pat assembly kodas sugeneruojamas šios funkcijos ir funkcijos, kuri stumia tik vieną, tai kitos funkcijos nereikia.
		constexpr iterator push(const iterator pos, const size_type count = 1) {
			std::ranges::move_backward(pos, this->end(), const_cast<iterator>(this->end()) + count);
			push_back(count);
			return pos;
		}

		constexpr iterator pop(const iterator pos, const size_type count = 1) {
			std::ranges::move(pos + count, this->end(), pos);
			pop_back(count);
			return pos;
		}

		template<sized_input_range R>
		constexpr iterator emplace_range(const iterator pos, R && r) {
			std::ranges::copy(r, push(pos, std::ranges::size(r)));
			return pos;
		}
#pragma endregion

#pragma region // fast-modify many anywhere
		constexpr iterator fast_pop(const iterator pos, const size_type count = 1) {
			std::ranges::move(const_cast<iterator>(this->end()) - count, this->end(), pos);
			pop_back(count);
			return pos;
		}

		template<sized_input_range R>
		constexpr iterator fast_emplace_range(const iterator pos, R && r) {
			emplace_back_range(std::views::as_rvalue(std::span{pos, std::ranges::size(r)}));
			std::ranges::copy(r, pos);
			return pos;
		}
#pragma endregion

		// assign
		template<std::ranges::input_range R>
		constexpr iterator assign_range(R && r) {
			return resize(std::ranges::copy(r, this->data()).out - 1);
		}

		template<std::ranges::input_range R>
		constexpr fixed_vector & operator=(R && r) & {
			return (assign_range(r), *this);
		}

		constexpr fixed_vector & operator=(fixed_vector && a) & {
			cast<base_type &>(*this) = std::move(a);
			ptr_to_back = std::exchange(a.ptr_to_back, default_value);
			return *this;
		}



		// Special member functions
		constexpr fixed_vector()
			: base_type{}, ptr_to_back{default_value} {}

		constexpr fixed_vector(fixed_vector && a)
			: base_type{std::move(a)}, ptr_to_back{std::exchange(a.ptr_to_back, default_value)} {}

		// Nedarome = default, nes konstruktorius vis tiek nebus trivial,
		// nes klasė turi kintamųjų su numatytais inicializatoriais.
		constexpr fixed_vector(base_type && a)
			: base_type{std::move(a)} { clear(); }

		// Negalime turėti konstruktoriaus, kuris priimtų rodyklę, nes
		// tik po konstruktoriaus įvykdymo galima gauti rodykles.
		constexpr fixed_vector(base_type && a, const const_t<std::placeholders::_1>)
			: base_type{std::move(a)} { resize(this->data()); }

		constexpr fixed_vector(base_type && a, const const_t<std::placeholders::_2>)
			: base_type{std::move(a)} { resize(this->max_data()); }

		constexpr fixed_vector(base_type && a, const size_type count)
			: base_type{std::move(a)} { resize(count); }

		// Nereikia konstruktoriaus, kuriame būtų naudojama fill, nes šį funkcionalumą
		// galima simuliuoti naudojant šį konstruktorių pavyzdžiui su repeat_view.
		template<std::ranges::input_range R>
		constexpr fixed_vector(base_type && a, R && r)
			: base_type{std::move(a)} { assign_range(r); }



		// Member objects
	protected:
		pointer ptr_to_back;
	};

	namespace pmr {
		template<class T>
		using fixed_vector = aa::fixed_vector<T, default_v<std::identity>>;
	}

}
