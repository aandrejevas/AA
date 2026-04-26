#pragma once

#include "../metaprogramming/general.hpp"
#include "../container/fixed_array.hpp"



namespace aa {

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0843r14.html
	// https://en.wikipedia.org/wiki/Dynamic_array
	// Negali būti tuple like, nes kinta kiek masyvo elementų yra "naudojama".
	template<not_cref T, class ALLOC = nothrow_allocator<T>>
	struct fixed_vector : fixed_array<T, ALLOC> {
		// Member types
		using base_type = fixed_array<T, ALLOC>;
		using typename base_type::value_type, typename base_type::size_type, typename base_type::difference_type,
			typename base_type::reference, typename base_type::const_reference,
			typename base_type::pointer, typename base_type::const_pointer,
			typename base_type::iterator, typename base_type::const_iterator,
			typename base_type::allocator_type;



		// Element access
		template<class S>
		constexpr auto && back(this S && self) {
			return std::forward_like<S>(*self.ptr_to_back);
		}



		// Neturime insert metodų, nes vietoje jų be problemų galima emplace metodus naudoti.
		// Modifiers
		constexpr iterator clear() {
			return ptr_to_back = std_r::destroy(std::views::reverse(*this)).base() - 1;
		}

#pragma region // modify one
		// modify one back
		// Neišeina emplace_back ir push_back apjungti, nes įsivaizduokime tokį scenarijų, visi masyvo elementai
		// pradžioje sukonstruojami ir mes norime tiesiog rodyklę pastumti, emplace_back iš naujo sukonstruotų elementą.
		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace_back(A &&... args) {
			return ptr_to_back = std_r::construct_at(this->mut_next_back(), std::forward<A>(args)...);
		}

		// modify one anywhere
		// Neteisinga būtų grąžinti ir paduoti const_iterator, nes sumažintume pateisinamą prieinamumą.
		// Nors funkcija teisingai veiktų ir padavus end(), būtų neteisinga end() paduoti todėl ir tai uždraudžiame.
		// template<class... A>
		// 	requires (std::constructible_from<value_type, A...>)
		// constexpr iterator emplace(const iterator pos, A &&... args) {
		// 	return std_r::construct_at(push(pos), std::forward<A>(args)...);
		// }

		// fast-modify one anywhere
		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator fast_emplace(const iterator pos, A &&... args) {
			emplace_back(std::move(*pos));
			*pos = value_type{std::forward<A>(args)...};
			return pos;
		}
#pragma endregion

#pragma region // modify many back
		template<bool CHECKED = true>
		constexpr iterator pop_back(const size_type count = 1) {
			return ptr_to_back = std_r::destroy(std::reverse_iterator{this->mut_next_back()}, std::reverse_iterator{
				get_prev(this->next_back(), sign(count), this->template prev_front<CHECKED>())}).base() - 1;
		}

		template<bool CHECKED = true>
		constexpr iterator push_back(const size_type count = 1) {
			return ptr_to_back = std_r::uninitialized_default_construct(this->mut_next_back(),
				get_next(this->next_back(), sign(count), this->template next_tail<CHECKED>())) - 1;
		}

		template<bool CHECKED = true, std_r::input_range R>
		constexpr iterator emplace_back_range(R && r) {
			return ptr_to_back = std_r::uninitialized_copy(r,
				std_r::subrange{this->mut_next_back(), this->template next_tail<CHECKED>()}).out - 1;
		}
#pragma endregion

#pragma region // modify many anywhere
		// Patikrinau, toks pat assembly kodas sugeneruojamas šios funkcijos ir funkcijos, kuri stumia tik vieną, tai kitos funkcijos nereikia.
		// constexpr iterator push(const iterator pos, const size_type count = 1) {
		// 	const size_type size = unsign(this->next_back() - pos);
		// 	if (size == 0) {
		// 		push_back(count);

