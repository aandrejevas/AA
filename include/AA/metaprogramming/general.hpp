#pragma once

// AA bibliotekos failuose elgiamasi lyg būtų įterpti toliau išdėstyti failai, nes tie failai suteikia galimybę
// sklandžiai programuoti naudojantis esminėmis c++ kalbos savybėmis. Tie failai yra ir jų įterpimo priežastys:
// • <cstddef> ir <cstdint>, failai įterpti, kad nereiktų naudoti daug raktažodžių, kad aprašyti pamatinius tipus.
// • <array> (<compare>, <initializer_list>), failas įterptas, kad nereiktų naudoti C stiliaus masyvų.
// • <string_view> (<compare>), failas įterptas, kad nereiktų naudoti C stiliaus teksto eilučių.
// • <type_traits> ir <concepts>, failai įterpti, kad išeitų lengvai protauti apie tipus.
// • <utility> (<compare>, <initializer_list>), failas įterptas, kad išeitų lengvai protauti apie išraiškas.
// • <limits>, failas įterptas, kad išeitų lengvai nautotis pamatinių tipų savybėmis.
// Failai paminėti skliausteliose prie įterpiamo failo nurodo kokie failai yra įterpiami pačio įterpiamo failo.

#include "../preprocessor/general.hpp"
#include <cstddef> // byte, size_t
#include <cstdint> // uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
#include <type_traits> // remove_reference_t, type_identity, bool_constant, true_type, false_type, integral_constant, conditional, conditional_t, is_void_v, has_unique_object_representations_v, is_trivial_v, is_trivially_copyable_v, is_trivially_default_constructible_v, add_const_t, is_const_v, is_arithmetic_v, invoke_result_t, underlying_type_t, extent_v, remove_cvref, remove_cvref_t, remove_const_t, is_pointer_v, remove_pointer_t, is_function_v
#include <concepts> // convertible_to, same_as, default_initializable, copy_constructible, unsigned_integral, relation, invocable, derived_from, copyable
#include <limits> // numeric_limits
#include <string_view> // string_view
#include <array> // array
#include <utility> // forward, declval, as_const, tuple_size, tuple_size_v, tuple_element, tuple_element_t, index_sequence, make_index_sequence



namespace aa {

	template<template<class...> class F, class... A1>
	struct bind_types {
		template<class... A2>
		struct front : std::type_identity<F<A1..., A2...>> {};

		template<class... A2>
		using front_t = front<A2...>::type;
	};

	template<template<size_t...> class F, size_t N>
	struct apply_indices : decltype(([]<size_t... I>(const std::index_sequence<I...>) consteval ->
		std::type_identity<F<I...>> { return {}; })(std::make_index_sequence<N>{})) {};

	template<template<size_t...> class F, size_t N>
	using apply_indices_t = apply_indices<F, N>::type;



	// Klasė negali būti pakeista į concept, nes concepts negali būti recursive.
	template<class T = void, class... A>
	struct are_unique : std::bool_constant<!(... || std::same_as<T, A>) && are_unique<A...>::value> {};

	template<class T>
	struct are_unique<T> : std::true_type {};

	template<class... A>
	AA_CONSTEXPR const bool are_unique_v = are_unique<A...>::value;

	// Klasė galėtų būti pakeista į concept, bet nėra logiška, kad concept'as galėtų būti naudojamas su 0 template parametrų.
	template<class T = void, class... A>
	struct are_same : std::bool_constant<!sizeof...(A) || (... && std::same_as<T, A>)> {};

	template<class... A>
	AA_CONSTEXPR const bool are_same_v = are_same<A...>::value;



	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<class T>
	concept uniquely_representable = std::has_unique_object_representations_v<T>;

	template<class T>
	concept trivial = std::is_trivial_v<T>;

	template<class T>
	concept trivially_copyable = std::is_trivially_copyable_v<T>;

