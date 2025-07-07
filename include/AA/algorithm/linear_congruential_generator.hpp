#pragma once

#include "../metaprogramming/general.hpp"
#include "arithmetic.hpp"
#include <chrono> // steady_clock
#include <numeric> // gcd



namespace aa {

	// lcg būsenos nėra ordered, jos sudaro ciklą.
	//
	// requires paaiškinimas, kad prie visų SEED reikšmių, periodas būtų MODULUS reikia tinkamų parametrų.
	// 1) MODULUS ir INCREMENT turi būti tarpusavyje pirminiai skaičiai.
	// 2) MULTIPLIER-1 turi dalintis iš visų MODULUS pirminių dauginamųjų.
	// 3) MULTIPLIER-1 turi dalintis iš 4 jei MODULUS dalinasi iš 4.
	// Kadangi MODULUS pirminė faktorizacija yra lygi 2^n tai mūsų atveju 3 sąlyga apima 2.
	// https://en.wikipedia.org/wiki/Linear_congruential_generator
	// Parinktos konstantos priderintos darbui su moduliu 2^32 todėl T sakome, kad yra uint32_t.
	template<std::unsigned_integral T = uint32_t, T A = 0x8088405, T C = 1>
		requires (std::gcd(representable_values_v<T>, C) == 1 && !remainder<4>(A - 1))
	struct linear_congruential_generator {
		// Member types
		using result_type = T;
		using modulus_type = next_unsigned_t<result_type>;



		// Member constants
		// https://cp-algorithms.com/algebra/module-inverse.html
		// https://en.wikipedia.org/wiki/Euler%27s_totient_function
		// https://en.wikipedia.org/wiki/Modular_multiplicative_inverse#Using_Euler's_theorem
		static constexpr modulus_type
			modulus = representable_values_v<result_type>;
		static constexpr result_type
			multiplier = A, increment = C,
			inverse = pow(multiplier, (modulus >> 1) - 1);



		// Characteristics
		// Negalime paversti į konstantas, nes standartinės bibliotekos generatoriai taip pat turi tokias funkcijas.
		static consteval result_type min() { return numeric_min; }
		static consteval result_type max() { return numeric_max; }



		// Special member functions
		constexpr linear_congruential_generator() { seed(); }
		constexpr linear_congruential_generator(const result_type s) { seed(s); }



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
		result_type state;



		// Generation
		// [0, MODULUS)
		constexpr result_type next() { return (state = (state * multiplier) + increment); }
		constexpr result_type prev() { return (state = inverse * (state - increment)); }
		constexpr result_type curr() const { return state; }

		constexpr result_type operator()() { return next(); }

		// https://www.nayuki.io/page/fast-skipping-in-a-linear-congruential-generator
		// https://docs.rs/randomize/2.2.2/src/randomize/lcg.rs.html#99-114
		template<std::unsigned_integral U>
		constexpr result_type jump(U n) {
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
		constexpr result_type dist(const linear_congruential_generator o) const {
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
		// steady_clock naudojame, nes jis negali grįžti į tą patį momentą.
		constexpr void seed() {
			state = sign_cast<result_type>(std::chrono::steady_clock::now().time_since_epoch().count());
		}

		constexpr void seed(const result_type s) {
			state = s;
		}
	};

}
