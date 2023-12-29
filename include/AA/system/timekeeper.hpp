#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/time.hpp"
#include <chrono> // steady_clock, duration, duration_cast



namespace aa {

	template<clock_like C = std::chrono::steady_clock>
	struct timekeeper {
		// Member types
		using clock_type = C;
		using rep = rep_in_use_t<clock_type>;
		using period = period_in_use_t<clock_type>;
		using duration = duration_in_use_t<clock_type>;
		using time_point = time_point_in_use_t<clock_type>;



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
			begin -= std::exchange(end, default_v<rep>);
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
		constexpr rep_in_use_t<D> elapsed() const {
			return std::chrono::duration_cast<D>(duration{end - begin}).count();
		}
	};

	template<class T = rep_in_use_t<std::chrono::steady_clock>, class U = rep_in_use_t<std::chrono::steady_clock>>
	timekeeper(const T & = default_value, const U & = default_value) -> timekeeper<>;

}