	template<class T>
	concept trivially_default_constructible = std::is_trivially_default_constructible_v<T>;

	template<class T, class U>
	concept unsigned_integral_same_as = std::unsigned_integral<T> && std::same_as<T, U>;

	template<class L, class R>
	concept remove_ref_same_as = std::same_as<std::remove_reference_t<L>, R>;

	// Klasės reikia, nes is_constructible tikrina ar klasę galima sukonstruoti naudojant lenktinius skliaustelius, o ne riestinius.
	template<class T, class... A>
	struct is_list_constructible : std::false_type {};

	template<class T, class... A>
		requires requires(A&&... args) { T{std::forward<A>(args)...}; }
	struct is_list_constructible<T, A...> : std::true_type {};

	template<class T, class... A>
	AA_CONSTEXPR const bool is_list_constructible_v = is_list_constructible<T, A...>::value;



	// https://en.wikipedia.org/wiki/Function_object
	template<class T>
	concept functor = requires { T::operator(); };

	template<class F, auto... A>
	concept nttp_accepting_functor = std::invocable<decltype(&(std::remove_reference_t<F>::template AA_CALL_OPERATOR<A...>)), F>;

	// Vietoje requires parametrų galima naudoti declval, bet atrodo tai negražiai, nėra reikalo to daryti.
	// https://mathworld.wolfram.com/Multiplier.html
	template<class L, class R = L>
	concept multiplier = requires(L && l, R && r) { (std::forward<L>(l) * std::forward<R>(r)); };

	// https://mathworld.wolfram.com/Multiplicand.html
	template<class R, class L = R>
	concept multiplicand = multiplier<L, R>;

	// AA_MUL uždeda skliaustelius apie išraišką, bet šiuo atveju tai nesvarbu, nes decltype ir su
	// skliausteliais ir be jų nustatytų tą patį tipą, nes jam paduodama išraiška, o ne kintamojo vardas.
	template<class L, multiplicand<L> R = L>
	struct product_result
		: std::type_identity<decltype(AA_MUL(std::declval<L>(), std::declval<R>()))> {};

	template<class L, class R = L>
	using product_result_t = product_result<L, R>::type;



	// T čia nebūtinai turi būti tuple_like, nes tiesiog sakome, kad šias deklaracijas galime naudoti su visais tipais.
	template<class T, size_t I>
	concept member_get = (I < std::tuple_size_v<std::remove_reference_t<T>>) && requires(T & t) {
		{ t.template get<I>() } -> remove_ref_same_as<std::tuple_element_t<I, std::remove_reference_t<T>>>;
	};

	template<class T, size_t I>
	concept adl_get = (I < std::tuple_size_v<std::remove_reference_t<T>>) && requires(T & t) {
		{ get<I>(t) } -> remove_ref_same_as<std::tuple_element_t<I, std::remove_reference_t<T>>>;
	};

	template<class T, size_t I>
	concept gettable = member_get<T, I> || adl_get<T, I>;

	namespace detail {
		template<size_t I>
		struct getter {
			template<gettable<I> T>
			[[gnu::always_inline]] AA_CONSTEXPR decltype(auto) operator()(T &&t) const {
				if constexpr (member_get<T, I>) {
					return t.template get<I>();
				} else {
					return get<I>(t);
				}
			}
		};
	}

	AA_CONSTEXPR const detail::getter<0> get_0 = {}, get_x = {};
	AA_CONSTEXPR const detail::getter<1> get_1 = {}, get_y = {};

	template<class T, size_t I>
	concept normal_get = requires(std::remove_const_t<T> &t) {
		{ detail::getter<I>{}(t) } -> std::same_as<std::tuple_element_t<I, std::remove_cvref_t<T>> &>;
		{ detail::getter<I>{}(std::as_const(t)) } -> std::same_as<std::tuple_element_t<I, const std::remove_cvref_t<T>> &>;
	};

