#pragma once

// Nerealizuojame algoritmų, kurie netikrintų ar masyvas tuščias, nes tą patį galime pasiekti su įprastais algoritmais ir atributo assume naudojimu. Nerealizuojame fixed_string, type_name, log, nes tokį funkcionalumą suteikia žurnalavimo (spdlog) ir tokios kaip nameof bibliotekos. Nerealizuojame savo lexer, nes galime naudoti tiesiog populiarų formatą kaip json. Nerealizuojame print ir read, nes galime naudoti bibliotekas fmt ir scn. Nerealizuojame AA_IF_DEBUG, AA_TRACE_ASSERT, timekeeper, nes jie realizuoti tokiose bibliotekose kaip Boost. Nerealizuojame to_array, nes galime naudoti std::to_array arba std::array<T, 0>{}. Nerealizuojame int_math, nes galime tiesiogiai naudoti math funkcijas (klaidinga bandyti išrašyti math funkcijų visas kombinacijas, o tą ir darėme). Neturime ptr_v ir out_v, nes tokie globalūs kintamieji padidina programos dydį, ko be problemų galima išvengti. Nerealizuojame enum bitwise operacijų, nes galime naudoti biblioteką magic_enum.

// Filosofija bibliotekos tokia, visos funkcijos žymimos constexpr ir tiek. Nesvarbu gali ar negali būti funkcija
// naudojama constexpr kontekste, ji bus pažymėta constexpr. Gal naudotojams kiek neaišku gali būti ar jie gali
// funkciją naudoti constexpr kontekste, bet aš nenoriu mąstyti apie tai ar funkcijos realizacija gali būti constexpr.

// constexpr ir consteval funkcijos specifikatoriai implikuoja inline, todėl nereikia naudoti jų kartu.
// Atitinkamai constexpr kintamojo specifikatorius implikuoja inline ir const specifikatorius.
// Reikia stengtis turėti ir naudoti kuo mažiau macros.
// Reikia stengtis vietoje constexpr kintamųjų naudoti consteval funkcijas, nes tai geriausia alternatyva, kol neturime consteval kintamųjų. NTTPs yra consteval kintamieji.

// Nenaudojame atributo "always_inline", nes standartinėje bibliotekoje atributas nenaudojamas, trukdo derintuvei
// atributas, gali būti, kad tik kenkiu greitaveikai naudodamas tą atributą, geriau už kompiliatorių nenuspręsiu,
// kur jį reikia naudoti, o kur ne, galiausiai išprotėčiau jei dar reiktų prižiūrėti ar visur tvarkingai sudėtas jis.

// Nenaudojame decltype ir auto, bet galime naudoti auto ir decltype(auto) nurodant gražinamos reikšmės tipą, galime naudoti auto nurodant deducing this tipą, galime naudoti auto nurodant kintamojo tipą, jei jis inicializuojamas su explicit konstruktoriumi.
// Nenaudojame const nurodant bevardžių parametrų tuščius tipus, nes tokie parametrai nepanaudojami ir viršutinio lygio const nekeičia funkcijos tipo tai tik sutaupome vietos nerašydami const.

// AA bibliotekos failuose elgiamasi lyg būtų įterpti toliau išdėstyti failai, nes tie failai suteikia galimybę
// sklandžiai programuoti naudojantis esminėmis C++ kalbos savybėmis. Tie failai yra ir jų įterpimo priežastys:
// • <cstddef> ir <cstdint>, failai įterpti, kad nereiktų naudoti daug raktažodžių, kad aprašyti pamatinius tipus.
// • <span> ir <array> (<compare>, <initializer_list>), failai įterpti, kad nereiktų naudoti C stiliaus masyvų.
// • <type_traits> ir <concepts>, failai įterpti, kad išeitų lengvai protauti apie tipus.
// • <utility> (<compare>, <initializer_list>), failas įterptas, kad išeitų lengvai protauti apie išraiškas.
// • <limits>, failas įterptas, kad išeitų lengvai naudotis pamatinių tipų savybėmis.
// • <bit>, failas įterptas, kad išeitų lengvai manipuliuoti pamatinių tipų bitus.
// • <functional>, failas įterptas, kad išeitų lengvai protauti apie funkcijų objektus.
// • <string_view> ir <string> (<compare>, <initializer_list>), failai įterpti, kad nereiktų naudoti C stiliaus teksto eilučių.
// • <new> ir <memory> (<compare>), failai įterpti, kad išeitų lengvai protauti apie atmintį.
// Failai paminėti skliausteliuose prie įterpiamo failo nurodo kokie failai yra įterpiami pačio įterpiamo failo.
// Neįterpiame <variant>, nes mums to failo reikėjo būtinai tik dėl std::monostate, o dabar ta struktūra randasi ir <utility> faile.

// Nenurodome ką iš failų naudojame, nes net jei niekas nebūtų naudojama iš failo, failas turėtų būti įterptas.
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <concepts>
#include <limits>
#include <span>
#include <array>
#include <bit>
#include <utility>
#include <functional>
#include <string_view>
#include <string>
#include <iterator>
#include <ranges>
#include <algorithm>
#include <new>
#include <memory>
#include <optional>

using namespace std::string_view_literals;

// TODO: https://github.com/microsoft/vscode-cpptools/issues/14234
#define AA_EXPAND(identifier) identifier...



namespace aa {

	namespace std_r = std::ranges;



	template<class T>
	concept uses_type = requires { typename std::remove_reference_t<T>::type; };

	template<uses_type T>
	using type_in_use_t = typename std::remove_reference_t<T>::type;

	template<class T>
	concept uses_value_type = requires { typename std::remove_reference_t<T>::value_type; };

	template<uses_value_type T>
	using value_type_in_use_t = typename std::remove_reference_t<T>::value_type;

	template<class T>
	concept uses_reference = requires { typename std::remove_reference_t<T>::reference; };

	template<uses_reference T>
	using reference_in_use_t = typename std::remove_reference_t<T>::reference;

	template<class T>
	concept uses_const_reference = requires { typename std::remove_reference_t<T>::const_reference; };

	template<uses_const_reference T>
	using const_reference_in_use_t = typename std::remove_reference_t<T>::const_reference;

	// https://devblogs.microsoft.com/oldnewthing/20190710-00/?p=102678
	template<class T>
	concept complete = requires { sizeof(T); };

	template<class T>
	concept incomplete = !complete<T>;

	template<class T>
	concept uses_char_type = requires { typename std::remove_reference_t<T>::char_type; };

	template<class T>
	concept uses_int_type = requires { typename std::remove_reference_t<T>::int_type; };

	template<class T>
	concept uses_off_type = requires { typename std::remove_reference_t<T>::off_type; };

	template<class T>
	concept uses_pos_type = requires { typename std::remove_reference_t<T>::pos_type; };

	template<class T>
	concept uses_state_type = requires { typename std::remove_reference_t<T>::state_type; };

	template<class T>
	concept char_traits_like = uses_char_type<T> && uses_int_type<T> && uses_off_type<T> && uses_pos_type<T> && uses_state_type<T>;

	// Netikriname ar tipo reikšmės using yra toks pat kaip jo traits char_type, nes skirtingi tipai gali skirtingų
	// pavadinimų reikšmės using'us turėti ar gali išviso tokio using neturėti. Patiems tipams paliekama tikrintis ar
	// jų reikšmės using'ai sutampa su jų traits char_type, o dėl šio concept tiesiog galima naudoti traits char_type.
	template<class T>
	concept uses_traits_type = (requires { typename std::remove_reference_t<T>::traits_type; })
		&& char_traits_like<typename std::remove_reference_t<T>::traits_type>;

	template<uses_traits_type T>
	using traits_type_in_use_t = typename std::remove_reference_t<T>::traits_type;

	template<uses_char_type T>
	using char_type_in_use_t = typename std::remove_reference_t<T>::char_type;

