#pragma once
#include <string>
#include <cmath>
#include <array>

using std::string;
using std::array;

namespace AGE2
{	
	struct Interval {
		const float left;
		const float right;

		constexpr Interval(const float left, const float right)
			:left(left), right(right)
		{}
	};
	
	template<unsigned N, template<unsigned V> class F>
	struct PerfTestFunction {
		static_assert(N > 0, "At least one dimension is required");
		
		static constexpr string name()
		{
			return F<N>::name;
		}
		static constexpr auto search_domain()
		{
			return F<N>::sd;
		}
		static constexpr float global_minimum()
		{
			return F<N>::global_minimum;
		}
		static float call(array<float, N> values)
		{
			return F<N>::call(values);
		}
	};

	//TODO: verificat functiile
	//FORMULA: 10n + SUM: FROM i=1 TO n [ pow(xi,2) - 10cos(2PIxi)]
	template<unsigned N>
	struct Rastrigin : PerfTestFunction<N, Rastrigin> {
		
		constexpr static char name[] = "rastrigin";

		constexpr static Interval sd{ -5.12f, 5.12f };
		
		constexpr static float global_minimum = 0.0f;

		constexpr static float call(array<float, N> values) {
			float sum = 0;
			for (unsigned i = 1; i <= N; ++i) {
				const auto xi = values[i - 1];

				sum += pow(xi, 2) - 10.0f * cosf(2.0f * float(M_PI) * xi);
			}

			const auto fx = 10 * N + sum;
			return fx;
		}
	};

	//FORMULA: 418.9829 * dim - SUM: FROM i=1 TO i=n [xi * sin(sqrt(abs(xi))) ]
	template<unsigned N>
	struct Schwefel : PerfTestFunction<N, Schwefel> {

		constexpr static char name[] = "schwefel";

		constexpr static Interval sd{ -500.0f, 500.0f };

		constexpr static float global_minimum = 0.0f;
		
		constexpr static float call(array<float, N> values) {
			float sum = 0;
			for (unsigned i = 1; i <= N; ++i) {
				const auto xi = values[i - 1];

				sum += xi * sin(sqrt(abs(xi)));
			}

			const auto fx = 418.9829f * N + sum;
			return fx;
		}
	};

	//FORMULA: SUM: FROM i=1 TO i=n [ exp(n,2) ]
	template<unsigned N>
	struct Sphere : PerfTestFunction<N, Sphere> {
		
		constexpr static char name[] = "sphere";

		constexpr static Interval sd{ -5.12f, 5.12f };

		constexpr static float global_minimum = 0.0f;

		constexpr static float call(array<float, N> values) {
			float sum = 0;
			for (unsigned i = 1; i <= N; ++i) {
				const auto xi = values[i - 1];

				sum += pow(xi, 2);
			}

			return sum;
		}
	};

	//FORMULA: SUM: FROM i=1 TO i=n [100 * pow((x[i+1] - pow(xi, 2)),2) + pow((1 - xi),2) ]
	template<unsigned N>
	struct Rosenbrock : PerfTestFunction<N, Rosenbrock> {

		constexpr static char name[] = "rosenbrock";

		constexpr static Interval sd{ -10.0f, 10.0f };

		constexpr static float global_minimum = 0.0f;

		constexpr static float call(array<float, N> values) {
			float sum = 0;
			for (unsigned i = 1; i < N; ++i) {//TODO: aici am impresia ca trebuia sa fie cv
				const auto xi = values[i - 1];
				const auto xip1 = values[i];

				sum += 100 * pow((xip1 - pow(xi, 2)), 2) + pow((1 - xi), 2);
			}

			return sum;
		}
	};

}