#pragma once

// AA bibliotekos failuose elgiamasi lyg būtų įterpti toliau išdėstyti failai, nes tie failai suteikia galimybę
// sklandžiai programuoti naudojantis esminėmis c++ kalbos savybėmis. Tie failai yra ir jų įterpimo priežastys:
// • <cstddef> ir <cstdint>, failai įterpti, kad nereiktų naudoti daug raktažodžių, kad aprašyti pamatinius tipus.
// • <array> (<compare>, <initializer_list>), failas įterptas, kad nereiktų naudoti C stiliaus masyvų.
// • <string_view> (<compare>) ir <string> (<compare>, <initializer_list>), failai įterpti, kad nereiktų naudoti C stiliaus teksto eilučių.
// • <type_traits> ir <concepts>, failai įterpti, kad išeitų lengvai protauti apie tipus.
// • <limits>, failas įterptas, kad išeitų lengvai nautotis pamatinių tipų savybėmis.
// Failai paminėti skliausteliose prie įterpiamo failo nurodo kokie failai yra įterpiami pačio įterpiamo failo.

#include "../preprocessor/general.hpp"
#include <cstddef> // byte, size_t
#include <cstdint> // uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
#include <type_traits> // remove_reference_t, type_identity, bool_constant, true_type, integral_constant, conditional, conditional_t, is_void_v, has_unique_object_representations_v, is_trivially_copyable_v, add_const_t, is_const_v, is_arithmetic_v, invoke_result_t, underlying_type_t, extent_v, remove_cvref_t
#include <concepts> // convertible_to, same_as, default_initializable, copy_constructible, unsigned_integral, floating_point, relation, invocable
#include <limits> // numeric_limits
#include <string_view> // string_view
#include <array> // array



namespace aa {

	template<class T>
	concept functor = requires { T::operator(); };



	template<class T, class F>
	concept convertible_from = std::is_void_v<T> || std::convertible_to<F, T>;

	template<class To, class F1, class... F>
	concept convertible_from_all = std::is_void_v<To> || (std::convertible_to<F1, To> && ... && std::convertible_to<F, To>);

	template<class To>
	concept convertible_from_floating_point = convertible_from_all<To, float, double, long double>;

	template<class F, class T>
	concept floating_point_and_convertible_to = std::floating_point<F> && std::convertible_to<F, T>;



	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<class T>
	concept uniquely_representable = std::has_unique_object_representations_v<T>;

	template<class T>
	concept trivially_copyable = std::is_trivially_copyable_v<T>;



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
	struct evaluator_result : std::type_identity<std::remove_cvref_t<std::invoke_result_t<const T &, const std::string_view &>>> {};

	template<class T>
	using evaluator_result_t = evaluator_result<T>::type;



	template<class T, class U>
	concept unsigned_integral_same_as = std::unsigned_integral<T> && std::same_as<T, U>;



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
	struct fist_not_void;

	template<class A1, class... A>
	struct fist_not_void<A1, A...> : std::conditional_t<std::is_void_v<A1>, fist_not_void<A...>, std::type_identity<A1>> {};

	template<class... A>
	using fist_not_void_t = fist_not_void<A...>::type;



	template<class T = void, class... A>
	struct are_unique : std::bool_constant<!(... || std::same_as<T, A>) && are_unique<A...>::value> {};

	template<class T>
	struct are_unique<T> : std::true_type {};

	template<class... A>
	inline constexpr const bool are_unique_v = are_unique<A...>::value;



	// C yra objektas, galėtume jį padavinėti per const&, o ne per value, bet nusprendžiau, kad kompiliavimo
	// metu nėra skirtumo kaip tas objektas bus padavinėjamas, net jei jis būtų labai didelis.
	template<auto C>
	struct constant : std::integral_constant<decltype(C), C> {};

	template<auto C>
	inline constexpr const decltype(C) constant_v = constant<C>::value;



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
		: std::integral_constant<size_t, AA_EXP2(std::numeric_limits<std::underlying_type_t<std::byte>>::digits * sizeof(T))> {};

	template<class T>
	inline constexpr const size_t representable_values_v = representable_values<T>::value;



	// Galima būtų naudoti source_location, bet ta klasė surenka daugiau duomenų negu reikia ir taip pat
	// kadangi neišeitų panaudoti strlen tai ir neišeitų užtikrinti lengvai compile time veikimo.
	template<class>
	inline consteval std::string_view type_name() {
		return std::string_view{__PRETTY_FUNCTION__ + 76, (std::extent_v<std::remove_reference_t<decltype(__PRETTY_FUNCTION__)>>) - 127};
	}

}