	template<uses_int_type T>
	using int_type_in_use_t = typename std::remove_reference_t<T>::int_type;

	template<uses_off_type T>
	using off_type_in_use_t = typename std::remove_reference_t<T>::off_type;

	template<uses_pos_type T>
	using pos_type_in_use_t = typename std::remove_reference_t<T>::pos_type;

	template<uses_state_type T>
	using state_type_in_use_t = typename std::remove_reference_t<T>::state_type;

	template<class T>
	concept uses_view_type = requires { typename std::remove_reference_t<T>::view_type; };

	template<uses_view_type T>
	using view_type_in_use_t = typename std::remove_reference_t<T>::view_type;

	template<class T>
	concept uses_allocator_type = requires { typename std::remove_reference_t<T>::allocator_type; };

	template<uses_allocator_type T>
	using allocator_type_in_use_t = typename std::remove_reference_t<T>::allocator_type;

	template<class T>
	concept uses_iterator = requires { typename std::remove_reference_t<T>::iterator; };

	template<uses_iterator T>
	using iterator_in_use_t = typename std::remove_reference_t<T>::iterator;

	template<class... A>
	concept same_as_every = (... && std::same_as<A...[0], A>);

	template<class T, class... A>
	concept same_as_any = (... || std::same_as<T, A>);

	template<class T>
	concept scoped_enum_like = std::regular<T> && std::is_scoped_enum_v<T>;

	template<class T>
	concept enum_like = std::regular<T> && std::is_enum_v<T>;

	template<class T>
	concept unscoped_enum_like = enum_like<T> && !scoped_enum_like<T>;

	template<class T>
	concept pointer_like = std::is_pointer_v<T>;

	template<class T>
	concept function_like = std::is_function_v<T>;

	template<class T>
	concept class_like = std::is_class_v<T>;

	template<class T, class... A>
	concept movable_constructible_from = std::movable<T> && std::constructible_from<T, A...>;

	template<class T>
	concept const_like = std::is_const_v<T>;

	template<class T>
	concept not_const = !const_like<T>;

	template<class T>
	concept empty_like = std::is_empty_v<T>;

	template<class T>
	concept not_const_not_movable = not_const<T> && !std::movable<T>;

	template<class T>
	concept wo_cv_movable = std::movable<std::remove_cv_t<T>>;

	template<class T>
	concept value_type_movable = std::movable<value_type_in_use_t<T>>;

	template<class T>
	concept const_movable_defaultable = const_like<T> && movable_constructible_from<std::remove_const_t<T>>;

	template<class T>
	concept empty_const_movable_defaultable = empty_like<T> && const_movable_defaultable<T>;

	template<class T>
	concept reference_like = std::is_reference_v<T>;

	template<class T>
	concept not_reference = !reference_like<T>;

	template<class T>
	concept not_cref = not_reference<T> && not_const<T>;

	template<class T>
	concept lvalue_reference_like = std::is_lvalue_reference_v<T>;

	template<class T>
	concept not_lvalue_reference_like = !lvalue_reference_like<T>;

	template<class T>
	concept rvalue_reference_like = std::is_rvalue_reference_v<T>;

	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<class T>
	concept uniquely_representable = std::has_unique_object_representations_v<T>;

	template<class T>
	concept trivially_copyable = std::is_trivially_copyable_v<T>;

	template<class L, class R>
	concept wo_cvref_same_as = std::same_as<std::remove_cvref_t<L>, R>;

	template<class F, class... A>
	concept cref_predicate = std::predicate<const F &, A...>;

	// TODO: https://github.com/cplusplus/papers/issues/1546
	template<class F, class U, class V>
	concept cref_relation = std::relation<const F &, U, V>;

	template<class F, class... A>
	concept cref_invocable = std::invocable<const F &, A...>;

	template<class F, class... A>
	concept invocable_with_one_of = (... || std::invocable<F, A>);

	template<class F, class R, class... A>
	concept invocable_r = std::is_invocable_r_v<R, F, A...>;

	template<class F, class R, class... A>
	concept cref_invocable_r = std::is_invocable_r_v<R, const F &, A...>;

	template<class F, class R, class... A>
	concept invocable_not_r = std::invocable<F, A...> && !invocable_r<R, F, A...>;

	template<class F, class T>
	concept constructible_to = std::constructible_from<T, F>;

	template<class F, class T>
	concept cref_constructible_to = std::constructible_from<T, const F &>;

	template<class F, class T>
	concept constructible_between = (std::constructible_from<T, F> && std::constructible_from<F, T>);

	template<class T, class U>
	concept different_from = std::__detail::__different_from<T, U>;

	template<class T>
	concept placeholder_like = !!std::is_placeholder_v<T>;

	// https://en.cppreference.com/w/cpp/concepts/boolean-testable.
	template<class B>
	concept boolean_testable = std::__detail::__boolean_testable<B>;

	template<class T, class U>
	concept weakly_equality_comparable_with = std::__detail::__weakly_eq_cmp_with<T, U>;

	template<class T, class U>
	concept partially_ordered_with = std::__detail::__partially_ordered_with<T, U> && weakly_equality_comparable_with<T, U>;

	template<class R, class L>
	concept assignable_to = std::assignable_from<L, R>;

	template<class T, class... F>
	concept constructible_from_every = (... && std::constructible_from<T, F>);

	template<class T>
	concept constructible_from_floating = constructible_from_every<T, float, double, long double>;

	// https://en.wikipedia.org/wiki/Function_object
	template<class T>
	concept functor = requires { &T::operator(); };

	template<class T>
	concept iterator_tag_like = same_as_any<T, std::input_iterator_tag, std::output_iterator_tag, std::forward_iterator_tag,
		std::bidirectional_iterator_tag, std::random_access_iterator_tag, std::contiguous_iterator_tag>;

	template<class T>
	concept uses_iterator_category = (requires { typename std::remove_reference_t<T>::iterator_category; })
		&& iterator_tag_like<typename std::remove_reference_t<T>::iterator_category>;

	template<uses_iterator_category T>
	using iterator_category_in_use_t = typename std::remove_reference_t<T>::iterator_category;

	template<class T>
	concept uses_iterator_concept = (requires { typename std::remove_reference_t<T>::iterator_concept; })
		&& iterator_tag_like<typename std::remove_reference_t<T>::iterator_concept>;

	template<uses_iterator_concept T>
	using iterator_concept_in_use_t = typename std::remove_reference_t<T>::iterator_concept;

	template<class T>
	concept uses_pointer = requires { typename std::remove_reference_t<T>::pointer; };

	template<uses_pointer T>
	using pointer_in_use_t = typename std::remove_reference_t<T>::pointer;

	template<class T>
	concept uses_const_pointer = requires { typename std::remove_reference_t<T>::const_pointer; };

	template<uses_const_pointer T>
	using const_pointer_in_use_t = typename std::remove_reference_t<T>::const_pointer;

	template<class T>
	concept uses_difference_type = requires { typename std::remove_reference_t<T>::difference_type; };

	template<uses_difference_type T>
	using difference_type_in_use_t = typename std::remove_reference_t<T>::difference_type;

	template<class T>
	concept uses_size_type = requires { typename std::remove_reference_t<T>::size_type; };

	template<uses_size_type T>
	using size_type_in_use_t = typename std::remove_reference_t<T>::size_type;

	template<class T, not_cref U>
	using copy_const_t = std::conditional_t<const_like<std::remove_reference_t<T>>, const U, U>;

	template<std::integral T, std::signed_integral U>
	using copy_unsigned_t = std::conditional_t<std::unsigned_integral<T>, std::make_unsigned_t<U>, U>;

	template<std::integral T, std::unsigned_integral U>
	using copy_signed_t = std::conditional_t<std::signed_integral<T>, std::make_signed_t<U>, U>;

	template<class T, class U>
	using forward_like_t = std::__like_t<T, U>;

	template<class T>
	using add_const_lvalue_ref_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

