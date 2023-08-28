#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include <iterator> // input_or_output_iterator, sentinel_for, next, prev, input_iterator, bidirectional_iterator, random_access_iterator, contiguous_iterator, sized_sentinel_for, iter_difference_t, iter_reference_t
#include <ranges> // view_base, iterator_t, sentinel_t, begin, borrowed_range
#include <memory> // to_address



namespace aa {

	template<std::input_or_output_iterator I, std::sentinel_for<I> S = I>
	struct unsafe_subrange : pair<I, S>, std::ranges::view_base {
		// Observers
		using typename pair<I, S>::tuple_type;
		using tuple_type::get;

		constexpr I &begin() { return get_0(*this); }
		constexpr S &rbegin() { return get_1(*this); }
		constexpr const I &begin() const { return get_0(*this); }
		constexpr const S &rbegin() const { return get_1(*this); }
		constexpr S end() const requires (std::input_or_output_iterator<S>) { return std::ranges::next(rbegin()); }
		constexpr I rend() const requires (std::bidirectional_iterator<I>) { return std::ranges::prev(begin()); }

		constexpr std::iter_difference_t<I> ssize() const requires (std::sized_sentinel_for<S, I>) { return sindexl() + 1; }
		constexpr aa::iter_size_t<I> size() const requires (std::sized_sentinel_for<S, I>) { return indexl() + 1; }

		constexpr std::iter_difference_t<I> sindexl() const requires (std::sized_sentinel_for<S, I>) { return rbegin() - begin(); }
		constexpr aa::iter_size_t<I> indexl() const requires (std::sized_sentinel_for<S, I>) { return unsign(sindexl()); }

		explicit consteval operator bool() const { return true; }
		static consteval bool empty() { return false; }
		constexpr bool single() const { return begin() == rbegin(); }

		constexpr std::iter_reference_t<I> front() const requires (std::input_iterator<I>) { return *begin(); }
		constexpr std::iter_reference_t<S> back() const requires (std::input_iterator<S>) { return *rbegin(); }

		constexpr aa::iter_pointer_t<I> data() const requires (std::contiguous_iterator<I>) { return std::to_address(begin()); }
		constexpr aa::iter_pointer_t<S> rdata() const requires (std::contiguous_iterator<S>) { return std::to_address(rbegin()); }

		constexpr I begin(const std::iter_difference_t<I> n) const requires (std::random_access_iterator<I>) { return begin() + n; }
		constexpr S rbegin(const std::iter_difference_t<S> n) const requires (std::random_access_iterator<S>) { return rbegin() - n; }

		constexpr std::iter_reference_t<I> operator[](const std::iter_difference_t<I> n) const requires (std::random_access_iterator<I>) { return get(n); }
		constexpr std::iter_reference_t<I> get(const std::iter_difference_t<I> n) const requires (std::random_access_iterator<I>) { return *begin(n); }
		constexpr std::iter_reference_t<S> rget(const std::iter_difference_t<S> n) const requires (std::random_access_iterator<S>) { return *rbegin(n); }


		constexpr std::iter_reference_t<I> operator[](const aa::iter_size_t<I> n) const requires (std::contiguous_iterator<I>) { return get(n); }
		constexpr std::iter_reference_t<I> get(const aa::iter_size_t<I> n) const requires (std::contiguous_iterator<I>) { return *data(n); }
		constexpr std::iter_reference_t<S> rget(const aa::iter_size_t<S> n) const requires (std::contiguous_iterator<S>) { return *rdata(n); }

		constexpr aa::iter_pointer_t<I> data(const aa::iter_size_t<I> n) const requires (std::contiguous_iterator<I>) { return data() + n; }
		constexpr aa::iter_pointer_t<S> rdata(const aa::iter_size_t<S> n) const requires (std::contiguous_iterator<S>) { return rdata() - n; }



		// Special member functions
		// Reikia konstruktorių, nes kitaip metami warnings -Wmissing-field-initializers.
		constexpr unsafe_subrange() = default;

		// convertible_to, nes std::ranges::subrange klasė taip pat realizuota.
		// convertible constraint yra griežtesnis negu constructable constraint.
		template<std::convertible_to<I> T1 = I, std::convertible_to<S> T2 = S>
		constexpr unsafe_subrange(T1 &&t1, T2 &&t2) : tuple_type{std::forward<T1>(t1), std::forward<T2>(t2)} {}

		template<std::ranges::borrowed_range R>
		constexpr unsafe_subrange(R &&r) : unsafe_subrange{std::ranges::begin(r), get_rbegin(r)} {}
	};

	template<class I, class S>
	unsafe_subrange(I &&, S &&) -> unsafe_subrange<I, S>;

	template<class R>
	unsafe_subrange(R &&) -> unsafe_subrange<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>>;

}
