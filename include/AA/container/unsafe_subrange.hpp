#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include <iterator> // input_or_output_iterator, sentinel_for, next, prev, bidirectional_iterator, random_access_iterator, contiguous_iterator, sized_sentinel_for, iter_difference_t, iter_reference_t
#include <ranges> // view_interface, iterator_t, sentinel_t, begin, borrowed_range
#include <memory> // to_address



namespace aa {

	template<std::input_or_output_iterator I, std::sentinel_for<I> S = I>
	struct unsafe_subrange : pair<I, S>, std::ranges::view_interface<unsafe_subrange<I, S>> {
		// Member types
	protected:
		using b = pair<I, S>;

	public:
		using std::ranges::view_interface<unsafe_subrange>::data;
		using pair<I, S>::get;



		// Observers
		AA_CONSTEXPR I &begin() { return b::template get<0>(); }
		AA_CONSTEXPR S &rbegin() { return b::template get<1>(); }
		AA_CONSTEXPR const I &begin() const { return b::template get<0>(); }
		AA_CONSTEXPR const S &rbegin() const { return b::template get<1>(); }
		AA_CONSTEXPR S end() const requires (std::input_or_output_iterator<S>) { return std::ranges::next(rbegin()); }
		AA_CONSTEXPR I rend() const requires (std::bidirectional_iterator<I>) { return std::ranges::prev(begin()); }

		AA_CONSTEXPR std::iter_difference_t<I> ssize() const requires (std::sized_sentinel_for<S, I>) { return last_sindex() + 1; }
		AA_CONSTEXPR aa::iter_size_t<I> size() const requires (std::sized_sentinel_for<S, I>) { return unsign(ssize()); }

		AA_CONSTEXPR std::iter_difference_t<I> last_sindex() const requires (std::sized_sentinel_for<S, I>) { return rbegin() - begin(); }
		AA_CONSTEXPR aa::iter_size_t<I> last_index() const requires (std::sized_sentinel_for<S, I>) { return unsign(last_sindex()); }

		static AA_CONSTEVAL bool empty() { return false; }
		AA_CONSTEXPR bool single() const { return begin() == rbegin(); }

		AA_CONSTEXPR std::iter_reference_t<S> back() const requires (std::input_or_output_iterator<S>) { return *rbegin(); }
		AA_CONSTEXPR aa::iter_pointer_t<S> rdata() const requires (std::contiguous_iterator<S>) { return std::to_address(rbegin()); }

		AA_CONSTEXPR I begin(const std::iter_difference_t<I> n) const requires (std::random_access_iterator<I>) { return begin() + n; }
		AA_CONSTEXPR S rbegin(const std::iter_difference_t<S> n) const requires (std::random_access_iterator<S>) { return rbegin() - n; }

		AA_CONSTEXPR std::iter_reference_t<I> get(const std::iter_difference_t<I> n) const requires (std::random_access_iterator<I>) { return *begin(n); }
		AA_CONSTEXPR std::iter_reference_t<S> rget(const std::iter_difference_t<S> n) const requires (std::random_access_iterator<S>) { return *rbegin(n); }


		AA_CONSTEXPR std::iter_reference_t<I> operator[](const aa::iter_size_t<I> n) const requires (std::contiguous_iterator<I>) { return get(n); }

		AA_CONSTEXPR std::iter_reference_t<I> get(const aa::iter_size_t<I> n) const requires (std::contiguous_iterator<I>) { return *data(n); }
		AA_CONSTEXPR std::iter_reference_t<S> rget(const aa::iter_size_t<S> n) const requires (std::contiguous_iterator<S>) { return *rdata(n); }

		AA_CONSTEXPR aa::iter_pointer_t<I> data(const aa::iter_size_t<I> n) const requires (std::contiguous_iterator<I>) { return data() + n; }
		AA_CONSTEXPR aa::iter_pointer_t<S> rdata(const aa::iter_size_t<S> n) const requires (std::contiguous_iterator<S>) { return rdata() - n; }



		// Special member functions
		// Reikia konstruktorių, nes kitaip metami warnings -Wmissing-field-initializers.
		AA_CONSTEXPR unsafe_subrange() = default;

		// convertible_to, nes std::ranges::subrange klasė taip pat realizuota.
		// convertible constraint yra griežtesnis negu constructable constraint.
		template<std::convertible_to<I> T1 = I, std::convertible_to<S> T2 = S>
		AA_CONSTEXPR unsafe_subrange(T1 &&t1, T2 &&t2) : b{std::forward<T1>(t1), std::forward<T2>(t2)} {}

		template<std::ranges::borrowed_range R>
		AA_CONSTEXPR unsafe_subrange(R &&r) : unsafe_subrange{std::ranges::begin(r), get_rbegin(r)} {}
	};

	template<class I, class S>
	unsafe_subrange(I &&, S &&) -> unsafe_subrange<I, S>;

	template<class R>
	unsafe_subrange(R &&) -> unsafe_subrange<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>>;

}