	template<class T>
	using add_const_rvalue_ref_t = std::add_rvalue_reference_t<std::add_const_t<T>>;

	template<not_cref T>
	[[nodiscard, gnu::always_inline]]
	constexpr std::add_lvalue_reference_t<T> as(std::add_lvalue_reference_t<T> x) { return x; }

	template<not_cref T>
	[[nodiscard, gnu::always_inline]]
	constexpr aa::add_const_lvalue_ref_t<T> as(aa::add_const_lvalue_ref_t<T> x) { return x; }

	template<not_cref T>
	[[nodiscard, gnu::always_inline]]
	constexpr std::add_rvalue_reference_t<T> as(std::add_rvalue_reference_t<T> x) { return std::move(x); }

	template<not_cref T>
	[[nodiscard, gnu::always_inline]]
	constexpr aa::add_const_rvalue_ref_t<T> as(aa::add_const_rvalue_ref_t<T> x) { return std::move(x); }

	// Nieko tokio, kad kopijuojame konstantas, kadangi viskas vyksta kompiliavimo metu.
	//
	// Negalime konstantų pakeisti funkcijomis, nes neišeina gauti adreso funkcijos
	// rezultato. Tai reiškia, kad šitoks sprendimas yra universalesnis.
	//
	// constant_t alias neturėtų prasmės, nes, kad juo naudotis jau reiktų nurodyti ką norime gauti.
	template<auto V>
	using t = decltype(V);

	template<class U>
	struct converter_base_t {
		// Vietoj operatorių apibrėžimo galėtume paveldėti iš monostate, bet kitur negalime paveldėti ir nepaveldime tai čia taip pat taip elgiamės, kad būtume nuoseklūs. Taip pat nebūtų teisinga paveldėti, nes vietoje monostate galėtų būtų naudojamos šios klasės.
		// Turime apibrėžti ir operator==, nes todėl, kad turime kitą operator==, iš operator<=> nėra sukuriamas automatiškai tas operatorius.
		friend consteval bool operator==(converter_base_t, converter_base_t) = default;
		friend consteval std::strong_ordering operator<=>(converter_base_t, converter_base_t) = default;

		// Reikia funkcijų ir šios klasės apskritai, nes be jos, getter'ių negalima lyginti tik su fundamental tipais kažkodėl.
		// Comparison operators
		template<different_from<U> T>
		friend constexpr bool operator==(const U self, const T & t) { return self.operator T() == t; }

		template<different_from<U> T>
		friend constexpr auto operator<=>(const U self, const T & t) { return self.operator T() <=> t; }

		// Arithmetic operators
		template<different_from<U> T>
		friend constexpr decltype(auto) operator+(const U self, const T & t) { return self.operator T() + t; }
		template<different_from<U> T>
		friend constexpr decltype(auto) operator+(const T & t, const U self) { return t + self.operator T(); }

		template<different_from<U> T>
		friend constexpr decltype(auto) operator-(const U self, const T & t) { return self.operator T() - t; }
		template<different_from<U> T>
		friend constexpr decltype(auto) operator-(const T & t, const U self) { return t - self.operator T(); }

		template<different_from<U> T>
		friend constexpr decltype(auto) operator*(const U self, const T & t) { return self.operator T() * t; }
		template<different_from<U> T>
		friend constexpr decltype(auto) operator*(const T & t, const U self) { return t * self.operator T(); }

		template<different_from<U> T>
		friend constexpr decltype(auto) operator/(const U self, const T & t) { return self.operator T() / t; }
		template<different_from<U> T>
		friend constexpr decltype(auto) operator/(const T & t, const U self) { return t / self.operator T(); }

		template<different_from<U> T>
		friend constexpr decltype(auto) operator%(const U self, const T & t) { return self.operator T() % t; }
		template<different_from<U> T>
		friend constexpr decltype(auto) operator%(const T & t, const U self) { return t % self.operator T(); }

		template<different_from<U> T>
		friend constexpr decltype(auto) operator&(const U self, const T & t) { return self.operator T() & t; }
		template<different_from<U> T>
		friend constexpr decltype(auto) operator&(const T & t, const U self) { return t & self.operator T(); }

		template<different_from<U> T>
		friend constexpr decltype(auto) operator|(const U self, const T & t) { return self.operator T() | t; }
		template<different_from<U> T>
		friend constexpr decltype(auto) operator|(const T & t, const U self) { return t | self.operator T(); }

		template<different_from<U> T>
		friend constexpr decltype(auto) operator^(const U self, const T & t) { return self.operator T() ^ t; }
		template<different_from<U> T>
		friend constexpr decltype(auto) operator^(const T & t, const U self) { return t ^ self.operator T(); }
	};

	// T tipas nesiskiria kai esame const T & kontekste ar T kontekste. Tai reiškia, kad
	// privalome pasirinkti grąžinti const T & arba T visiems atvejams. Yra pasirinkta grąžinti T,
	// nes buvo nuspręsta, kad šias klases turi būti įmanoma naudoti ir ne su constexpr konstruktorius
	// turinčiomis klasėmis ir joms neišeitų grąžinti const T &. Jei vis dėlto reikia const T &
	// tipo, galima dirbti su atitinkamais constexpr kintamaisiais, kurie yra apibrėžti po šių klasių.
	template<auto... A>
	struct converter_to_value_t : converter_base_t<converter_to_value_t<A...>> {
		// T turi būti movable, nes mums reikia žinoti ar galime iš gražinamos reikšmės sukonstruoti T.
		// consteval, nes taip priversime naudotojus patiems sukonstruoti reikšmę, jei jos neišeina sukonstruoti kompiliavimo metu.
		template<movable_constructible_from<const t<A> &...> T>
		consteval operator T() const { return T{A...}; }
	};

	using converter_to_default_value_t = converter_to_value_t<>;

	template<bool MAX>
	struct converter_to_numeric_extrema_t : converter_base_t<converter_to_numeric_extrema_t<MAX>> {
		// consteval for the same reason as above.
		template<std::movable T>
		consteval operator T() const {
			if constexpr (MAX)	return std::numeric_limits<T>::max();
			else				return std::numeric_limits<T>::min();
		}
	};

	using converter_to_numeric_max_t = converter_to_numeric_extrema_t<true>;
	using converter_to_numeric_min_t = converter_to_numeric_extrema_t<false>;

	// Galėtų struktūra būti pakeista globaliu kintamuoju, bet tada padidėtų programos dydis ir mes su šia realizacija neprarandame greitaveikos.
	template<uintptr_t N>
	struct converter_to_address_t : converter_base_t<converter_to_address_t<N>> {
		template<class T>
		constexpr operator T * () const {
			if constexpr (!!N)	return std::bit_cast<T *>(N);
			else				return nullptr;
		}
	};

	template<auto... A>
	constexpr converter_to_value_t<A...> value;
	constexpr converter_to_default_value_t default_value;

	template<bool MAX>
	constexpr converter_to_numeric_extrema_t<MAX> numeric_extrema;
	constexpr converter_to_numeric_max_t numeric_max;
	constexpr converter_to_numeric_min_t numeric_min;

	template<uintptr_t N>
	constexpr converter_to_address_t<N> address;

	// Išmeta klaidą parašius šią išraišką constant<AAA{}>, jei tik movable uždėtas constraint.
	// struct AAA {
	// 	constexpr AAA & operator=(AAA &&) = default;
	// 	constexpr AAA(AAA &&) = default;
	// 	constexpr AAA(const AAA &) = delete;
	// 	constexpr AAA() = default;
	// };
	template<std::copyable auto V>
	using constant = std::integral_constant<t<V>, V>;

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2781r9.html
	template<std::copyable T>
	consteval T c(const T value = default_value) {
		return value;
	}

	template<class T>
	consteval std::type_identity<T> c_type() {
		return default_value;
	}

	struct noop {
		static consteval void operator()() {}
	};