	template<class T>
	struct is_tuple {
		template<size_t... I>
		struct normal : std::false_type {};

		template<size_t... I>
			requires (... && normal_get<T, I>)
		struct normal<I...> : std::true_type {};

		template<size_t... I>
		static AA_CONSTEXPR const bool normal_v = normal<I...>::value;

		template<size_t... I>
		struct valid : std::false_type {};

		template<size_t... I>
			requires (... && gettable<T, I>)
		struct valid<I...> : std::true_type {};

		template<size_t... I>
		static AA_CONSTEXPR const bool valid_v = valid<I...>::value;
	};

	template<class T>
	concept tuple_like = std::derived_from<std::tuple_size<std::remove_reference_t<T>>,
		std::integral_constant<size_t, std::tuple_size_v<std::remove_reference_t<T>>>>
		&& apply_indices_t<is_tuple<T>::template valid, std::tuple_size_v<std::remove_reference_t<T>>>::value;

	// Tokia klasė, nes neeina nurodyti/naudoti type parameter pack ir non-type parameter pack šalia vienas kito,
	// o to reikia, kad išeitų sukurti bendrą binder klasę, kuri pirma bind'intų tipus, o tada likusius indeksus.
	template<template<class...> class F, tuple_like T>
	struct map_to_types {
		template<size_t... I>
		struct from_indices : std::type_identity<F<std::tuple_element_t<I, std::remove_reference_t<T>>...>> {};

		template<size_t... I>
		using from_indices_t = from_indices<I...>::type;
	};

	template<template<class...> class F, tuple_like T>
	struct apply_types : apply_indices<map_to_types<F, T>::template from_indices_t, std::tuple_size_v<std::remove_reference_t<T>>> {};

	template<template<class...> class F, class T>
	using apply_types_t = apply_types<F, T>::type;

	template<class T>
	concept array_like = tuple_like<T> && apply_types_t<are_same, T>::value;

	template<array_like T>
	struct array_element : std::tuple_element<0, std::remove_reference_t<T>> {};

	template<class T>
	using array_element_t = array_element<T>::type;

	template<class T>
	concept vector_like = array_like<T> && arithmetic<array_element_t<T>> && std::copyable<std::remove_reference_t<T>>
		&& apply_types_t<bind_types<is_list_constructible, std::remove_reference_t<T>>::template front_t, T>::value
		&& apply_indices_t<is_tuple<T>::template normal, std::tuple_size_v<std::remove_reference_t<T>>>::value;

	template<size_t N, class T>
	concept tupleN_like = tuple_like<T> && (std::tuple_size_v<T> == N);

	template<class T>
	concept pair_like = tupleN_like<2, T>;

	template<size_t N, class T>
	concept arrayN_like = array_like<T> && (std::tuple_size_v<T> == N);

	template<class T>
	concept array2_like = arrayN_like<2, T>;

	template<size_t N, class T>
	concept vectorN_like = vector_like<T> && (std::tuple_size_v<T> == N);

	template<class T>
	concept vector2_like = vectorN_like<2, T>;



	template<class F, class... A>
	concept function_pointer = std::invocable<F, A...> && std::is_pointer_v<F> && std::is_function_v<std::remove_pointer_t<F>>;

	template<class F, class... A>
	concept invocable_ref = std::invocable<F &, A...>;

	template<class T, class F>
	concept convertible_from = std::convertible_to<F, T>;

	template<class T, class F>
	concept void_or_convertible_from = std::is_void_v<T> || std::convertible_to<F, T>;

	template<class T>
	concept void_or_convertible_from_floating_point = std::is_void_v<T>
		|| (std::convertible_to<float, T> && std::convertible_to<double, T> && std::convertible_to<long double, T>);



	template<class T>
	concept storable = std::default_initializable<std::remove_reference_t<T>> && std::copy_constructible<T>;

	template<class U, class V, class T>
	concept in_relation_with = std::relation<const T &, const U &, const V &>;