		// 	} else if (count > size) {
		// 		std_r::uninitialized_default_construct(this->next_back(), pos + count);
		// 		ptr_to_back = std_r::uninitialized_move(
		// 			pos, this->next_back(),
		// 			pos + count, this->next_back() + count).out - 1;

		// 	} else if (count <= size) {
		// 		std_r::move_backward(pos, this->next_back() - count, this->mut_next_back());
		// 		ptr_to_back = std_r::uninitialized_move(
		// 			this->mut_next_back() - count, this->next_back(),
		// 			this->mut_next_back(), this->next_back() + count).out - 1;
		// 	}
		// 	return pos;
		// }

		constexpr iterator pop(const iterator pos, const size_type count = 1) {
			std_r::move(pos + count, this->next_back(), pos);
			pop_back(count);
			return pos;
		}

		// template<sized_input_range R>
		// constexpr iterator emplace_range(const iterator pos, R && r) {
		// 	std_r::copy(r, push(pos, std_r::size(r)));
		// 	return pos;
		// }
#pragma endregion

#pragma region // fast-modify many anywhere
		constexpr iterator fast_pop(const iterator pos, const size_type count = 1) {
			std_r::move(this->mut_next_back() - count, this->next_back(), pos);
			pop_back(count);
			return pos;
		}

		// template<sized_input_range R>
		// constexpr iterator fast_emplace_range(const iterator pos, R && r) {
		// 	emplace_back_range(std::views::as_rvalue(std::span{pos, std_r::size(r)}));
		// 	std_r::copy(r, pos);
		// 	return pos;
		// }
#pragma endregion

		// assign
		// template<class... A>
		// 	requires (std::constructible_from<value_type, A...>)
		// constexpr iterator assign(A &&... args) {
		// 	return std_r::construct_at(resize(this->data()), std::forward<A>(args)...);
		// }

		// template<std_r::input_range R>
		// constexpr iterator assign_range(R && r) {
		// 	return resize(std_r::copy(r, this->data()).out - 1);
		// }

		// template<std_r::input_range R>
		// constexpr fixed_vector & operator=(R && r) & {
		// 	return (assign_range(r), *this);
		// }

		constexpr fixed_vector & operator=(fixed_vector && a) & {
			std_r::destroy_at(this);
			return *std_r::construct_at(this, std::move(a));
		}



		// Special member functions
		// Nedarome = default, nes konstruktorius vis tiek nebus trivial,
		// nes klasė turi kintamųjų su numatytais inicializatoriais.
		consteval fixed_vector()
			: base_type{}, ptr_to_back{default_value} {}

		constexpr fixed_vector(fixed_vector && a)
			: base_type{std::move(a)}, ptr_to_back{std::exchange(a.ptr_to_back, default_value)} {}

		// Negalime turėti konstruktoriaus, kuris priimtų rodyklę, nes
		// tik po konstruktoriaus įvykdymo galima gauti rodykles.
		template<bool CHECKED = true>
		constexpr fixed_vector(const size_type s, const size_type count = 0, constant<CHECKED> = default_value)
			: base_type{std::allocator_arg, s}
		{
			ptr_to_back = std_r::uninitialized_default_construct(this->data(),
				get_next(std::as_const(*this).data(), sign(count), this->template next_tail<CHECKED>())) - 1;
		}

		// Nereikia konstruktoriaus, kuriame būtų naudojama fill, nes šį funkcionalumą
		// galima simuliuoti naudojant šį konstruktorių pavyzdžiui su repeat_view.
		template<bool CHECKED = true, std_r::input_range R>
		constexpr fixed_vector(const size_type s, R && r, constant<CHECKED> = default_value)
			: base_type{std::allocator_arg, s}
		{
			ptr_to_back = std_r::uninitialized_copy(r,
				std_r::subrange{this->data(), this->template next_tail<CHECKED>()}).out - 1;
		}

		constexpr ~fixed_vector() {
			this->destruct();
		}



		// Member objects
	protected:
		pointer ptr_to_back;
	};

}