	template<auto V>
	using type_in_c_t = type_in_use_t<t<V>>;

	// <><><><><><><><><><><><><><><><><><><><><><><><> STR RANGES <><><><><><><><><><><><><><><><><><><><><><><><>

	template<class B, class I>
	concept bool_auxiliary_range_for = (std_r::random_access_range<B> && std_r::output_range<B, bool>
		&& boolean_testable<std_r::range_reference_t<B>> && constructible_between<I, std_r::range_difference_t<B>>);

	template<class R>
	concept sized_random_access_range = std_r::random_access_range<R> && std_r::sized_range<R>;

	template<class T>
	concept sized_contiguous_range = std_r::contiguous_range<T> && std_r::sized_range<T>;

	template<class T>
	concept sized_input_range = std_r::input_range<T> && std_r::sized_range<T>;

	template<class R>
	concept not_range = !std_r::range<R>;

	template<class R>
	concept permutable_range = (sized_random_access_range<R> && std::permutable<std_r::iterator_t<R>>);

	template<class I>
	using iter_size_t = std::make_unsigned_t<std::iter_difference_t<I>>;

	template<class I>
	using iter_pointer_t = std::add_pointer_t<std::iter_reference_t<I>>;

	template<class R>
	using range_pointer_t = std::add_pointer_t<std_r::range_reference_t<R>>;

	template<class T>
	concept reverse_range = requires(T & t) {
		std_r::rbegin(t);
		std_r::rend(t);
	};

	// Galėtume realizuoti taip: function_result_t<t<&t<std_r::rbegin>::operator()<T &>>>;
	// Bet yra draudžiama gauti std funkcijos adresą: https://en.cppreference.com/w/cpp/language/extending_std.html.
	// Jeigu jau bandytume tai daryt tai reiktų naudoti static_cast, kad gauti tinkamą užklojimą, ir mes to nenorime daryti.
	template<class T>
	using reverse_iterator_t = std::invoke_result_t<t<std_r::rbegin>, T &>;

	template<reverse_range R>
	using reverse_sentinel_t = std::invoke_result_t<t<std_r::rend>, R &>;

	template<class T>
	concept member_back_exists = requires(T & t) { { t.back() } -> std::same_as<std_r::range_reference_t<T>>; };

	template<class R>
	concept prev_end_accessible_by_end = std_r::bidirectional_range<R>
		&& std::constructible_from<std_r::iterator_t<R>, std_r::sentinel_t<R>>;

	template<class R>
	concept prev_end_accessible_by_back = member_back_exists<R>
		&& std::constructible_from<std_r::iterator_t<R>, range_pointer_t<R>>;

	// common_range negalime naudoti, nes mano ranges nėra common.
	template<class R>
	concept prev_end_accessible_range = prev_end_accessible_by_back<R>
		|| prev_end_accessible_by_end<R> || sized_random_access_range<R>;

	// Nėra atitinkamos funkcijos rend iteratoriui, nes jis nėra svarbus.
	// get_rdata nėra funkcijos, nes iš šios funkcijos galima gauti rodyklę.
	// Čia turėtų būti O(1) sudėtingumo funkcija, todėl netinka naudoti distance.
	// Algoritmai priima ranges kur tik įmanoma. Jei turime tik iteratorius tai galima paduoti iš jų sukurtą range.
	template<prev_end_accessible_range R>
	constexpr std_r::iterator_t<R> get_prev_end(R && r) {
		/*  */ if constexpr (prev_end_accessible_by_back<R>) {
			return std_r::iterator_t<R>{std::addressof(r.back())};

		} else if constexpr (prev_end_accessible_by_end<R>) {
			return std_r::prev(std_r::iterator_t<R>{std_r::end(r)});

		} else {
			return std_r::next(std_r::begin(r), std_r::ssize(r) - 1);
		}
	}

	template<class S, class I>
	concept comparable_distance_sentinel_for = std::sentinel_for<S, I> && requires(const I & i, const S & s) {
		{ i - s } -> partially_ordered_with<std::iter_difference_t<I>>;
		{ s - i } -> partially_ordered_with<std::iter_difference_t<I>>;
	};

	template<bool POSITIVE>
	struct infinity_t {
		friend consteval bool operator==(infinity_t, infinity_t) = default;
		friend consteval std::strong_ordering operator<=>(infinity_t, infinity_t) = default;


		template<different_from<infinity_t> T>
		friend consteval bool operator==(infinity_t, const T &) { return false; }

		template<different_from<infinity_t> T>
		friend consteval std::strong_ordering operator<=>(infinity_t, const T &) {
			if constexpr (POSITIVE) return std::strong_ordering::greater; else return std::strong_ordering::less;
		}


		template<different_from<infinity_t> T>
		friend consteval infinity_t<POSITIVE> operator-(infinity_t, const T &) { return default_value; }

		template<different_from<infinity_t> T>
		friend consteval infinity_t<!POSITIVE> operator-(const T &, infinity_t) { return default_value; }


		template<different_from<infinity_t> T>
		friend consteval infinity_t<POSITIVE> operator+(infinity_t, const T &) { return default_value; }

		template<different_from<infinity_t> T>
		friend consteval infinity_t<POSITIVE> operator+(const T &, infinity_t) { return default_value; }


		friend consteval infinity_t<POSITIVE> operator+(infinity_t) { return default_value; }
		friend consteval infinity_t<!POSITIVE> operator-(infinity_t) { return default_value; }
	};

	using positive_infinity_t = infinity_t<true>;
	using negative_infinity_t = infinity_t<false>;

	template<bool TOP>
	constexpr infinity_t<TOP> infinity;
	constexpr positive_infinity_t positive_infinity;
	constexpr negative_infinity_t negative_infinity;

	template<std::random_access_iterator I, std::sized_sentinel_for<I> S>
	constexpr I get_next(const I i, const S bound) {
		return get_next(i, numeric_max, bound);
	}

	template<std::random_access_iterator I, comparable_distance_sentinel_for<I> S>
	constexpr I get_next(const I i, const std::iter_difference_t<I> n, const S bound = positive_infinity) {
		const auto difference = bound - i;

		if (n >= difference) {
			/**/ if constexpr (std::constructible_from<I, S>)		return bound;
			else if constexpr (std::sized_sentinel_for<S, I>)		return i + difference;
			else for (I j = i; true; ++j) if (j == bound)			return j;

		} else
			return i + n;
	}

	template<std::random_access_iterator I, std::sized_sentinel_for<I> S>
	constexpr I get_prev(const I i, const S bound) {
		return get_prev(i, numeric_max, bound);
	}

	template<std::random_access_iterator I, comparable_distance_sentinel_for<I> S>
	constexpr I get_prev(const I i, const std::iter_difference_t<I> n, const S bound = negative_infinity) {
		const auto difference = i - bound;

		if (n >= difference) {
			/**/ if constexpr (std::constructible_from<I, S>)		return bound;
			else if constexpr (std::sized_sentinel_for<S, I>)		return i - difference;
			else for (I j = i; true; --j) if (j == bound)			return j;

		} else
			return i - n;
	}

	// <><><><><><><><><><><><><><><><><><><><><><><><> END RANGES <><><><><><><><><><><><><><><><><><><><><><><><>

	// Apskritai šitokių esybių reikia, nes ne constexpr kontekstuose, mes norime būti tikri, kad mes naudojame constexpr reikšmes. constexpr kontekstuose nėra būtina šitokių esybių naudoti.
	// Dabar neišeina naudoti default_v su fundamentaliais tipais, bet tokiais atvejais galima naudoti value_v.
	template<std::movable T>
	consteval size_t numeric_digits() {
		return std::numeric_limits<T>::digits;
	}



	// Kartais patogiau naudoti lambda su generic tipo parametru, bet ne atvejais, kai skiriasi parametrų skaičiai.
	// Negalime tikrinti ar F turi operator(), nes gal ta funkcija yra templated ir mes neturime kaip to patikrinti.
	// https://www.cppstories.com/2019/02/2lines3featuresoverload.html
	template<class... F>
	struct overload : F... {
		using F::operator()...;
	};

