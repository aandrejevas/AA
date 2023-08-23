#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/time.hpp"
#include <chrono> // steady_clock, duration, duration_cast



namespace aa {

	template<clock C = std::chrono::steady_clock>
	struct timekeeper {
		// Member types
		using clock_type = C;
		using rep = typename clock_type::rep;
		using period = typename clock_type::period;
		using duration = typename clock_type::duration;
		using time_point = typename clock_type::time_point;



		// Member objects
		// Turi būti inicializuoti šitie kintamieji su ta pačia reikšme, nes kitaip neteisingai
		// veiktų resume funkcija jei sukūrus timekeeper objektą iškarto ji būtų iškviesta.
		rep begin = 0, end = 0;



		// Member functions
		static constexpr rep now() {
			return clock_type::now().time_since_epoch().count();
		}

		constexpr void reset() {
			begin = 0;
			end = 0;
		}

		constexpr void start() {
			begin = now();
		}

		constexpr void resume() {
			begin -= std::exchange(end, value_v<rep, 0>);
			begin += now();
		}

		constexpr void stop() {
			end = now();
		}

		template<class F, class... A>
			requires (std::invocable<F, A...>)
		constexpr void measure(F &&f, A&&... args) {
			start();
			std::invoke(std::forward<F>(f), std::forward<A>(args)...);
			stop();
		}

		template<class F, class... A>
			requires (std::invocable<F, A...>)
		constexpr void append(F &&f, A&&... args) {
			resume();
			std::invoke(std::forward<F>(f), std::forward<A>(args)...);
			stop();
		}

		template<class F, class... A>
			requires (std::invocable<F, A...>)
		constexpr void exclude(F &&f, A&&... args) {
			stop();
			std::invoke(std::forward<F>(f), std::forward<A>(args)...);
			resume();
		}

		// Negali tikrinimas būti (end == 0), nes reset() metodą iškvietus rodytų, kad running.
		constexpr bool is_running() const {
			return end < begin;
		}

		// Pagal nutylėjimą, laikas pateikiamas sekundėmis.
		template<class D = std::chrono::duration<double>>
		constexpr typename D::rep elapsed() const {
			return std::chrono::duration_cast<D>(duration{end - begin}).count();
		}
	};

	template<class T = typename std::chrono::steady_clock::rep, class U = typename std::chrono::steady_clock::rep>
	timekeeper(const T & = default_value, const U & = default_value) -> timekeeper<>;

}
