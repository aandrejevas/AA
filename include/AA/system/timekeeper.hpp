#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/time.hpp"
#include <concepts> // invocable
#include <functional> // invoke
#include <utility> // forward
#include <chrono> // steady_clock, duration, duration_cast



namespace aa {

	template<clock C = std::chrono::steady_clock>
	struct timekeeper {
		// Turi būti inicializuoti šitie kintamieji su ta pačia reikšme, nes kitaip neteisingai
		// veiktų resume funkcija jei sukūrus timekeeper objektą iškarto ji būtų iškviesta.
		C::rep begin = 0, end = 0;

		static AA_CONSTEXPR C::rep now() {
			return C::now().time_since_epoch().count();
		}

		AA_CONSTEXPR void reset() {
			begin = 0;
			end = 0;
		}

		AA_CONSTEXPR void start() {
			begin = now();
		}

		AA_CONSTEXPR void resume() {
			begin += (now() - end);
		}

		AA_CONSTEXPR void stop() {
			end = now();
		}

		template<class F, class... A>
			requires (std::invocable<F, A...>)
		AA_CONSTEXPR void measure(F &&f, A&&... args) {
			start();
			std::invoke(std::forward<F>(f), std::forward<A>(args)...);
			stop();
		}

		template<class F, class... A>
			requires (std::invocable<F, A...>)
		AA_CONSTEXPR void append(F &&f, A&&... args) {
			resume();
			std::invoke(std::forward<F>(f), std::forward<A>(args)...);
			stop();
		}

		// Pagal nutylėjimą, laikas pateikiamas sekundėmis.
		template<class D = std::chrono::duration<double>>
		AA_CONSTEXPR D::rep elapsed() const {
			return std::chrono::duration_cast<D>(typename C::duration{end - begin}).count();
		}
	};

	template<class... A>
	timekeeper(A&&...)->timekeeper<>;

}
