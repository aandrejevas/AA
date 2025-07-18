#pragma once

// Nerealizuojame algoritmų, kurie netikrintų ar masyvas tuščias, nes tą patį galime pasiekti su įprastais algoritmais ir atributo assume naudojimu. Nerealizuojame fixed_string, type_name, log, nes tokį funkcionalumą suteikia žurnalavimo (spdlog) ir tokios kaip nameof bibliotekos. Nerealizuojame savo lexer, nes galime naudoti tiesiog populiarų formatą kaip json. Nerealizuojame print ir read, nes galime naudoti bibliotekas fmt ir scn. Nerealizuojame AA_IF_DEBUG, AA_TRACE_ASSERT, timekeeper, nes jie realizuoti tokiose bibliotekose kaip Boost. Nerealizuojame to_array, nes galime naudoti std::to_array arba std::array<T, 0>{}. Nerealizuojame int_math, nes galime tiesiogiai naudoti math funkcijas (klaidinga bandyti išrašyti math funkcijų visas kombinacijas, o tą ir darėme). Neturime ptr_v ir out_v, nes tokie globalūs kintamieji padidina programos dydį, ko be problemų galima išvengti.

// Filosofija bibliotekos tokia, visos funkcijos žymimos constexpr ir tiek. Nesvarbu gali ar negali būti funkcija
// naudojama constexpr kontekste, ji bus pažymėta constexpr. Gal naudotojams kiek neaišku gali būti ar jie gali
// funkciją naudoti constexpr kontekste, bet aš nenoriu mąstyti apie tai ar funkcijos realizacija gali būti constexpr.

// constexpr ir consteval funkcijos specifikatoriai implikuoja inline, todėl nereikia naudoti jų kartu.
// Atitinkamai constexpr kintamojo specifikatorius implikuoja inline ir const specifikatorius.
// Reikia stengtis turėti ir naudoti kuo mažiau macros.
// Reikia stengtis vietoje consteval funkcijų naudoti constexpr kintamuosius nebent nėra kito pasirinkimo.

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
// • <memory> (<compare>), failas įterptas, kad išeitų lengvai protauti apie atmintį.
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
#include <memory>



namespace aa {

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

	template<class T>
	using add_cref_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

	template<class T>
	concept not_cv = std::same_as<std::remove_reference_t<T>, std::remove_cvref_t<T>>;

	template<class T>
	concept not_cvref = std::same_as<T, std::remove_cvref_t<T>>;

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
	concept object_pointer_like = pointer_like<T> && std::is_object_v<std::remove_pointer_t<T>>;

	template<class T>
	concept function_pointer_like = (pointer_like<T> && std::is_function_v<std::remove_pointer_t<T>>) || std::is_member_pointer_v<T>;

	template<class T>
	concept class_like = std::is_class_v<T>;

	template<class T>
	concept reference_like = std::is_reference_v<T>;

	template<class T>
	concept lvalue_reference_like = std::is_lvalue_reference_v<T>;

	template<class T>
	concept not_lvalue_reference_like = !lvalue_reference_like<T>;

	template<class T>
	concept rvalue_reference_like = std::is_rvalue_reference_v<T>;

	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<class T, class F>
	concept arithmetic_or_assignable_from = arithmetic<T> || std::assignable_from<T &, F>;

	template<class T>
	concept regular_scalar = arithmetic<T> || object_pointer_like<T>;

	template<class T>
	concept uniquely_representable = std::has_unique_object_representations_v<T>;

	template<class T>
	concept trivially_copyable = std::is_trivially_copyable_v<T>;

	template<class L, class R>
	concept wo_cvref_same_as = std::same_as<std::remove_cvref_t<L>, R>;

	template<class F, class... A>
	concept cref_predicate = std::predicate<const F &, A...>;

	template<class F, class U, class V>
	concept cref_relation = std::relation<const F &, U, V>;

	template<class F, class... A>
	concept ref_invocable = std::invocable<F &, A...>;

	template<class F, class... A>
	concept cref_invocable = std::invocable<const F &, A...>;

	template<class F, class... A>
	concept invocable_with_one_of = (... || std::invocable<F, A>);

	template<class F, class R, class... A>
	concept invocable_r = std::is_invocable_r_v<R, F, A...>;

	template<class F, class R, class... A>
	concept cref_invocable_r = std::is_invocable_r_v<R, const F &, A...>;

	template<class F, class T>
	concept constructible_to = std::constructible_from<T, F>;

	template<class F, class T>
	concept constructible_between = (std::constructible_from<T, F> && std::constructible_from<F, T>);

	template<class T, class U>
	concept not_same_as_and_equality_comparable = !std::same_as<T, U> && std::equality_comparable<T>;

	template<class T, class U>
	concept not_same_as_and_totally_ordered = !std::same_as<T, U> && std::totally_ordered<T>;

