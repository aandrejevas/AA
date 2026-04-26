#pragma once

#include "../metaprogramming/general.hpp"
#include "managed.hpp"
#include "nothrow_allocator.hpp"
#include <memory_resource>



namespace aa {

	// Neturime fixed_array konteinerio, nes talpinimas pabaigos rodyklės nepagreitintų funkcijų, nes
	// greitaveika nenukenčia prie adreso pridėjus skaičių, kuris yra žinomas kompiliavimo metu.
	//
	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0401r6.html
	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0316r0.html
	// https://en.wikipedia.org/wiki/Array_data_structure
	template<not_cref T, class ALLOC = nothrow_allocator<T>>
	struct fixed_array {
		// Member types
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using allocator_type = ALLOC;



		// Element access
		template<class S>
		constexpr auto && operator[](this S && self, const size_type pos) {
			return std::forward_like<S>(self.ptr_to_front[pos]);
		}

		// Gražiname const_pointer, nes taip uždraudžiame keitimą atminties, kuri nepriklauso konteineriui ir todėl, kad negalėtume paduoti gražinamos rodyklės į fixed_vector metodus.
		template<bool CHECKED = true>
		constexpr auto prev_front(this const auto & self) {
			if constexpr (!CHECKED) return negative_infinity; else return		self.data() - 1;
		}

		constexpr auto data(this auto && self) { return				std::addressof(self.front()); }
		constexpr auto begin(this auto && self) { return			std::addressof(self.front()); }

		constexpr auto data_back(this auto && self) { return		std::addressof(self.back()); }

		constexpr const_pointer next_back(this const auto & self) { return		self.data_back() + 1; }
		constexpr const_pointer end(this const auto & self) { return			self.data_back() + 1; }

		constexpr auto data_tail(this auto && self) { return		std::addressof(self.tail()); }

		template<bool CHECKED = true>
		constexpr auto next_tail(this const auto & self) {
			if constexpr (!CHECKED) return positive_infinity; else return		self.data_tail() + 1;
		}

	protected:
		constexpr pointer mut_prev_front(this auto & self) { return				self.data() - 1; }
		constexpr pointer mut_next_back(this auto & self) { return				self.data_back() + 1; }
		constexpr pointer mut_next_tail(this auto & self) { return				self.data_tail() + 1; }

	public:
		template<class S>
		constexpr auto && front(this S && self) {
			return std::forward_like<S>(*self.ptr_to_front);
		}

		template<class S>
		constexpr auto && back(this S && self) {
			return std::forward_like<S>(*self.ptr_to_tail);
		}

		template<class S>
		constexpr auto && tail(this S && self) {
			return std::forward_like<S>(*self.ptr_to_tail);
		}



		// Capacity
		constexpr bool empty(this const auto & self) { return self.data_back() == self.prev_front(); }
		constexpr bool single(this const auto & self) { return self.data_back() == self.data(); }
		constexpr bool full(this const auto & self) { return self.data_back() == self.data_tail(); }

		constexpr size_type size(this const auto & self) { return unsign(self.data_back() - self.prev_front()); }
		constexpr size_type back_index(this const auto & self) { return unsign(self.data_back() - self.data()); }

		constexpr size_type capacity(this const auto & self) { return unsign(self.data_tail() - self.prev_front()); }
		constexpr size_type tail_index(this const auto & self) { return unsign(self.data_tail() - self.data()); }
		// Galėtume neturėti šio metodo, nes gražinamos reikšmės yra pasiekiamos per konstantą tai
		// neprarastume greitaveikos, bet turime juos dėl įsivaizduojamo patogumo.

		// capacity - size
		constexpr size_type space(this const auto & self) { return unsign(self.data_tail() - self.data_back()); }

		static consteval size_type max_size() { return difference_type{numeric_max}; }
		static consteval size_type min_size() { return numeric_min; }

		constexpr bool has_ownership() const {
			return ptr_to_front.has_ownership();
		}



		// Modifiers
		// Neturime iš range kopijavimo metodų ar konstruktorių, nes visų atvejų kopijavimo ir taip neapimtume.
		// Šios klasės specialūs metodai yra ištrinti dėl unique_ptr naudojimo.
		// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3812r0.html
		constexpr fixed_array & operator=(fixed_array && o) & {
			std_r::destroy_at(this);
			return *std_r::construct_at(this, std::move(o));
		}



		// Special member functions
	protected:
		constexpr void destruct(this auto & self) {
			if (self.has_ownership()) {
				std_r::destroy(self);
				self.ptr_to_front.unset_unchecked(self.capacity());
			}
		}

		constexpr fixed_array(std::allocator_arg_t, const size_type size)
			: ptr_to_front{c<ALLOC>().allocate(size)}, ptr_to_tail{(ptr_to_front - 1) + size} {}

	public:
		// Reikia tokio konstruktoriaus, nes gal norės naudotojas vėliau daryti construct_at ant šito objekto. Tuščia būsena taip pat gaunama po move.
		// Inicializuojame su nullptr, nes tai suteikia saugumo ir tas priskyrimas bus išoptimizuotas (unique_ptr inicializacijos negalėtume sustabdyti).
		consteval fixed_array()
			: ptr_to_front{}, ptr_to_tail{default_value} {}

		constexpr fixed_array(fixed_array && o)
			: ptr_to_front{std::move(o.ptr_to_front)}, ptr_to_tail{std::exchange(o.ptr_to_tail, default_value)} {}

		// Nenaudojame std::allocator, nes klasė neturi atminties išskyrimo funkcijų, kurios nemestų išimčių.
		constexpr fixed_array(const size_type size)
			: fixed_array{std::allocator_arg, size}
		{
			std_r::uninitialized_default_construct(*this);
		}

		constexpr ~fixed_array() {
			destruct();
		}



		// Member objects
	protected:
		managed_by_allocator<allocator_type> ptr_to_front;
		// Ne const_pointer, nes ptr_to_front ir taip nebūtų const.
		pointer ptr_to_tail;
	};

}