	template<class T, class U, class V = U>
	concept storable_relation = in_relation_with<U, V, T> && storable<T>;

	template<class U, class T>
	concept hashable_by = std::invocable<const T &, const U &> && std::same_as<std::invoke_result_t<const T &, const U &>, size_t>;

	template<class T, class U>
	concept storable_hasher = hashable_by<U, T> && storable<T>;

	// evaluator'ius gali nebūtinai gražinti be kvalifikatorių tipą, bet copy_constructible turėtų užtikrinti,
	// kad kad ir ką gražintų evaluator'ius, kad tai būtų paverčiama į tipą be kvalifikatorių.
	template<class T>
	concept storable_evaluator = std::invocable<const T &, const std::string_view &> && storable<T>
		&& std::copy_constructible<std::remove_cvref_t<std::invoke_result_t<const T &, const std::string_view &>>>;

	template<storable_evaluator T>
	struct evaluator_result : std::remove_cvref<std::invoke_result_t<const T &, const std::string_view &>> {};

	template<class T>
	using evaluator_result_t = evaluator_result<T>::type;

	template<class T, class U>
	concept storable_vector2_getter = storable<T>
		&& std::invocable<const T &, const U &> && vector2_like<std::remove_cvref_t<std::invoke_result_t<const T &, const U &>>>;

	template<class U, storable_vector2_getter<U> T>
	struct vector2_getter_result : std::remove_cvref<std::invoke_result_t<const T &, const U &>> {};

	template<class U, class T>
	using vector2_getter_result_t = vector2_getter_result<U, T>::type;



	// https://mathworld.wolfram.com/Hypermatrix.html
	// Gal reiktų vadinti struktūrą ne array, o hypermatrix, bet pasiliekama prie array vardo,
	// nes galime interpretuoti stuktūros pavadinimą kaip masyvą masyvų.
	template<class T, size_t N1, size_t... N>
	struct array : array<std::array<T, N1>, N...> {};

	template<class T, size_t N1>
	struct array<T, N1> : std::type_identity<std::array<T, N1>> {};

	template<class T, size_t N1, size_t... N>
	using array_t = array<T, N1, N...>::type;



	namespace detail {
		template<class T, size_t D, size_t N0, size_t... N>
		struct square_array_base : square_array_base<T, D - 1, N0, N0, N...> {};

		template<class T, size_t N0, size_t... N>
		struct square_array_base<T, 0, N0, N...> : array<T, N...> {};
	}

	// https://mathworld.wolfram.com/SquareArray.html
	template<class T, size_t D, size_t N>
	struct square_array : detail::square_array_base<T, D, N> {};

	template<class T, size_t D, size_t N>
	using square_array_t = square_array<T, D, N>::type;



	template<class...>
	struct first_not_void;

	template<class A1, class... A>
	struct first_not_void<A1, A...> : std::conditional_t<std::is_void_v<A1>, first_not_void<A...>, std::type_identity<A1>> {};

	template<class... A>
	using first_not_void_t = first_not_void<A...>::type;



	// C yra objektas, galėtume jį padavinėti per const&, o ne per value, bet nusprendžiau, kad kompiliavimo
	// metu nėra skirtumo kaip tas objektas bus padavinėjamas, net jei jis būtų labai didelis.
	template<auto C>
	struct constant : std::integral_constant<decltype(C), C> {};

	template<auto C>
	AA_CONSTEXPR const decltype(C) constant_v = constant<C>::value;

	template<convertible_from<size_t> T>
	struct zero : std::integral_constant<T, static_cast<T>(0uz)> {};

	template<class T>
	AA_CONSTEXPR const T zero_v = zero<T>::value;

	template<convertible_from<size_t> T>
	struct one : std::integral_constant<T, static_cast<T>(1uz)> {};

	template<class T>
	AA_CONSTEXPR const T one_v = one<T>::value;