	// Lambdos ne consteval, nes nereikia constexpr kontekstuose to žymėti. Analogiškos lambdos taip pat neturi tokio specifikatoriaus.
	template<class F>
	using function_t = type_in_c_t<overload{
		([]<bool N, class R, class... A>(std::type_identity<R(A...) noexcept(N)>) static { return c_type<R(A...)>(); }),
		([]<bool N, class R, class... A>(std::type_identity<R(A...) & noexcept(N)>) static { return c_type<R(A...)>(); }),
		([]<bool N, class R, class... A>(std::type_identity<R(A...) && noexcept(N)>) static { return c_type<R(A...)>(); }),
		([]<bool N, class R, class... A>(std::type_identity<R(A...) const noexcept(N)>) static { return c_type<R(A...)>(); }),
		([]<bool N, class R, class... A>(std::type_identity<R(A...) const & noexcept(N)>) static { return c_type<R(A...)>(); }),
		([]<bool N, class R, class... A>(std::type_identity<R(A...) const && noexcept(N)>) static { return c_type<R(A...)>(); }),
		([]<function_like U>(this const auto lambda, std::type_identity<U *>) { return lambda(c_type<U>()); }),
		([]<class U, class T>(this const auto lambda, std::type_identity<U T:: *>) {
			if constexpr (function_like<U>)	return lambda(c_type<U>());
			else							return lambda(c_type<U()>());
		}),
		([]<functor T>(this const auto lambda, std::type_identity<T>) { return lambda(c_type<t<&T::operator()>>()); })
	}(c_type<std::remove_cvref_t<F>>())>;

	template<class F>
	concept function_type_deducible = requires { typename function_t<F>; };

	template<function_type_deducible F, size_t I = 0>
	using function_argument_t = type_in_c_t<([]<class R, class... A>(std::type_identity<R(A...)>) static {
		if constexpr (I < sizeof...(A)) {
			return c_type<A...[I]>();
		} else {
			return c_type<void>();
		}
	})(c_type<function_t<F>>())>;

	template<function_type_deducible F>
	using function_result_t = type_in_c_t<([]<class R, class... A>(std::type_identity<R(A...)>)
		static { return c_type<R>(); })(c_type<function_t<F>>())>;

	template<class F>
	consteval size_t get_function_arity() {
		if constexpr (function_type_deducible<F>) {
			return ([]<class R, class... A>(std::type_identity<R(A...)>) static { return sizeof...(A); })(c_type<function_t<F>>());
		} else {
			return numeric_max;
		}
	}



	namespace detail {
		template<size_t, class T>
		struct tuple_unit {
			// Member types
			using value_type = T;
			using reference = value_type &;
			using const_reference = const value_type &;
			using pointer = std::remove_reference_t<value_type> *;
			using const_pointer = const std::remove_reference_t<value_type> *;

			// Comparisons
			friend constexpr auto operator<=>(const tuple_unit &, const tuple_unit &) = default;

			// Special member functions
			constexpr tuple_unit() = default;

			constexpr tuple_unit() requires (const_movable_defaultable<value_type>) : value{default_value} {}

#pragma GCC diagnostic push // GCC BUG, meta klaidą, kur jos nėra, naudojant agregate tipus. Peržiūrėti thank_you_2025.
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
			template<constructible_to<value_type> U = value_type>
			constexpr tuple_unit(U && u) : value{std::forward<U>(u)} {}
#pragma GCC diagnostic pop

			// Member objects
			// Atributas no_unique_address sumažina tuple dydį, kai naudojami tušti objektai, bet nesumažina iki minimalaus dydžio.
			// Pvz: 'sizeof(aa::tuple<std::monostate, int, std::monostate, int, std::monostate, double, std::monostate>)', šio tuple dydis be atributo yra 40 baitų, su atributu dydis tampa 24 baitai, o minimalus dydis yra 16 baitų.
			template<class H>
			constexpr auto && operator()(this H && self) {
				return std::forward<H>(self).value;
			}

			[[no_unique_address]] value_type value;
		};

		// Negalime paveldėti iš T ar iš kokio kito tuščio tipo, nes kažkodėl tada tuple dydis padidėja.
		template<size_t I, empty_const_movable_defaultable T>
		struct tuple_unit<I, T> {
			// Member types
			using value_type = T;
			using reference = value_type;
			using const_reference = value_type;
			using pointer = value_type *;
			using const_pointer = value_type *;

			// Comparisons
			friend consteval std::strong_ordering operator<=>(tuple_unit, tuple_unit) = default;

			// Member objects
			static consteval value_type operator()() { return default_value; }

			// Special member functions
			constexpr tuple_unit() = default;

			template<constructible_to<value_type> U = value_type>
			constexpr tuple_unit(U &&) {}
		};
	}

	template<class T>
	concept complete_tuple_size = complete<std::tuple_size<std::remove_reference_t<T>>>;

	template<complete_tuple_size T>
	consteval size_t tuple_size() { return std::tuple_size_v<std::remove_reference_t<T>>; }

	template<class T, size_t I>
	concept complete_tuple_element = complete<std::tuple_element<I, std::remove_reference_t<T>>>;

	template<size_t I, complete_tuple_element<I> T>
	using tuple_element_t = std::tuple_element_t<I, std::remove_reference_t<T>>;



	// https://docs.libreoffice.org/o3tl/html/temporary_8hxx_source.html
	template<not_lvalue_reference_like T>
	constexpr T & stay(T && x = default_value) {
		// Turime atlikti cast, kitaip meta klaidą.
		return std::forward<T &>(x);
	}

	// T čia neturi būti tuple_like, nes tuple_like tipo visi get validūs, o čia tikrinamas tik vienas get.
	template<class T, size_t I>
	concept member_get_exists = requires(T && t) {
		std::forward<T>(t).template get<I>();
	};

	template<class T, size_t I>
	concept adl_get_exists = requires(T && t) {
		get<I>(std::forward<T>(t));
	};

	template<class T, size_t I>
	concept gettable = member_get_exists<T, I> || adl_get_exists<T, I>;

	// Negalime turėti funkcijos, nes neišeina turėti function aliases patogių.
	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2769r1.html
	template<size_t I>
	constexpr auto get_element = []<gettable<I> T>(T && t) static -> decltype(auto) {
		if constexpr (member_get_exists<T, I>)	return std::forward<T>(t).template get<I>();
		else									return get<I>(std::forward<T>(t));
	};

	constexpr t<get_element<0>> get_0, get_x, get_key;
	constexpr t<get_element<1>> get_1, get_y, get_val;
	constexpr t<get_element<2>> get_2, get_z;
	constexpr t<get_element<3>> get_3;

	template<size_t I, gettable<I> T>
	using get_result_t = std::invoke_result_t<t<get_element<I>>, T>;

	template<class F, auto... A>
	using call_template_t = t<&std::remove_cvref_t<F>::template operator()<A...>>;

	template<auto... A>
	consteval auto get_call() {
		return ([]<class F>(const F &) static consteval { return &F::template operator()<A...>; });
	}

	template<class... A>
	consteval auto get_call() {
		return ([]<class F>(const F &) static consteval { return &F::template operator()<A...>; });
	}

	consteval auto get_call() {
		return ([]<class F>(const F &) static consteval { return &F::template operator()<>; });
	}

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2989r2.pdf
	// Nepaverčiame konstantos į consteval funkciją, nes tada dar turėtume apibrėžti constexpr funkciją tai vienu ir kitu atveju turėtume tiek pat materialių esybių.
	template<auto C>
	constexpr auto invoke = []<class F, class... A>(F && f = default_value, A &&... args) static -> decltype(auto) {
		/*  */ if constexpr (std::is_member_function_pointer_v<t<C(f)>>) {
			return (std::forward<F>(f).*C(f))(std::forward<A>(args)...);

		} else if constexpr (std::is_member_object_pointer_v<t<C(f)>> && (sizeof...(A) == 0)) {
			return std::forward<F>(f).*C(f);

		} else {
			return C(f)(std::forward<A>(args)...);
		}};