	// https://en.cppreference.com/w/cpp/concepts/boolean-testable.
	template<class B>
	concept bool_testable = (std::constructible_from<bool, B>
		&& requires(B && b) { { !std::forward<B>(b) } -> constructible_to<bool>; });

	template<class R, class L>
	concept assignable_to = std::assignable_from<L, R>;

	template<class T, class... F>
	concept constructible_from_every = (... && std::constructible_from<T, F>);

	template<class T>
	concept constructible_from_floating = constructible_from_every<T, float, double, long double>;

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

	// Nieko tokio, kad kopijuojame konstantas, kadangi viskas vyksta kompiliavimo metu.
	//
	// Negalime konstantų pakeisti funkcijomis, nes neišeina gauti adreso funkcijos
	// rezultato. Tai reiškia, kad šitoks sprendimas yra universalesnis.
	//
	// constant_t alias neturėtų prasmės, nes, kad juo naudotis jau reiktų nurodyti ką norime gauti.
	template<auto V>
	using const_t = decltype(V);

	// Išmeta klaidą parašius šią išraišką const_v<AAA{}> ar constant<AAA{}>, jei tik movable uždėtas constraint.
	// struct AAA {
	// 	constexpr AAA & operator=(AAA &&) = default;
	// 	constexpr AAA(AAA &&) = default;
	// 	constexpr AAA(const AAA &) = delete;
	// 	constexpr AAA() = default;
	// };
	template<std::copyable auto V>
	constexpr const_t<V> const_v = V;

	template<std::copyable auto V>
	using constant = std::integral_constant<const_t<V>, V>;

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2781r8.html
	// Negalime turėti literal, nes nėra būdo kaip gauti constexpr reikšmės funkcijoje.
	template<auto V>
	constexpr constant<V> c;

	template<class T>
	constexpr std::type_identity<T> t;

	template<size_t... I>
	constexpr std::index_sequence<I...> s;

	template<auto V>
	using type_in_const_t = type_in_use_t<const_t<V>>;

	template<class T, constructible_to<T> X>
	constexpr T cast(X && x) {
		return static_cast<T>(std::forward<X>(x));
	}

	// <><><><><><><><><><><><><><><><><><><><><><><><> STR RANGES <><><><><><><><><><><><><><><><><><><><><><><><>

	template<class B, class I>
	concept bool_auxiliary_range_for = (std::ranges::random_access_range<B> && std::ranges::output_range<B, bool>
		&& bool_testable<std::ranges::range_reference_t<B>> && constructible_between<I, std::ranges::range_difference_t<B>>);

	template<class R>
	concept sized_random_access_range = std::ranges::random_access_range<R> && std::ranges::sized_range<R>;

	template<class T>
	concept sized_contiguous_range = std::ranges::contiguous_range<T> && std::ranges::sized_range<T>;

	template<class T>
	concept sized_input_range = std::ranges::input_range<T> && std::ranges::sized_range<T>;

	template<class R>
	concept nonrange = !std::ranges::range<R>;

	template<class R>
	concept permutable_range = (sized_random_access_range<R> && std::permutable<std::ranges::iterator_t<R>>);

	template<class I>
	concept char_output_iterator = std::output_iterator<I, const char &>;

	template<class I>
	concept int_input_iterator = std::input_iterator<I> && std::assignable_from<int &, std::iter_reference_t<I>>;

	template<class T, class I, class F>
	concept ptr_in_indirect_predicate_with = std::indirect_binary_predicate<F, I, const T *>;

	template<class I>
	using iter_size_t = std::make_unsigned_t<std::iter_difference_t<I>>;

	template<std::contiguous_iterator I>
	using iter_pointer_t = std::add_pointer_t<std::iter_reference_t<I>>;

	template<std::ranges::contiguous_range R>
	using range_pointer_t = std::add_pointer_t<std::ranges::range_reference_t<R>>;

	template<class T>
	concept reverse_range = requires(T& t) {
		std::ranges::rbegin(t);
		std::ranges::rend(t);
	};

	// Galėtume realizuoti taip: function_result_t<const_t<&const_t<std::ranges::rbegin>::operator()<T &>>>;
	// Bet yra draudžiama gauti std funkcijos adresą: https://en.cppreference.com/w/cpp/language/extending_std.html.
	template<class T>
	using reverse_iterator_t = std::invoke_result_t<const_t<std::ranges::rbegin>, T &>;

	template<reverse_range R>
	using reverse_sentinel_t = std::invoke_result_t<const_t<std::ranges::rend>, R &>;

	template<class R>
	concept rbegin_accessible_by_const_cast_range = std::ranges::contiguous_range<R>
		&& requires(const std::ranges::sentinel_t<R> s) { const_cast<std::ranges::iterator_t<R>>(s); };

	template<class R>
	concept rbegin_accessible_by_constructor_range = std::ranges::bidirectional_range<R>
		&& std::constructible_from<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>>;

