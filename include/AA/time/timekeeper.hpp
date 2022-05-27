#pragma once

#include "../metaprogramming/clock.hpp"
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

		static inline constexpr C::rep now() {
			return C::now().time_since_epoch().count();
		}

		inline constexpr void reset() {
			begin = 0;
			end = 0;
		}

		inline constexpr void start() {
			begin = now();
		}

		inline constexpr void resume() {
			begin += (now() - end);
		}

		inline constexpr void stop() {
			end = now();
		}

		template<class F, class... A>
			requires (std::invocable<F, A...>)
		inline constexpr void measure(F &&f, A&&... args) {
			start();
			std::invoke(std::forward<F>(f), std::forward<A>(args)...);
			stop();
		}

		template<class F, class... A>
			requires (std::invocable<F, A...>)
		inline constexpr void append(F &&f, A&&... args) {
			resume();
			std::invoke(std::forward<F>(f), std::forward<A>(args)...);
			stop();
		}

		// Pagal nutylėjimą, laikas pateikiamas sekundėmis.
		template<class D = std::chrono::duration<double>>
		inline constexpr D::rep elapsed() const {
			return std::chrono::duration_cast<D>(typename C::duration{end - begin}).count();
		}
	};

}
