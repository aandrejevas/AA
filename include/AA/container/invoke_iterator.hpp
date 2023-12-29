#pragma once

#include "../metaprogramming/general.hpp"



namespace aa {

	enum struct invoke_iter_config : size_t {
		PRE_INCREMENT = int_exp2(0uz),
		PRE_DECREMENT = int_exp2(1uz),
		POST_INCREMENT = int_exp2(2uz),
		POST_DECREMENT = int_exp2(3uz),
		ASSIGNMENT = int_exp2(4uz),
		SUBTRACTION = int_exp2(5uz),
		ADDITION = int_exp2(6uz),
	};

	// F ne data member, nes, kad klasė galėtų būti iterator, ji turi būti default constructible ir lambd'os yra tokios
	// tik jei jos neturi captures, bet tokiu atveju bet koks F gali būti paduodamas tiesiog kaip template parametras.
	template<invoke_iter_config C, invoke_iterator_invocable auto F>
	struct invoke_iterator : invoke_iterator_tuple_t<const_t<F>> {
		// Member types
		using typename invoke_iterator_tuple_t<const_t<F>>::tuple_type;
		using I = std::remove_cvref_t<function_argument_t<const_t<F>>>;
		using value_type = std::iter_value_t<I>;
		using difference_type = std::iter_difference_t<I>;
		using reference = std::iter_reference_t<I>;
		using pointer = pointer_in_use_t<std::iterator_traits<I>>;
		using iterator_category = iterator_concept_in_use_t<std::iterator_traits<I>>;



		// Gal gali atrodyti, kad kai kurių funkcijų nereikia ar panašiai, bet funkcijų declarations turi nesikeisti.
		// Observers
	protected:
		using enum invoke_iter_config;

		template<invoke_iter_config FLAG>
		constexpr void log() const {
			using namespace operators;
			if constexpr (C & FLAG) {
				apply<tuple_type>([&]<size_t... INDEX> -> void {
					std::invoke(F, i, get_v<INDEX>(*this)...);
				});
			}
		}

	public:
		constexpr reference operator*() const {
			return *i;
		}

		constexpr pointer operator->() const requires (std::contiguous_iterator<I>) {
			return std::to_address(i);
		}

		constexpr reference operator[](const difference_type n) const requires (std::random_access_iterator<I>) {
			return i[n];
		}

		friend constexpr invoke_iterator operator+(const invoke_iterator &o, const difference_type n) requires (std::random_access_iterator<I>) {
			return {o.i + n};
		}

		friend constexpr invoke_iterator operator+(const difference_type n, const invoke_iterator &o) requires (std::random_access_iterator<I>) {
			return {o.i + n};
		}

		friend constexpr invoke_iterator operator-(const invoke_iterator &o, const difference_type n) requires (std::random_access_iterator<I>) {
			return {o.i - n};
		}

		template<std::sized_sentinel_for<I> S>
		constexpr difference_type operator-(const S &s) const {
			return i - s;
		}

		template<not_instance_of_and_sized_sentinel_for<invoke_iterator, I> S>
		friend constexpr difference_type operator-(const S &s, const invoke_iterator &o) {
			return s - o.i;
		}

		template<std::sentinel_for<I> S>
		constexpr bool operator==(const S &s) const {
			return i == s;
		}

		template<std::three_way_comparable_with<I> S>
		constexpr auto operator<=>(const S &s) const {
			return i <=> s;
		}

		constexpr auto operator<=>(const invoke_iterator &o) const requires (std::three_way_comparable<I>) {
			return i <=> o.i;
		}



		// Modifiers
		constexpr invoke_iterator &operator++() {
			log<PRE_INCREMENT>(); return (++i, *this);
		}

		constexpr invoke_iterator operator++(const int) {
			log<POST_INCREMENT>(); return {i++};
		}

		constexpr invoke_iterator &operator--() requires (std::bidirectional_iterator<I>) {
			log<PRE_DECREMENT>(); return (--i, *this);
		}

		constexpr invoke_iterator operator--(const int) requires (std::bidirectional_iterator<I>) {
			log<POST_DECREMENT>(); return {i--};
		}

		constexpr invoke_iterator &operator+=(const difference_type n) requires (std::random_access_iterator<I>) {
			log<ADDITION>(); return (i += n, *this);
		}

		constexpr invoke_iterator &operator-=(const difference_type n) requires (std::random_access_iterator<I>) {
			log<SUBTRACTION>(); return (i -= n, *this);
		}

		template<assignable_to<I &> S>
		constexpr invoke_iterator &operator=(S &&s) {
			log<ASSIGNMENT>(); return (i = std::forward<S>(s), *this);
		}

		constexpr invoke_iterator &operator=(const invoke_iterator &o) {
			log<ASSIGNMENT>(); return (i = o.i, *this);
		}



		// Special member functions
		constexpr invoke_iterator() = default;

		template<constructible_to<I> S, class... V>
			requires (std::constructible_from<tuple_type, V...>)
		constexpr invoke_iterator(S &&s, V&&... v) : tuple_type{std::forward<V>(v)...}, i{std::forward<S>(s)} {}

		constexpr invoke_iterator(const invoke_iterator &o) : tuple_type{o}, i{o.i} {}



		// Member objects
		I i;
	};

}