	// common_range negalime naudoti, nes mano ranges nėra common.
	template<class R>
	concept rbegin_accessible_range = rbegin_accessible_by_const_cast_range<R>
		|| rbegin_accessible_by_constructor_range<R> || sized_random_access_range<R>;

	template<class R>
	concept unusual_range = rbegin_accessible_range<R> && std::same_as<reverse_iterator_t<R>, std::ranges::iterator_t<R>>;

	// Nėra atitinkamos funkcijos rend iteratoriui, nes jis nėra svarbus.
	// get_rdata nėra funkcijos, nes iš šios funkcijos galima gauti rodyklę.
	// Čia turėtų būti O(1) sudėtingumo funkcija, todėl netinka naudoti distance.
	// Algoritmai priima ranges kur tik įmanoma. Jei turime tik iteratorius tai galima paduoti iš jų sukurtą range.
	template<rbegin_accessible_range R>
	constexpr std::ranges::iterator_t<R> get_rbegin(R && r) {
		if constexpr (unusual_range<R>) {
			// std::reverse_iterator nėra constructible į iterator.
			return std::ranges::rbegin(r);

		} else if constexpr (rbegin_accessible_by_const_cast_range<R>) {
			return std::ranges::prev(const_cast<std::ranges::iterator_t<R>>(std::ranges::end(r)));

		} else if constexpr (rbegin_accessible_by_constructor_range<R>) {
			return std::ranges::prev(cast<std::ranges::iterator_t<R>>(std::ranges::end(r)));

		} else {
			return std::ranges::begin(r) + (std::ranges::ssize(r) - 1);
		}
	}

	// <><><><><><><><><><><><><><><><><><><><><><><><> END RANGES <><><><><><><><><><><><><><><><><><><><><><><><>

	template<class T, class... A>
	concept movable_constructible_from = std::movable<T> && std::constructible_from<T, A...>;

	namespace detail {
		struct getter {
			// Vietoj operatorių apibrėžimo galėtume paveldėti iš monostate, bet kitur negalime paveldėti ir nepaveldime tai čia taip pat taip elgiamės, kad būtume nuoseklūs. Taip pat nebūtų teisinga paveldėti, nes vietoje monostate galėtų būtų naudojamos šios klasės.
			// Turime apibrėžti ir operator==, nes todėl, kad turime kitą operator==, iš operator<=> nėra sukuriamas automatiškai tas operatorius.
			consteval bool operator==(this getter, getter) = default;
			consteval std::strong_ordering operator<=>(this getter, getter) = default;

			// Reikia funkcijų ir šios klasės apskritai, nes be jos, getter'ių negalima lyginti tik su fundamental tipais kažkodėl.
			template<class S, not_same_as_and_equality_comparable<S> T>
			constexpr bool operator==(this const S self, const T & t) { return cast<T>(self) == t; }

			template<class S, not_same_as_and_totally_ordered<S> T>
			constexpr auto operator<=>(this const S self, const T & t) { return cast<T>(self) <=> t; }
		};

		// T tipas nesiskiria kai esame const T & kontekste ar T kontekste. Tai reiškia, kad
		// privalome pasirinkti grąžinti const T & arba T visiems atvejams. Yra pasirinkta grąžinti T,
		// nes buvo nuspręsta, kad šias klases turi būti įmanoma naudoti ir ne su constexpr konstruktorius
		// turinčiomis klasėmis ir joms neišeitų grąžinti const T &. Jei vis dėlto reikia const T &
		// tipo, galima dirbti su atitinkamais constexpr kintamaisiais, kurie yra apibrėžti po šių klasių.
		template<auto... A>
		struct value_getter : getter {
			// T turi būti movable, nes mums reikia žinoti ar galime iš gražinamos reikšmės sukonstruoti T.
			// Nereikia turėti ir consteval funkcijos, nes visa esmė constexpr specifikatoriaus ir yra,
			// kad funkcijos rezultatas bus apskaičiuotas kompiliavimo metu, jei tai yra įmanoma padaryti.
			template<movable_constructible_from<const const_t<A> &...> T>
			constexpr operator T() const { return T{A...}; }
		};

		struct numeric_max_getter : getter {
			// Ne consteval, nes max gali būti ne consteval funkcija.
			template<std::movable T>
			constexpr operator T() const { return std::numeric_limits<T>::max(); }
		};

		struct numeric_min_getter : getter {
			template<std::movable T>
			constexpr operator T() const { return std::numeric_limits<T>::min(); }
		};

		// Galėtų struktūra būti pakeista globaliu kintamuoju, bet tada padidėtų programos dydis ir mes su šia realizacija neprarandame greitaveikos.
		template<uintptr_t N>
		struct ptr_getter : getter {
			template<class T>
			constexpr operator T * () const {
				if constexpr (!!N)	return std::bit_cast<T *>(N);
				else				return nullptr;
			}
		};
	}

	template<auto... A>
	constexpr detail::value_getter<A...> value;

