#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include <iterator> // next, prev, bidirectional_iterator, random_access_iterator, contiguous_iterator, iter_value_t, iter_difference_t, iter_reference_t
#include <ranges> // view_base, iterator_t, begin, borrowed_range
#include <memory> // to_address



namespace aa {

	// Nereikia antro template parametro S, nes S rodytų į paskutinį elementą ir jei
	// I ir S rodo į tos pačios sekos elementus tai S turi būti tiesiog įmanoma paversti į I.
	// rend operaciją taip pat būtina palaikyti todėl logiška reikalauti, kad iš I galėtume gauti tą elementą.
	template<std::bidirectional_iterator I>
	struct unsafe_subrange : pair<I>, std::ranges::view_base {
		// Member types
		using typename pair<I>::tuple_type;
		using tuple_type::get;
		using value_type = std::iter_value_t<I>;
		using size_type = iter_size_t<I>;
		using difference_type = std::iter_difference_t<I>;
		using reference = std::iter_reference_t<I>;
		using pointer = iter_pointer_t<I>;
		using iterator = I;



		// Observers
		constexpr iterator &begin() { return get_0(*this); }
		constexpr iterator &rbegin() { return get_1(*this); }
		constexpr const iterator &begin() const { return get_0(*this); }
		constexpr const iterator &rbegin() const { return get_1(*this); }
		constexpr iterator end() const { return std::ranges::next(rbegin()); }
		constexpr iterator rend() const { return std::ranges::prev(begin()); }

		constexpr difference_type ssize() const requires (std::random_access_iterator<I>) { return sindexl() + 1; }
		constexpr size_type size() const requires (std::random_access_iterator<I>) { return indexl() + 1; }

		constexpr difference_type sindexl() const requires (std::random_access_iterator<I>) { return rbegin() - begin(); }
		constexpr size_type indexl() const requires (std::random_access_iterator<I>) { return unsign(sindexl()); }

		explicit consteval operator bool() const { return true; }
		static consteval bool empty() { return false; }
		constexpr bool single() const { return begin() == rbegin(); }

		constexpr reference front() const { return *begin(); }
		constexpr reference back() const { return *rbegin(); }

		constexpr pointer data() const requires (std::contiguous_iterator<I>) { return std::to_address(begin()); }
		constexpr pointer rdata() const requires (std::contiguous_iterator<I>) { return std::to_address(rbegin()); }

		constexpr iterator begin(const difference_type n) const requires (std::random_access_iterator<I>) { return begin() + n; }
		constexpr iterator rbegin(const difference_type n) const requires (std::random_access_iterator<I>) { return rbegin() - n; }

		constexpr reference operator[](const difference_type n) const requires (std::random_access_iterator<I>) { return get(n); }
		constexpr reference get(const difference_type n) const requires (std::random_access_iterator<I>) { return *begin(n); }
		constexpr reference rget(const difference_type n) const requires (std::random_access_iterator<I>) { return *rbegin(n); }


		constexpr reference operator[](const size_type n) const requires (std::contiguous_iterator<I>) { return get(n); }
		constexpr reference get(const size_type n) const requires (std::contiguous_iterator<I>) { return *data(n); }
		constexpr reference rget(const size_type n) const requires (std::contiguous_iterator<I>) { return *rdata(n); }

		constexpr pointer data(const size_type n) const requires (std::contiguous_iterator<I>) { return data() + n; }
		constexpr pointer rdata(const size_type n) const requires (std::contiguous_iterator<I>) { return rdata() - n; }



		// Special member functions
		// Reikia konstruktorių, nes kitaip metami warnings -Wmissing-field-initializers.
		constexpr unsafe_subrange() = default;

		// convertible_to constraint yra naudojamas klasėje std::ranges::subrange.
		// Bet constructable constraint naudojame, nes jis griežtesnis.
		// Pagal mano padarytus testus tie constraints labai mažai kuo skiriasi.
		template<constructible_to<I> T1 = I, constructible_to<I> T2 = I>
		constexpr unsafe_subrange(T1 &&t1, T2 &&t2) : tuple_type{std::forward<T1>(t1), std::forward<T2>(t2)} {}

		template<std::ranges::borrowed_range R>
		constexpr unsafe_subrange(R &&r) : tuple_type{std::ranges::begin(r), get_rbegin(r)} {}
	};

	template<class I, class S>
	unsafe_subrange(I &&, S &&) -> unsafe_subrange<std::common_type_t<I, S>>;

	template<class R>
	unsafe_subrange(R &&) -> unsafe_subrange<std::ranges::iterator_t<R>>;

}