	template<convertible_from<size_t> T>
	struct two : std::integral_constant<T, static_cast<T>(2uz)> {};

	template<class T>
	AA_CONSTEXPR const T two_v = two<T>::value;



	template<class, class...>
	struct next_type;

	template<class T, class A1, class A2, class... A>
	struct next_type<T, A1, A2, A...> : std::conditional_t<std::same_as<T, A1>, std::type_identity<A2>, next_type<T, A2, A...>> {};

	template<class T, class... A>
	using next_type_t = next_type<T, A...>::type;

	template<class T>
	struct next_unsigned : next_type<T, uint8_t, uint16_t, uint32_t, uint64_t> {};
	template<class T>
	using next_unsigned_t = next_unsigned<T>::type;

	template<class T>
	struct prev_unsigned : next_type<T, uint64_t, uint32_t, uint16_t, uint8_t> {};
	template<class T>
	using prev_unsigned_t = prev_unsigned<T>::type;

	template<class T>
	struct next_signed : next_type<T, int8_t, int16_t, int32_t, int64_t> {};
	template<class T>
	using next_signed_t = next_signed<T>::type;

	template<class T>
	struct prev_signed : next_type<T, int64_t, int32_t, int16_t, int8_t> {};
	template<class T>
	using prev_signed_t = prev_signed<T>::type;



	template<template<class> class F, bool C, class T>
	struct apply_if : std::conditional<C, F<T>, T> {};

	template<template<class> class F, bool C, class T>
	using apply_if_t = apply_if<F, C, T>::type;



	template<bool C, class T>
	struct add_const_if : apply_if<std::add_const_t, C, T> {};

	template<bool C, class T>
	using add_const_if_t = add_const_if<C, T>::type;



	template<class F, class T>
	struct copy_const : add_const_if<std::is_const_v<F>, T> {};

	template<class F, class T>
	using copy_const_t = copy_const<F, T>::type;



	// Naudojamas size_t tipas kaip konstantos tipas, o ne sekančio dydžio integer tipas, nes konstanta
	// turėtų būti naudojama malloc ir panašiuose kontekstuose, o ten reikalaujama size_t tipo išraiška.
	// T turi turėti unique object representations, nes kitaip neišeis patikimai apskaičiuoti konstantos.
	//
	// Daugybos nepaverčiame į postūmio operaciją, nes kompiliavimo metu ilgiau užtruktų nustatyti ar galime daryti
	// postūmį negu tiesiog įvykdyti daugybos operaciją, programos veikimo laikui irgi nepadėtų tokios kostrukcijos.
	//
	// Vietoje byte negalime naudoti uint8_t, nes jei sistemoje baitas būtų ne 8 bitų, tas tipas nebus apibrėžtas.
	template<uniquely_representable T>
	struct representable_values
		: std::integral_constant<size_t, AA_SHL(1uz, sizeof(T[std::numeric_limits<std::underlying_type_t<std::byte>>::digits]))> {};

	template<class T>
	AA_CONSTEXPR const size_t representable_values_v = representable_values<T>::value;



	AA_CONSTEXPR const struct {
		template<class T>
		AA_CONSTEVAL operator T() const { return std::numeric_limits<T>::max(); }
	} numeric_max;

	AA_CONSTEXPR const struct {
		template<class T>
		AA_CONSTEVAL operator T() const { return std::numeric_limits<T>::min(); }
	} numeric_min;



	// Galima būtų naudoti source_location, bet ta klasė surenka daugiau duomenų negu reikia ir taip pat
	// kadangi neišeitų panaudoti strlen tai ir neišeitų užtikrinti lengvai compile time veikimo.
	template<class>
	AA_CONSTEVAL std::string_view type_name() {
		return std::string_view{__PRETTY_FUNCTION__ + 76, (std::extent_v<std::remove_reference_t<decltype(__PRETTY_FUNCTION__)>>) - 127};
	}

}