	// Nesijaudinant galima naudoti tag tipus kaip funkcijų parametrus.
	// https://www.fluentcpp.com/2021/03/05/stdindex_sequence-and-its-improvement-in-c20/
	// Jei lambdą iškeltume į funkciją, tai ji jokio funkcionalumo nesuteiktų, nes patogiau kiekvienu atveju būtų ne ją naudoti, o atitinkamą invoke.
	// apply ir invoke funktoriai, o ne funkcijos, nes sutaupome vieną funkcijos iškvietimą:
	// Funkcijos atvejis: apply -> invoke -> f
	// Funktoriaus atvejis: apply (invoke) -> f
	// Taip pat mažiau kompiliatoriui tenka generuoti esybių, nes pernaudojami template parametrai.
	template<size_t N>
	constexpr auto apply = invoke<([] static {
		if constexpr (!N) return get_call(); else return get_call<__integer_pack(N)...>();
	})()>;

	// Negalime tiesiog naudoti __integer_pack(N)... vietoje I, nes meta klaidą:
	// sorry, unimplemented: '__integer_pack(N)' is not the entire pattern of the pack expansion
	template<class F, class... A>
	constexpr decltype(auto) greedy_invoke(F && f = default_value, A &&... args) {
		if constexpr (get_function_arity<F>() < sizeof...(A)) {
			return apply<get_function_arity<F>()>([&]<size_t... I> -> decltype(auto) {
				return std::invoke(std::forward<F>(f), std::forward<A...[I]>(AA_EXPAND(args)[I])...); });
		} else
			return std::invoke(std::forward<F>(f), std::forward<A>(args)...);
	}

	template<class T, size_t N = 0>
	concept tuple_like = (tuple_size<T>() >= N) && apply<tuple_size<T>()>(
		[]<size_t... I> static { return (... && std::constructible_from<tuple_element_t<I, T>, get_result_t<I, T>>); });

	template<class F, class T, class... A>
		requires (std::invocable<F, T &, A...>)
	constexpr T & extend(T & t, F && f = default_value, A &&... args) {
		std::invoke(std::forward<F>(f), t, std::forward<A>(args)...);
		return t;
	}

	template<class F, class T, size_t... I>
	concept tuple_constructible_to = tuple_like<F, tuple_size<T>()>
		&& (... && std::constructible_from<tuple_element_t<I, T>, get_result_t<I, F>>);



	// Naudojame constexpr algoritmą, kad surasti indeksą, nes lygiai taip pat ir GCC daro.
	template<auto A, std::equality_comparable_with<t<A>> auto... V>
	constexpr size_t pack_index_v = pack_index_v<true, (A == V)...>;

	template<bool A, bool... V>
	constexpr size_t pack_index_v<A, V...> = get_key(*std_r::find(std::views::enumerate(stay(std::array{V...})), A, get_val));

	template<class U, class... T>
	constexpr size_t type_pack_index_v = pack_index_v<true, std::same_as<U, T>...>;

	template<class, class...>
	struct basic_tuple;

	// https://danlark.org/2020/04/13/why-is-stdpair-broken/
	// https://en.wikipedia.org/wiki/Tuple
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
	template<size_t... S, class... T>
	struct basic_tuple<std::index_sequence<S...>, T...> : detail::tuple_unit<S, T>... {
#pragma GCC diagnostic pop
		// Member types
		using tuple_type = basic_tuple;

		template<size_t I>
		using unit_type = detail::tuple_unit<I, T...[I]>;

		template<size_t I>
		using value_type = value_type_in_use_t<unit_type<I>>;

		template<size_t I>
		using reference = reference_in_use_t<unit_type<I>>;

		template<size_t I>
		using const_reference = const_reference_in_use_t<unit_type<I>>;

		template<size_t I>
		using pointer = pointer_in_use_t<unit_type<I>>;

		template<size_t I>
		using const_pointer = const_pointer_in_use_t<unit_type<I>>;

		// Member constants
		template<class U>
		static consteval size_t index() { return type_pack_index_v<U, T...>; }

		// Capacity
		static consteval size_t tuple_size() { return sizeof...(T); }

		// Comparisons
		friend constexpr auto operator<=>(const tuple_type &, const tuple_type &) = default;

		// Element access
		template<size_t I, class H>
		constexpr auto && operator[](this H && self, constant<I>) {
			return std::forward<H>(self).template get<I>();
		}

		template<size_t I, class H>
		constexpr auto && get(this H && self) {
			return std::forward<H>(self).unit_type<I>::operator()();
		}

		template<class U, class H>
		constexpr auto && operator[](this H && self, std::type_identity<U>) {
			return std::forward<H>(self).template get<U>();
		}

		template<class U, class H>
		constexpr auto && get(this H && self) {
			return std::forward<H>(self).unit_type<index<U>()>::operator()();
		}

		// Special member functions
		template<constructible_to<value_type<S>>... U>
			requires (!!sizeof...(S))
		constexpr basic_tuple(U &&... u) : unit_type<S>{std::forward<U>(u)}... {}

		template<size_t... I, constructible_to<value_type<I>>... U>
			requires (!!sizeof...(I))
		constexpr basic_tuple(std::index_sequence<I...>, U &&... u) : unit_type<I>{std::forward<U>(u)}... {}

		// Pvz: tuple<unit<int>> a = tuple{tuple{unit{1}}}; Neįmanoma šio konstruktoriaus sumaišyti su kitu konstruktoriumi.
		template<tuple_constructible_to<tuple_type, S...> U>
		constexpr basic_tuple(U && u) : unit_type<S>{get_element<S>(std::forward<U>(u))}... {}

		constexpr basic_tuple() = default;
	};

	template<class... T>
	using tuple = basic_tuple<std::index_sequence_for<T...>, T...>;

	// Reikia deduction guide, kitaip nesikompiliuos toks kodas: aa::tuple _ = {2, 4u, 5.0, 8.f};
	template<class... T>
	basic_tuple(T &&...) -> tuple<T...>;

	template<class T1>
	using unit = tuple<T1>;

	template<class T1, class T2 = T1>
	using pair = tuple<T1, T2>;

	template<class T1, class T2 = T1, class T3 = T2>
	using triplet = tuple<T1, T2, T3>;

	template<class T1, class T2 = T1, class T3 = T2, class T4 = T3>
	using quartet = tuple<T1, T2, T3, T4>;

	template<class T1, class T2 = T1, class T3 = T2, class T4 = T3, class T5 = T4>
	using quintet = tuple<T1, T2, T3, T4, T5>;

	template<class T1, class T2 = T1, class T3 = T2, class T4 = T3, class T5 = T4, class T6 = T5>
	using sextet = tuple<T1, T2, T3, T4, T5, T6>;



	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2098r1.pdf
	template<class T, template<class...> class F>
	concept specialization_of = ([]<class... A>(std::type_identity<F<A...>>) static { return true; })(c_type<std::remove_cvref_t<T>>());

	template<sized_contiguous_range T>
	using range_char_traits_t = type_in_c_t<([] static {
		if constexpr (uses_traits_type<T>) {
			return c_type<traits_type_in_use_t<T>>();
		} else {
			// Darome prielaidą, kad char_traits yra apibrėžtas su visais tipais.
			return c_type<std::char_traits<std_r::range_value_t<T>>>();
		}
	})()>;

	template<class T, class U>
	concept same_range_char_traits_as = std::same_as<range_char_traits_t<T>, U>;

	template<class T>
	concept range_uses_traits_type = sized_contiguous_range<T> &&
		std::same_as<char_type_in_use_t<range_char_traits_t<T>>, std_r::range_value_t<T>>;

