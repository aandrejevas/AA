#pragma once

#include "../metaprogramming/general.hpp"
#include "managed.hpp"
#include <memory_resource>



namespace aa {

	// Neturime fixed_array konteinerio, nes talpinimas pabaigos rodyklės nepagreitintų funkcijų, nes
	// greitaveika nenukenčia prie adreso pridėjus skaičių, kuris yra žinomas kompiliavimo metu.
	//
	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0401r6.html
	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0316r0.html
	// https://en.wikipedia.org/wiki/Array_data_structure
	template<not_cref T, cref_invocable<T *> auto DELETER = default_v<std::default_delete<T[]>>>
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
		using deleter_type = const_t<DELETER>;



		// Element access
		template<class S>
		constexpr auto && operator[](this S && self, const size_type pos) {
			return std::forward_like<S>(self.ptr_to_front[pos]);
		}

		constexpr auto data(this auto && self) {
			return std::addressof(self.front());
		}

		constexpr auto back_data(this auto && self) {
			return std::addressof(self.back());
		}

		constexpr auto last_data(this auto && self) {
			return std::addressof(self.last());
		}

		template<class S>
		constexpr auto && front(this S && self) {
			return std::forward_like<S>(*self.ptr_to_front);
		}

		template<class S>
		constexpr auto && back(this S && self) {
			return std::forward_like<S>(*self.ptr_to_last);
		}

		template<class S>
		constexpr auto && last(this S && self) {
			return std::forward_like<S>(*self.ptr_to_last);
		}



		// Iterators
		constexpr auto begin(this auto && self) { return self.data(); }

		// Gražiname const_iterator, nes taip uždraudžiame keitimą atminties, kuri nepriklauso konteineriui
		// ir todėl, kad negalėtume paduoti gražinamos rodyklės į fixed_vector metodus.
		constexpr const_iterator end(this const auto & self) { return self.back_data() + 1; }

		constexpr auto rbegin(this auto && self) { return self.back_data(); }

		constexpr const_iterator rend(this const auto & self) { return self.data() - 1; }



		// Capacity
		constexpr bool empty(this const auto & self) { return self.back_data() == self.rend(); }
		constexpr bool single(this const auto & self) { return self.back_data() == self.data(); }
		constexpr bool full(this const auto & self) { return self.back_data() == self.last_data(); }

		constexpr size_type size(this const auto & self) { return unsign(self.back_data() - self.rend()); }
		constexpr size_type back_index(this const auto & self) { return unsign(self.back_data() - self.data()); }

		constexpr size_type capacity(this const auto & self) { return unsign(self.last_data() - self.rend()); }
		constexpr size_type last_index(this const auto & self) { return unsign(self.last_data() - self.data()); }
		// Galėtume neturėti šio metodo, nes gražinamos reikšmės yra pasiekiamos per konstantą tai
		// neprarastume greitaveikos, bet turime juos dėl įsivaizduojamo patogumo.

		// capacity - size
		constexpr size_type space(this const auto & self) { return unsign(self.last_data() - self.back_data()); }

		static constexpr size_type min_size = numeric_min, max_size = numeric_max_v<difference_type>;

		constexpr bool has_ownership() const {
			return ptr_to_front.has_ownership();
		}



		// Modifiers
		// Neturime iš range kopijavimo metodų ar konstruktorių, nes visų atvejų kopijavimo ir taip neapimtume.
		// Šios klasės specialūs metodai yra ištrinti dėl unique_ptr naudojimo.
		constexpr fixed_array & operator=(fixed_array && o) & {
			ptr_to_front = std::move(o.ptr_to_front);
			ptr_to_last = std::exchange(o.ptr_to_last, default_value);
			return *this;
		}



		// Special member functions
	private:
		constexpr fixed_array(const pointer p, const size_type size)
			: ptr_to_front{p}, ptr_to_last{(p - 1) + size} {}

	public:
		// Reikia tokio konstruktoriaus, nes gal norės naudotojas vėliau daryti construct_at ant šito objekto. Tuščia būsena taip pat gaunama po move.
		// Inicializuojame su nullptr, nes tai suteikia saugumo ir tas priskyrimas bus išoptimizuotas (unique_ptr inicializacijos negalėtume sustabdyti).
		constexpr fixed_array()
			: ptr_to_front{}, ptr_to_last{default_value} {}

		constexpr fixed_array(fixed_array && o)
			: ptr_to_front{std::move(o.ptr_to_front)}, ptr_to_last{std::exchange(o.ptr_to_last, default_value)} {}

		// Nenaudojame std::allocator, nes klasė neturi atminties išskyrimo funkcijų, kurios nemestų išimčių.
		constexpr fixed_array(const size_type size) requires (std::same_as<deleter_type, std::default_delete<value_type[]>>)
			: fixed_array{new(std::nothrow) value_type[size], size} {}

		// FIXME: implement a null memory_resource which does not throw
		constexpr fixed_array(const size_type size, std::pmr::monotonic_buffer_resource & r) requires (std::same_as<deleter_type, std::identity>)
			: fixed_array{std::pmr::polymorphic_allocator<value_type>{&r}.allocate(size), size} {}

		constexpr fixed_array(const size_type size, const pointer p) requires (std::same_as<deleter_type, std::identity>)
			: fixed_array{p, size} {}



		// Member objects
	protected:
		managed<pointer, DELETER> ptr_to_front;
		// Ne const_pointer, nes ptr_to_front ir taip nebūtų const.
		pointer ptr_to_last;
	};

	namespace pmr {
		template<class T>
		using fixed_array = aa::fixed_array<T, default_v<std::identity>>;
	}

}
