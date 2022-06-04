#pragma once

#include "../metaprogramming/general.hpp"
#include "arithmetic.hpp"
#include <cstddef> // size_t
#include <chrono> // system_clock
#include <concepts> // unsigned_integral
#include <limits> // numeric_limits
#include <numeric> // gcd



namespace aa {

	// Idėja kilo turėti iterator tipą, bet tai bloga idėja, nes lcg reiktų naudoti kaip function object.
	// lcg būsenos nėra ordered, jos sudaro ciklą.
	// https://en.wikipedia.org/wiki/Linear_congruential_generator
	template<std::unsigned_integral T, T A, T C>
	struct lcg {
		// Member types
		using result_type = T;
		using modulus_type = next_unsigned_t<result_type>;
		using generator_type = lcg<T, A, C>;



		//  Member constants
		// https://cp-algorithms.com/algebra/module-inverse.html
		// https://en.wikipedia.org/wiki/Euler%27s_totient_function
		// https://en.wikipedia.org/wiki/Modular_multiplicative_inverse#Using_Euler's_theorem
		inline static constexpr const modulus_type modulus = representable_values_v<result_type>;
		inline static constexpr const result_type multiplier = A, increment = C, decremented_multiplier = multiplier - 1,
			inverse = power(multiplier, (modulus >> 1) - 1);

		// Nedarome requires, nes tame kontekste neturėtume modulus ir decremented_multiplier konstantų.
		static_assert(std::gcd<size_t, size_t>(modulus, increment) == 1
			&& !remainder<2uz, size_t>(decremented_multiplier) && !remainder<4uz, size_t>(decremented_multiplier));



		// Characteristics
		inline static consteval result_type min() { return std::numeric_limits<result_type>::min(); }
		inline static consteval result_type max() { return std::numeric_limits<result_type>::max(); }



		// Member objects
		// Sena praktika yra inicializuoti pradinę būseną su esamu laiku.
		// Paleidus tą patį .exe pradinė būsena būsena turi būti kita todėl negalima naudoti compile time SEED'o (__TIME__).
		//
		// Generuojami 32 bitų atsitiktiniai skaičiai [0, 2^32), ne 64 bitų atsitiktiniai skaičiai, nes dabartinis
		// metodas, kurio pagalba galima generuoti atsitiktinius skaičius bet kokiame intervale, mus riboja.
		//
		// Galima generuoti atsitiktinius skaičius bet kokiame intervale naudojant slankiojo kablelio skaičius,
		// bet nors šališkumas metodo toks pat kaip ir dabar naudojamo metodo, jo greitaveika yra mažesnė.
		// Greičiausias būdas kaip generuoti nešališkus atsitiktinius skaičius intervale: https://arxiv.org/abs/1805.10941.
		// Naudojami metodai su šališkumu, nes mums svarbiausia greitaveika, o šališkumas nėra didelis.
		result_type state = static_cast<result_type>(std::chrono::system_clock::now().time_since_epoch().count());



		// Generation
		// [0, MODULUS)
		inline constexpr result_type next() { return (state = (state * multiplier) + increment); }
		inline constexpr result_type prev() { return (state = inverse * (state - increment)); }
		inline constexpr result_type curr() const { return state; }

		inline constexpr result_type operator()() { return next(); }

		// https://www.nayuki.io/page/fast-skipping-in-a-linear-congruential-generator
		// https://docs.rs/randomize/2.2.2/src/randomize/lcg.rs.html#99-114
		template<std::unsigned_integral U>
		inline constexpr result_type jump(U n) {
			result_type acc_mult = 1, acc_plus = 0,
				cur_mult = multiplier, cur_plus = increment;
			do {
				if (n & 1) {
					acc_mult *= cur_mult;
					(acc_plus *= cur_mult) += cur_plus;
				}
				if (n >>= 1) {
					cur_plus *= cur_mult + 1;
					cur_mult *= cur_mult;
				} else break;
			} while (true);
			return (state = (state * acc_mult) + acc_plus);
		}

		// https://math.stackexchange.com/questions/2008585/computing-the-distance-between-two-linear-congruential-generator-states
		inline constexpr result_type dist(const generator_type &o) const {
			result_type d = 0, mask = 1, t_state = state,
				cur_mult = multiplier, cur_plus = increment;
			do {
				if ((t_state ^ o.state) & mask) {
					(t_state *= cur_mult) += cur_plus;
					d |= mask;
				}
				if (t_state != o.state) {
					cur_plus *= cur_mult + 1;
					cur_mult *= cur_mult;
					mask <<= 1;
				} else break;
			} while (true);
			return d;
		}

		// Seeding
		inline constexpr void seed() {
			state = static_cast<result_type>(std::chrono::system_clock::now().time_since_epoch().count());
		}

		inline constexpr void seed(const result_type s) {
			state = s;
		}
	};

	using pascal_lcg = lcg<prev_unsigned_t<size_t>, 0x8088405, 1>;

}