	template<class T>
	concept regular_unsigned_integral = std::unsigned_integral<T> && std::has_single_bit(numeric_digits<T>());

	// Nors galėtume paveldėti tiesiog iš first, bet to nedarome, kad nekurti nereikalingų paveldėjimo ryšių.
	template<class T, class... A>
	using first_not_t = A...[pack_index_v<false, std::same_as<T, A>...>];

	// Reikia using šio, nes testavimui reikėjo sukurti tuple su 100 elementų ir nėra variantas turėti 100 using'ų.
	template<template<class...> class T, auto F, size_t N>
	using filled_t = type_in_c_t<apply<N>([]<size_t... I>
		static { return c_type<T<type_in_c_t<invoke<get_call<I>()>(F)>...>>(); })>;

	// GCC 15.1.0 BUG: ICE with 'return (... && requires { requires std::constructible_from<typename T::value_type<I>, get_result_t<I, T>>; })'
	// Netikriname ar 'std::constructible_from<typename T::value_type<I>, get_result_t<I, T>>', nes tai patikrins tuple_like.
	template<class T>
	concept new_tuple_like = (requires { { T::tuple_size() } -> wo_cvref_same_as<size_t>; })
		&& apply<T::tuple_size()>([]<size_t... I> static { return (... && requires { typename T::value_type<I>; }); });

	template<class T, size_t N = 0>
	concept uniform_tuple_like = (tuple_like<T, N>
		&& apply<tuple_size<T>()>([]<size_t... I> static { return same_as_every<tuple_element_t<I, T>...>; }));

	template<class T, size_t N = 0>
	concept fixed_string_like = range_uses_traits_type<T> && uniform_tuple_like<T, N>;

	template<class F, size_t N>
	concept constexprifier_like = !!N && apply<N>([]<size_t... I> static { return same_as_every<call_template_t<F, I>...>; });

	template<size_t N, constexprifier_like<N> F>
	constexpr std::array constexprifier_table_v = apply<N>([]<size_t... I> static ->
		std::array<call_template_t<F, 0uz>, N> { return {(&std::remove_cvref_t<F>::template operator()<I>)...}; });

	template<size_t N, constexprifier_like<N> F, class... A>
	constexpr decltype(auto) constexprify(const size_t i, F && f = default_value, A &&... args) {
		if constexpr (std::is_member_function_pointer_v<call_template_t<F, 0uz>>) {
			return (std::forward<F>(f).*constexprifier_table_v<N, F>[i])(std::forward<A>(args)...);
		} else {
			return constexprifier_table_v<N, F>[i](std::forward<A>(args)...);
		}
	}



	template<class T>
	using propagate_const_t = type_in_c_t<invoke<get_call<T>()>([]<class U>(this const auto lambda) {
		/*  */ if constexpr (pointer_like<U>) {
			return c_type<type_in_c_t<invoke<get_call<std::remove_pointer_t<U>>()>(lambda)> *const>();

		} else if constexpr (lvalue_reference_like<U>) {
			return c_type<type_in_c_t<invoke<get_call<std::remove_reference_t<U>>()>(lambda)> &>();

		} else if constexpr (rvalue_reference_like<U>) {
			return c_type<type_in_c_t<invoke<get_call<std::remove_reference_t<U>>()>(lambda)> &&>();

		} else {
			return c_type<const U>();
		}
	})>;

	template<class U, class V, class T>
	concept in_relation_with = cref_relation<T, const U &, const V &>;

	template<class U, class T>
	concept hashable_by = cref_invocable_r<T, size_t, const U &>;

	template<class U, template<class> class T>
	concept hashable_by_template = (hashable_by<U, T<U>> && movable_constructible_from<T<U>>);

	template<class T>
	using array_t = std::array<std::remove_extent_t<T>, std::extent_v<T>>;

	// https://mathworld.wolfram.com/Hypermatrix.html
	template<class T, size_t... N>
		requires (!!sizeof...(N))
	using hypermatrix_t = type_in_c_t<invoke<get_call<N...>()>([]<size_t I1, size_t... I>(this const auto lambda) {
		if constexpr (sizeof...(I)) {
			return c_type<std::array<type_in_c_t<invoke<get_call<I...>()>(lambda)>, I1>>();

		} else {
			return c_type<std::array<T, I1>>();
		}
	})>;

	// Netikriname ar INVOCABLE yra funkcijos rodyklė, nes gali būti naudinga naudoti šiuos tipus ir su objektais.
	// Yra neleidžiama gauti std funkcijų adresų, bet visų atvejų atskirų neišrašysime, kai būtų toks adresas paduotas.
	template<auto INVOCABLE, auto... V>
	using lift_bind_back_t = t<[]<class... A>
		requires (cref_invocable<t<INVOCABLE>, A..., const t<V> &...>)
	(A &&... args) static -> decltype(auto) {
		return std::invoke(INVOCABLE, std::forward<A>(args)..., V...);
	}>;

	template<auto INVOCABLE, auto... V>
	using lift_bind_front_t = t<[]<class... A>
		requires (cref_invocable<t<INVOCABLE>, const t<V> &..., A...>)
	(A &&... args) static -> decltype(auto) {
		return std::invoke(INVOCABLE, V..., std::forward<A>(args)...);
	}>;

	template<auto INVOCABLE, auto... V>
		requires (cref_invocable<t<INVOCABLE>, const t<V> &...>)
	using lift_ignore_args_t = t<[]<class... A>(A &&...) static -> decltype(auto) {
		return std::invoke(INVOCABLE, V...);
	}>;

	template<auto INVOCABLE, auto... V>
		requires (cref_invocable<t<INVOCABLE>, const t<V> &...>)
	using lift_wo_args_t = t<[] static -> decltype(auto) {
		return std::invoke(INVOCABLE, V...);
	}>;

	// Nedarome cast operacijos ant rezultato (kad, pavyzdžiui, leisti naudotojui pasirinkti gauti didesnį integral tipą),
	// nes tokia realizacija yra visados klaidinga. Naudotojas galės, jei norės pats tokią operaciją atlikti.
	template<std::integral X>
	constexpr auto unsign(const X x) {
		return std::bit_cast<std::make_unsigned_t<X>>(x);
	}

	template<enum_like X>
	constexpr auto unsign(const X x) {
		return unsign(std::to_underlying(x));
	}

	template<std::integral X>
	constexpr auto sign(const X x) {
		return std::bit_cast<std::make_signed_t<X>>(x);
	}

	template<enum_like X>
	constexpr auto sign(const X x) {
		return sign(std::to_underlying(x));
	}

	// https://en.cppreference.com/w/cpp/language/implicit_cast.html#Integral_conversions
	// Kai pirminis tipas yra signed ir rezultato tipas yra unsigned ir jis didesnis, tada reikšmė yra sign-extended. Todėl reikia šios funkcijos.
	template<std::integral T, std::integral X>
	constexpr T sign_cast(const X x) {
		if constexpr (std::unsigned_integral<T>)	return T{unsign(x)};
		else										return T{x};
	}

	template<std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_exp2(const T x) {
		return c(U{1}) << x;
	}

	// T yra tipas, kurio bitus skaičiuosime, U nurodo kokiu tipu pateikti rezultatus.
	// [0, digits<T>) ∪ {0b(1)_digits<T>}
	template<std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_log2(const T x) {
		return c(U{numeric_digits<std::make_signed_t<T>>()}) - sign_cast<U>(std::countl_zero(x));
	}

	// Naudojamas size_t tipas kaip konstantos tipas, o ne sekančio dydžio integer tipas, nes konstanta
	// turėtų būti naudojama malloc ir panašiuose kontekstuose, o ten reikalaujama size_t tipo išraiška.
	// T turi turėti unique object representations, nes kitaip neišeis patikimai apskaičiuoti konstantos.
	//
	// Vietoje byte negalime naudoti uint8_t, nes jei sistemoje baitas būtų ne 8 bitų, tas tipas nebus apibrėžtas.
	template<uniquely_representable T>
	constexpr size_t representable_values_v = int_exp2(sizeof(T) * numeric_digits<std::byte>());