	constexpr detail::value_getter default_value;

	constexpr detail::numeric_max_getter numeric_max;

	constexpr detail::numeric_min_getter numeric_min;

	template<uintptr_t N = 0>
	constexpr detail::ptr_getter<N> ptr;

	template<std::movable T, auto... A>
	constexpr T value_v = value<A...>;

	// Dabar neišeina naudoti default_v su fundamentaliais tipais, bet tokiais atvejais galima naudoti value_v.
	template<movable_constructible_from T>
	constexpr T default_v;

	template<std::movable T>
	constexpr T numeric_max_v = numeric_max;

	template<enum_like T>
	constexpr std::underlying_type_t<T> numeric_max_v<T> = numeric_max;

	template<std::movable T>
	constexpr T numeric_min_v = numeric_min;

	template<enum_like T>
	constexpr std::underlying_type_t<T> numeric_min_v<T> = numeric_min;

	template<std::movable T>
	constexpr size_t numeric_digits_v = std::numeric_limits<T>::digits;

	template<enum_like T>
	constexpr size_t numeric_digits_v<T> = std::numeric_limits<std::underlying_type_t<T>>::digits;



	// Kartais patogiau naudoti lambda su generic tipo parametru, bet ne atvejais, kai skiriasi parametrų skaičiai.
	// Negalime tikrinti ar F turi operator(), nes gal ta funkcija yra templated ir mes neturime kaip to patikrinti.
	// https://www.cppstories.com/2019/02/2lines3featuresoverload.html
	template<class... F>
	struct overload : F... {
		using F::operator()...;
		using is_transparent = void;
	};

	// Lambdos ne consteval, nes nereikia constexpr kontekstuose to žymėti. Analogiškos lambdos taip pat neturi tokio specifikatoriaus.
	template<class F>
	using function_t = type_in_const_t<overload{
		([]<bool N, class R, class... A>(std::type_identity<R(A...) noexcept(N)>) static -> auto { return t<R(A...)>; }),
		([]<bool N, class R, class... A>(std::type_identity<R(*)(A...) noexcept(N)>) static -> auto { return t<R(A...)>; }),
		([]<class T, class R>(std::type_identity<R T:: *>) static -> auto { return t<R()>; }),
		([]<bool N, class T, class R, class... A>(std::type_identity<R(T:: *)(A...) noexcept(N)>) static -> auto { return t<R(A...)>; }),
		([]<bool N, class T, class R, class... A>(std::type_identity<R(T:: *)(A...) & noexcept(N)>) static -> auto { return t<R(A...)>; }),
		([]<bool N, class T, class R, class... A>(std::type_identity<R(T:: *)(A...) && noexcept(N)>) static -> auto { return t<R(A...)>; }),
		([]<bool N, class T, class R, class... A>(std::type_identity<R(T:: *)(A...) const noexcept(N)>) static -> auto { return t<R(A...)>; }),
		([]<bool N, class T, class R, class... A>(std::type_identity<R(T:: *)(A...) const & noexcept(N)>) static -> auto { return t<R(A...)>; }),
		([]<bool N, class T, class R, class... A>(std::type_identity<R(T:: *)(A...) const && noexcept(N)>) static -> auto { return t<R(A...)>; }),
		([]<class T>(this const auto lambda, std::type_identity<T>) -> auto { return lambda(t<const_t<&T::operator()>>); })
	}(t<std::remove_cvref_t<F>>)>;

	template<class F, size_t I = 0>
	using function_argument_t = type_in_const_t<([]<class R, class... A>(std::type_identity<R(A...)>) static ->
		auto { return t<A...[I]>; })(t<function_t<F>>)>;

	// GCC 15.1.0 BUG: ICE Segmentation fault when using the lambda directly
	namespace detail {
		template<class F>
		constexpr std::type_identity function_result_v = ([]<class R, class... A>(std::type_identity<R(A...)>) static ->
			auto { return t<R>; })(t<function_t<F>>);
	}

	template<class F>
	using function_result_t = type_in_const_t<detail::function_result_v<F>>;

	template<class F>
	constexpr size_t function_arity_v = ([]<class R, class... A>(std::type_identity<R(A...)>) static ->
		size_t { return sizeof...(A); })(t<function_t<F>>);



	template<class T>
	concept empty_unit_param_like = std::is_empty_v<T> && movable_constructible_from<std::remove_const_t<T>>;

	template<class T>
	concept nonempty_unit_param_like = !empty_unit_param_like<T>;

	template<class T>
	concept const_unit_param_like = std::is_const_v<T> && movable_constructible_from<std::remove_const_t<T>>;

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
			constexpr auto operator<=>(const tuple_unit &) const = default;

			// Special member functions
			constexpr tuple_unit() = default;
			constexpr tuple_unit() requires (const_unit_param_like<value_type>) : value{default_value} {}
			template<constructible_to<value_type> U = value_type>
			constexpr tuple_unit(U && u) : value{std::forward<U>(u)} {}

