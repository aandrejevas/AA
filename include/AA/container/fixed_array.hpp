#pragma once

#include "../metaprogramming/general.hpp"
#include <memory>
#include <memory_resource>



namespace aa {

	// Neturime fixed_array konteinerio, nes talpinimas pabaigos rodyklės nepagreitintų funkcijų, nes
	// greitaveika nenukenčia prie adreso pridėjus skaičių, kuris yra žinomas kompiliavimo metu.

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0316r0.html
	// https://en.wikipedia.org/wiki/Array_data_structure
	template<class T, class D = std::default_delete<T[]>>
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
		using deleter_type = D;



		// Element access
		template<class S>
		constexpr forward_like_t<S, value_type> operator[](this S &&self, const size_type pos) {
			return std::forward_like<S>(self.elements[pos]);
		}

		template<class S>
		constexpr std::add_pointer_t<forward_like_t<S, value_type>> data(this S &&self) {
			return self.elements.get();
		}

		template<class S>
		constexpr std::add_pointer_t<forward_like_t<S, value_type>> back_data(this S &&self) {
			return self.max_data();
		}

		template<class S>
		constexpr std::add_pointer_t<forward_like_t<S, value_type>> max_data(this S &&self) {
			return self._max_data;
		}

		template<class S>
		constexpr forward_like_t<S, value_type> front(this S &&self) {
			return std::forward_like<S>(*self.data());
		}

		template<class S>
		constexpr forward_like_t<S, value_type> back(this S &&self) {
			return std::forward_like<S>(*self.back_data());
		}



		// Iterators
		template<class S>
		constexpr std::add_pointer_t<forward_like_t<S, value_type>> begin(this S &&self) { return self.data(); }

		// Gražiname const_iterator, nes taip uždraudžiame keitimą atminties, kuri nepriklauso konteineriui
		// ir todėl, kad negalėtume paduoti gražinamos rodyklės į fixed_vector metodus.
		constexpr const_iterator end(this const auto &self) { return self.back_data() + 1; }

		template<class S>
		constexpr std::add_pointer_t<forward_like_t<S, value_type>> rbegin(this S &&self) { return self.back_data(); }

		constexpr const_iterator rend(this const auto &self) { return self.data() - 1; }



		// Capacity
		constexpr bool empty(this const auto &self) { return self.back_data() == self.rend(); }
		constexpr bool single(this const auto &self) { return self.back_data() == self.data(); }
		constexpr bool full(this const auto &self) { return self.back_data() == self.max_data(); }

		constexpr size_type size(this const auto &self) { return unsign(self.back_data() - self.rend()); }
		constexpr size_type last_index(this const auto &self) { return unsign(self.back_data() - self.data()); }

		constexpr size_type max_size(this const auto &self) { return unsign(self.max_data() - self.rend()); }
		constexpr size_type max_index(this const auto &self) { return unsign(self.max_data() - self.data()); }
		// Galėtume neturėti šio metodo, nes gražinamos reikšmės yra pasiekiamos per konstantą tai
		// neprarastume greitaveikos, bet turime juos dėl įsivaizduojamo patogumo.

		template<class S>
		constexpr forward_like_t<S, deleter_type> get_deleter(this S &&self) {
			return std::forward_like<S>(self.elements.get_deleter());
		}



		// Modifiers
		// Neturime iš range kopijavimo metodų ar konstruktorių, nes visų atvejų kopijavimo ir taip neapimtume.
		// Šios klasės specialūs metodai yra ištrinti dėl unique_ptr naudojimo.
		constexpr fixed_array &operator=(fixed_array &&o) {
			elements = std::move(o.elements);
			_max_data = std::exchange(o._max_data, nullptr);
			return *this;
		}



		// Special member functions
	private:
		constexpr fixed_array(const pointer p, const size_type size)
			: elements{p}, _max_data{(p - 1) + size} {}

	public:
		constexpr fixed_array()
			: elements{nullptr}, _max_data{nullptr} {}

		constexpr fixed_array(fixed_array &&o)
			: elements{std::move(o.elements)}, _max_data{std::exchange(o._max_data, nullptr)} {}

		constexpr fixed_array(const size_type size) requires (std::same_as<deleter_type, std::default_delete<value_type[]>>)
			: fixed_array{new value_type[size], size} {}

		constexpr fixed_array(const size_type size, std::pmr::monotonic_buffer_resource &r) requires (std::same_as<deleter_type, std::identity>)
			: fixed_array{std::pmr::polymorphic_allocator<value_type>{&r}.allocate(size), size} {}



		// Member objects
	protected:
		std::unique_ptr<value_type[], deleter_type> elements;
		// Ne const_pointer, nes elements ir taip nebūtų const.
		pointer _max_data;
	};

	namespace pmr {
		template<class T>
		using fixed_array = aa::fixed_array<T, std::identity>;
	}

}