	template<class F, class... A>
	concept out_unary_invocable = lvalue_reference_like<function_argument_t<F>>
		&& movable_constructible_from<std::remove_reference_t<function_argument_t<F>>, A...>;

	template<class F, class... A>
	concept out_unary_call_invocable = out_unary_invocable<call_template_t<F>, A...>
		&& tuple_like<function_argument_t<call_template_t<F>>>;

	template<class F, class... A>
		requires (out_unary_invocable<F, A...>)
	constexpr auto make(F && f = default_value, A &&... args) {
		using value_type = std::remove_reference_t<function_argument_t<F>>;
		if constexpr (!sizeof...(A)) {
			value_type d;
			std::invoke(std::forward<F>(f), d);
			return d;
		} else {
			value_type d = {std::forward<A>(args)...};
			std::invoke(std::forward<F>(f), d);
			return d;
		}
	}

	template<auto EMPTY = default_value, class F, class... A>
		requires (out_unary_invocable<F, A...> && cref_constructible_to<t<EMPTY>, std::remove_reference_t<function_argument_t<F>>>)
	constexpr std::remove_reference_t<function_argument_t<F>> make_if(F && f = default_value, A &&... args) {
		using value_type = std::remove_reference_t<function_argument_t<F>>;
		if constexpr (!sizeof...(A)) {
			value_type d;
			if (std::invoke_r<bool>(std::forward<F>(f), d))
				return d; else return EMPTY;
		} else {
			value_type d = {std::forward<A>(args)...};
			if (std::invoke_r<bool>(std::forward<F>(f), d))
				return d; else return EMPTY;
		}
	}

	template<class F, class... A>
		requires (out_unary_invocable<F, A...>)
	constexpr std::optional<std::remove_reference_t<function_argument_t<F>>> make_opt(F && f = default_value, A &&... args) {
		using value_type = std::remove_reference_t<function_argument_t<F>>;
		if constexpr (!sizeof...(A)) {
			value_type d;
			if (std::invoke_r<bool>(std::forward<F>(f), d))
				return d; else return std::nullopt;
		} else {
			value_type d = {std::forward<A>(args)...};
			if (std::invoke_r<bool>(std::forward<F>(f), d))
				return d; else return std::nullopt;
		}
	}

	template<class F, class... A>
		requires (out_unary_call_invocable<F, A...>)
	constexpr auto make_tuple(F && f = default_value, A &&... args) {
		using value_type = std::remove_reference_t<function_argument_t<call_template_t<F>>>;
		if constexpr (!sizeof...(A)) {
			value_type d;
			apply<tuple_size<value_type>()>(std::forward<F>(f), d);
			return d;
		} else {
			value_type d = {std::forward<A>(args)...};
			apply<tuple_size<value_type>()>(std::forward<F>(f), d);
			return d;
		}
	}

	template<class T>
	constexpr auto to_pointer(T & t) {
		if constexpr (std::indirectly_readable<T>) {
			return std::to_address(t);
		} else {
			return std::addressof(t);
		}
	}

	template<class T>
	constexpr decltype(auto) to_reference(T & t) {
		if constexpr (std::indirectly_readable<T>) {
			return *t;
		} else {
			return t;
		}
	}



	// Galėtume vietoje _1 naudoti std::ignore. Bet minėtos konstantos tipas nėra tuščias.
	// Taip pat jei reiktų negalėtume to pačio parametro su skirtingomis konstantomis specializuoti.
	//
	// Comparators do not declare is_transparent, nes jų ir taip neišeitų panaudoti su c++ standarto konteineriais.
	//
	// Mes nurodome dešinės pusės operandą su template parametru, nes realizuojame šią gražią elgseną: sakykime turime
	// objektą tipo less<3>, šio objekto operator() grąžins true tik tada kai paduotas kintamasis bus mažesnis už 3.
	template<auto R = c_type<void>()>
	struct less {
		static consteval t<R> value() { return R; }

		template<std::totally_ordered_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l < R; }
	};

	template<>
	struct less<c_type<void>()> {
		template<auto R, std::totally_ordered_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l < R; }
	};

	template<auto R = c_type<void>()>
	struct less_equal {
		static consteval t<R> value() { return R; }

		template<std::totally_ordered_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l <= R; }
	};

	template<>
	struct less_equal<c_type<void>()> {
		template<auto R, std::totally_ordered_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l <= R; }
	};

	template<auto R = c_type<void()>>
	struct greater {
		static consteval t<R> value() { return R; }

		template<std::totally_ordered_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l > R; }
	};

	template<>
	struct greater<c_type<void>()> {
		template<auto R, std::totally_ordered_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l > R; }
	};

	template<auto R = c_type<void()>>
	struct greater_equal {
		static consteval t<R> value() { return R; }

		template<std::totally_ordered_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l >= R; }
	};

	template<>
	struct greater_equal<c_type<void>()> {
		template<auto R, std::totally_ordered_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l >= R; }
	};

	template<auto R = c_type<void>()>
	struct equal_to {
		static consteval t<R> value() { return R; }

		template<std::equality_comparable_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l == R; }
	};

	template<>
	struct equal_to<c_type<void>()> {
		template<auto R, std::equality_comparable_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l == R; }
	};

	template<auto R = c_type<void>()>
	struct not_equal_to {
		static consteval t<R> value() { return R; }

		template<std::equality_comparable_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l != R; }
	};

	template<>
	struct not_equal_to<c_type<void>()> {
		template<auto R, std::equality_comparable_with<t<R>> L>
		static constexpr bool operator()(const L & l) { return l != R; }
	};



	// Parameter pack negali susidėti iš daug elementų todėl šitą funkciją reiktų naudoti tik kai reikia mažai
	// iteracijų. Tačiau toks iteravimas greitesnis, nes nereikia prižiūrėti papildomo ciklo kintamojo.
	//
	// Greitaveika nenukenčia padavinėjant template parametrus todėl neturime funkcijos užklojimo kito.
	// P yra value tipas, nes funktoriai, kurie bus kviečiami daug kartų yra taip padavinėjami. Kitaip P tipas būtų rvalue reference.
	template<size_t N, class F>
	constexpr void repeat(F f) {
		apply<N>([&]<size_t... I> -> void { (aa::invoke<get_call<I>()>(f), ...); });
	}



	template<class T, class... A>
	using next_type_t = A...[(type_pack_index_v<T, A...>) + 1];

	template<std::integral T>
	using next_int_t = copy_unsigned_t<T, next_type_t<std::make_signed_t<T>, int8_t, int16_t, int32_t, int64_t>>;

	template<std::integral T>
	using prev_int_t = copy_unsigned_t<T, next_type_t<std::make_signed_t<T>, int64_t, int32_t, int16_t, int8_t>>;

	template<std::integral X>
	constexpr auto uprank(const X x) {
		return sign_cast<next_int_t<X>>(x);
	}

	template<std::integral X>
	constexpr auto downrank(const X x) {
		return sign_cast<prev_int_t<X>>(x);
	}

}



// Negalime tikrinti ar prieš šį momentą tuple_size<T> buvo deklaruotas tipas ar ne, nes įeitume į begalinį
// ciklą. Reiškia turi mums pats tipas pranešti ar jis nori būti laikomas kaip tuple like tipas.
template<aa::new_tuple_like T>
struct std::tuple_size<T> : aa::constant<T::tuple_size()> {};

template<size_t I, aa::new_tuple_like T>
struct std::tuple_element<I, T> : std::type_identity<typename T::value_type<I>> {};

template<aa::enum_like T>
struct std::numeric_limits<T> : std::numeric_limits<std::underlying_type_t<T>> {};