			// Member objects
			// Nenaudojame atributo no_unique_address, nes jis čia nieko nekeistų.
			value_type value;
		};

		// Negalime paveldėti iš T, nes kažkodėl tada tuple dydis padidėja.
		template<size_t I, empty_unit_param_like T>
		struct tuple_unit<I, T> {
			// Member types
			using value_type = const T;
			using reference = value_type &;
			using const_reference = value_type &;
			using pointer = value_type *;
			using const_pointer = value_type *;

			// Comparisons
			consteval std::strong_ordering operator<=>(this tuple_unit, tuple_unit) = default;

			// Member objects
			static constexpr value_type value = default_value;
		};
	}

	template<class T>
	concept complete_tuple_size = complete<std::tuple_size<std::remove_reference_t<T>>>;

	template<complete_tuple_size T>
	constexpr size_t tuple_size_v = std::tuple_size_v<std::remove_reference_t<T>>;

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

	constexpr const_t<get_element<0>> get_0, get_x, get_key;
	constexpr const_t<get_element<1>> get_1, get_y, get_val;
	constexpr const_t<get_element<2>> get_2, get_z;
	constexpr const_t<get_element<3>> get_3;

	template<size_t I, gettable<I> T>
	using get_result_t = std::invoke_result_t<const_t<get_element<I>>, T>;

	template<class F, auto... A>
	using call_template_t = const_t<&std::remove_cvref_t<F>::template operator()<A...>>;

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2989r2.pdf
	template<auto... A, class F, class... T>
	constexpr decltype(auto) invoke(F && f, T &&... t) {
		return std::forward<F>(f).template operator()<A...>(std::forward<T>(t)...);
	}

	// Nesijaudinant galima naudoti tag tipus kaip funkcijų parametrus.
	// https://www.fluentcpp.com/2021/03/05/stdindex_sequence-and-its-improvement-in-c20/
	// Jei lambdą iškeltume į funkciją, tai ji jokio funkcionalumo nesuteiktų, nes patogiau kiekvienu atveju būtų ne ją naudoti, o atitinkamą invoke.
	template<size_t N, class F, class... A>
	constexpr decltype(auto) apply(F && f, A &&... args) {
		return ([&]<size_t... I>(std::index_sequence<I...>) -> decltype(auto) {
			return invoke<I...>(std::forward<F>(f), std::forward<A>(args)...);
		})(default_v<std::make_index_sequence<N>>);
	}

	template<class T, size_t N = 0>
	concept tuple_like = (tuple_size_v<T> >= N) && apply<tuple_size_v<T>>(
		[]<size_t... I> static -> bool { return (... && std::constructible_from<tuple_element_t<I, T>, get_result_t<I, T>>); });

	template<class F, class T, class... A>
		requires (std::invocable<F, T &, A...>)
	constexpr T & extend(T & t, F && f = default_value, A &&... args) {
		std::invoke(std::forward<F>(f), t, std::forward<A>(args)...);
		return t;
	}

	template<class F, class T, size_t... I>
	concept tuple_constructible_to = tuple_like<F, tuple_size_v<T>>
		&& (... && std::constructible_from<tuple_element_t<I, T>, get_result_t<I, F>>);



	// Naudojame constexpr algoritmą, kad surasti indeksą, nes lygiai taip pat ir GCC daro.
	template<auto A, std::equality_comparable_with<const_t<A>> auto... V>
	constexpr size_t pack_index_v = pack_index_v<true, (A == V)...>;

	template<bool A, bool... V>
	constexpr size_t pack_index_v<A, V...> = get_key(*std::ranges::find(std::views::enumerate(stay(std::array{V...})), A, get_val));

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
		static constexpr size_t index = type_pack_index_v<U, T...>;

		// Capacity
		static constexpr size_t tuple_size = sizeof...(T);

		// Comparisons
		constexpr auto operator<=>(const tuple_type &) const = default;

		// Element access
		template<size_t I, class H>
		constexpr auto && operator[](this H && self, constant<I>) {
			return std::forward<H>(self).template get<I>();
		}

		template<size_t I, class H>
		constexpr auto && get(this H && self) {
			return std::forward<H>(self).unit_type<I>::value;
		}

		template<class U, class H>
		constexpr auto && operator[](this H && self, std::type_identity<U>) {
			return std::forward<H>(self).template get<U>();
		}

		template<class U, class H>
		constexpr auto && get(this H && self) {
			return std::forward<H>(self).unit_type<index<U>>::value;
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
	concept specialization_of = ([]<class... A>(std::type_identity<F<A...>>) static -> bool { return true; })(t<std::remove_cvref_t<T>>);

	template<sized_contiguous_range T>
	using range_char_traits_t = type_in_const_t<([] static -> auto {
		if constexpr (uses_traits_type<T>) {
			return t<traits_type_in_use_t<T>>;
		} else {
			// Darome prielaidą, kad char_traits yra apibrėžtas su visais tipais.
			return t<std::char_traits<std::ranges::range_value_t<T>>>;
		}
	})()>;

	template<class T, class U>
	concept same_range_char_traits_as = std::same_as<range_char_traits_t<T>, U>;

	template<class T>
	concept range_uses_traits_type = sized_contiguous_range<T> &&
		std::same_as<char_type_in_use_t<range_char_traits_t<T>>, std::ranges::range_value_t<T>>;

	template<class T>
	concept regular_unsigned_integral = std::unsigned_integral<T> && std::has_single_bit(numeric_digits_v<T>);

	// Nors galėtume paveldėti tiesiog iš first, bet to nedarome, kad nekurti nereikalingų paveldėjimo ryšių.
	template<class T, class... A>
	using first_not_t = A...[pack_index_v<false, std::same_as<T, A>...>];

	template<bool B, class T>
	using add_const_if_t = std::conditional_t<B, const T, T>;

	// Reikia using šio, nes testavimui reikėjo sukurti tuple su 100 elementų ir nėra variantas turėti 100 using'ų.
	template<template<class...> class T, auto F, size_t N>
	using filled_t = type_in_const_t<apply<N>([]<size_t... I> static ->
		auto { return t<T<type_in_const_t<invoke<I>(F)>...>>; })>;

	// GCC 15.1.0 BUG: ICE with 'return (... && requires { requires std::constructible_from<typename T::value_type<I>, get_result_t<I, T>>; })'
	// Netikriname ar 'std::constructible_from<typename T::value_type<I>, get_result_t<I, T>>', nes tai patikrins tuple_like.
	template<class T>
	concept new_tuple_like = (requires { { T::tuple_size } -> wo_cvref_same_as<size_t>; })
		&& apply<T::tuple_size>([]<size_t... I> static -> bool { return (... && requires { typename T::value_type<I>; }); });

	template<class T, size_t N = 0>
	concept uniform_tuple_like = (tuple_like<T, N>
		&& apply<tuple_size_v<T>>([]<size_t... I> static -> bool { return same_as_every<tuple_element_t<I, T>...>; }));

	template<class T, size_t N = 0>
	concept fixed_string_like = range_uses_traits_type<T> && uniform_tuple_like<T, N>;

	template<class F, size_t N>
	concept constexprifier_like = !!N && apply<N>([]<size_t... I> static -> bool { return same_as_every<call_template_t<F, I>...>; });

	template<size_t N, constexprifier_like<N> F>
	constexpr std::array constexprifier_table_v = apply<N>([]<size_t... I> static ->
		std::array<call_template_t<F, 0uz>, N> { return {(&std::remove_cvref_t<F>::template operator()<I>)...}; });

	template<size_t N, constexprifier_like<N> F, class... A>
	constexpr decltype(auto) constexprify(const size_t i, F && f, A &&... args) {
		if constexpr (std::is_member_function_pointer_v<call_template_t<F, 0uz>>) {
			return (std::forward<F>(f).*constexprifier_table_v<N, F>[i])(std::forward<A>(args)...);
		} else {
			return constexprifier_table_v<N, F>[i](std::forward<A>(args)...);
		}
	}



	template<class T>
	using propagate_const_t = type_in_const_t<([]<class U>(this const auto lambda, std::type_identity<U>) -> auto {
		if constexpr (pointer_like<U>) {
			return t<type_in_const_t<lambda(t<std::remove_pointer_t<U>>)> *const>;
		} else if constexpr (lvalue_reference_like<U>) {
			return t<type_in_const_t<lambda(t<std::remove_reference_t<U>>)> &>;
		} else if constexpr (rvalue_reference_like<U>) {
			return t<type_in_const_t<lambda(t<std::remove_reference_t<U>>)> &&>;
		} else {
			return t<const U>;
		}
	})(t<T>)>;

	template<class U, class V, class T>
	concept in_relation_with = cref_relation<T, const U &, const V &>;

	template<class U, class T>
	concept hashable_by = cref_invocable_r<T, size_t, const U &>;

	template<class U, template<class> class T>
	concept hashable_by_template = (hashable_by<U, T<U>> && movable_constructible_from<T<U>>);

	// https://mathworld.wolfram.com/Hypermatrix.html
	template<class T, size_t... N>
		requires (!!sizeof...(N))
	using hypermatrix_t = type_in_const_t<([]<size_t I1, size_t... I>(this const auto lambda, constant<I1>, const constant<I>... args) -> auto {
		if constexpr (sizeof...(I)) {
			return t<std::array<type_in_const_t<lambda(args...)>, I1>>;
		} else {
			return t<std::array<T, I1>>;
		}
	})(c<N>...)>;

	template<function_pointer_like auto INVOCABLE, auto... V>
	using lift_t = const_t<[]<class... A>(A &&... args) static -> decltype(auto)
		requires (std::invocable<const const_t<INVOCABLE> &, A..., const const_t<V> &...>)
	{
		return std::invoke(INVOCABLE, std::forward<A>(args)..., V...);
	}>;

	// Nedarome cast operacijos ant rezultato (kad, pavyzdžiui, leisti naudotojui pasirinkti gauti didesnį integral tipą),
	// nes tokia realizacija yra visados klaidinga. Naudotojas galės, jei norės pats tokią operaciją atlikti.
	template<std::integral X>
	constexpr std::make_unsigned_t<X> unsign(const X x) {
		return std::bit_cast<std::make_unsigned_t<X>>(x);
	}

	template<std::integral X>
	constexpr std::make_signed_t<X> sign(const X x) {
		return std::bit_cast<std::make_signed_t<X>>(x);
	}

	// https://en.cppreference.com/w/cpp/language/implicit_cast.html#Integral_conversions
	// Kai pirminis tipas yra signed ir rezultato tipas yra unsigned ir jis didesnis, tada reikšmė yra sign-extended. Todėl reikia šios funkcijos.
	template<std::integral T, std::integral X>
	constexpr T sign_cast(const X x) {
		if constexpr (std::unsigned_integral<T>)	return cast<T>(unsign(x));
		else										return cast<T>(x);
	}

	template<std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_exp2(const T x) {
		return value_v<U, 1> << x;
	}

	// T yra tipas, kurio bitus skaičiuosime, U nurodo kokiu tipu pateikti rezultatus.
	// [0, digits<T>) ∪ {0b(1)_digits<T>}
	template<std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_log2(const T x) {
		return (value_v<U, numeric_digits_v<std::make_signed_t<T>>>) - sign_cast<U>(std::countl_zero(x));
	}

	// Naudojamas size_t tipas kaip konstantos tipas, o ne sekančio dydžio integer tipas, nes konstanta
	// turėtų būti naudojama malloc ir panašiuose kontekstuose, o ten reikalaujama size_t tipo išraiška.
	// T turi turėti unique object representations, nes kitaip neišeis patikimai apskaičiuoti konstantos.
	//
	// Vietoje byte negalime naudoti uint8_t, nes jei sistemoje baitas būtų ne 8 bitų, tas tipas nebus apibrėžtas.
	template<uniquely_representable T>
	constexpr size_t representable_values_v = int_exp2(sizeof(T) * numeric_digits_v<std::byte>);

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

	template<class F, class... A>
		requires (out_unary_call_invocable<F, A...>)
	constexpr auto make_tuple(F && f = default_value, A &&... args) {
		using value_type = std::remove_reference_t<function_argument_t<call_template_t<F>>>;
		if constexpr (!sizeof...(A)) {
			value_type d;
			apply<tuple_size_v<value_type>>(std::forward<F>(f), d);
			return d;
		} else {
			value_type d = {std::forward<A>(args)...};
			apply<tuple_size_v<value_type>>(std::forward<F>(f), d);
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

	// Klasė naudinga naudoti su klasėmis, kurių move konstruktorius yra ištrintas, nes tokios klasės negali dalyvauti NRVO.
	// https://devblogs.microsoft.com/oldnewthing/20230612-00/?p=108329
	// Laikome konstantą, nes kintamojo nėra prasmės laikyti šioje klasėje, tokiu atveju reikėtų šios klasės nenaudoti.
	template<nonempty_unit_param_like T>
	struct make_wo_moving : unit<const T> {
		// Member types
		using typename unit<const T>::tuple_type;
		using unit_type = typename tuple_type::unit_type<0>;
		using typename unit_type::value_type,
			typename unit_type::reference, typename unit_type::const_reference,
			typename unit_type::pointer, typename unit_type::const_pointer;

		// Observers
		constexpr auto operator->() const {
			return to_pointer(unit_type::value);
		}

		constexpr const_pointer operator&() const {
			return std::addressof(unit_type::value);
		}

		constexpr operator const_reference() const {
			return unit_type::value;
		}

		constexpr decltype(auto) operator*() const {
			return to_reference(unit_type::value);
		}

		constexpr const_reference get() const {
			return unit_type::value;
		}

		// Special member functions
		template<std::invocable<reference> F>
		constexpr make_wo_moving(F && f) {
			std::invoke(std::forward<F>(f), unit_type::value);
		}

		template<std::invocable<reference> F, constructible_to<value_type> U = value_type>
		constexpr make_wo_moving(F && f, U && u) : tuple_type{std::forward<U>(u)} {
			std::invoke(std::forward<F>(f), unit_type::value);
		}
	};



	template<scoped_enum_like E>
	constexpr E to_enum(const std::underlying_type_t<E> u) {
		return std::bit_cast<E>(u);
	}

	namespace operators {
		template<scoped_enum_like E>
		constexpr bool operator!(const E rhs) {
			return !std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator+(const E rhs) {
			return std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator~(const E rhs) {
			return ~std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator|(const E lhs, const E rhs) {
			return std::to_underlying(lhs) | std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator|(const std::underlying_type_t<E> lhs, const E rhs) {
			return lhs | std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator|(const E lhs, const std::underlying_type_t<E> rhs) {
			return std::to_underlying(lhs) | rhs;
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator&(const E lhs, const E rhs) {
			return std::to_underlying(lhs) & std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator&(const std::underlying_type_t<E> lhs, const E rhs) {
			return lhs & std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator&(const E lhs, const std::underlying_type_t<E> rhs) {
			return std::to_underlying(lhs) & rhs;
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator^(const E lhs, const E rhs) {
			return std::to_underlying(lhs) ^ std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator^(const std::underlying_type_t<E> lhs, const E rhs) {
			return lhs ^ std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator^(const E lhs, const std::underlying_type_t<E> rhs) {
			return std::to_underlying(lhs) ^ rhs;
		}
	}



	// Galėtume vietoje _1 naudoti std::ignore. Bet minėtos konstantos tipas nėra tuščias.
	// Taip pat jei reiktų negalėtume to pačio parametro su skirtingomis konstantomis specializuoti.
	//
	// Comparators do not declare is_transparent, nes jų ir taip neišeitų panaudoti su c++ standarto konteineriais.
	//
	// Mes nurodome dešinės pusės operandą su template parametru, nes realizuojame šią gražią elgseną: sakykime turime
	// objektą tipo less<3>, šio objekto operator() grąžins true tik tada kai paduotas kintamasis bus mažesnis už 3.
	template<auto R = std::placeholders::_1>
	struct less {
		static constexpr const_t<R> value = R;

		template<std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l < R; }
	};

	template<>
	struct less<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l < R; }
	};

	template<auto R = std::placeholders::_1>
	struct less_equal {
		static constexpr const_t<R> value = R;

		template<std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l <= R; }
	};

	template<>
	struct less_equal<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l <= R; }
	};

	template<auto R = std::placeholders::_1>
	struct greater {
		static constexpr const_t<R> value = R;

		template<std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l > R; }
	};

	template<>
	struct greater<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l > R; }
	};

	template<auto R = std::placeholders::_1>
	struct greater_equal {
		static constexpr const_t<R> value = R;

		template<std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l >= R; }
	};

	template<>
	struct greater_equal<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l >= R; }
	};

	template<auto R = std::placeholders::_1>
	struct equal_to {
		static constexpr const_t<R> value = R;

		template<std::equality_comparable_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l == R; }
	};

	template<>
	struct equal_to<std::placeholders::_1> {
		template<auto R, std::equality_comparable_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l == R; }
	};

	template<auto R = std::placeholders::_1>
	struct not_equal_to {
		static constexpr const_t<R> value = R;

		template<std::equality_comparable_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l != R; }
	};

	template<>
	struct not_equal_to<std::placeholders::_1> {
		template<auto R, std::equality_comparable_with<const_t<R>> L>
		static constexpr bool operator()(const L & l) { return l != R; }
	};



	// Parameter pack negali susidėti iš daug elementų todėl šitą funkciją reiktų naudoti tik kai reikia mažai
	// iteracijų. Tačiau toks iteravimas greitesnis, nes nereikia prižiūrėti papildomo ciklo kintamojo.
	//
	// Greitaveika nenukenčia padavinėjant template parametrus todėl neturime funkcijos užklojimo kito.
	// P yra value tipas, nes funktoriai, kurie bus kviečiami daug kartų yra taip padavinėjami. Kitaip P tipas būtų rvalue reference.
	template<size_t N, class F>
	constexpr void repeat(F f) {
		apply<N>([&]<size_t... I> -> void { (aa::invoke<I>(f), ...); });
	}



	template<class T, class... A>
	using next_type_t = A...[(type_pack_index_v<T, A...>) + 1];

	template<class T>
	using next_unsigned_t = next_type_t<T, uint8_t, uint16_t, uint32_t, uint64_t>;

	template<class T>
	using prev_unsigned_t = next_type_t<T, uint64_t, uint32_t, uint16_t, uint8_t>;

	template<class T>
	using next_signed_t = next_type_t<T, int8_t, int16_t, int32_t, int64_t>;

	template<class T>
	using prev_signed_t = next_type_t<T, int64_t, int32_t, int16_t, int8_t>;

}



// Negalime tikrinti ar prieš šį momentą tuple_size<T> buvo deklaruotas tipas ar ne, nes įeitume į begalinį
// ciklą. Reiškia turi mums pats tipas pranešti ar jis nori būti laikomas kaip tuple like tipas.
template<aa::new_tuple_like T>
struct std::tuple_size<T> : aa::constant<T::tuple_size> {};

template<size_t I, aa::new_tuple_like T>
struct std::tuple_element<I, T> : std::type_identity<typename T::value_type<I>> {};
