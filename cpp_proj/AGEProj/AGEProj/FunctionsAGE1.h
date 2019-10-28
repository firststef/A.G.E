#pragma once
#define _USE_MATH_DEFINES
#include "AGE.h"
#include <bitset>
#include <string>
#include <utility>
#include <math.h>
#include <cmath>

namespace AGE1
{
	constexpr int L = sizeof(unsigned long) * 8;
	//on Windows sizeof(unsigned long) eq 4, but on Linux sizeof(unsigned long) is 8
	//to use the std::bitset function .to_ulong() safely, we use the above init

	using bitstring = std::bitset<L>;

	struct Converter
	{
		union
		{
			unsigned long ul;
			float f;
		};
	};
	
	struct Interval {
		const float left;
		const float right;
	};

	template<unsigned N>
	struct NDimensionPoint
	{
		bitstring coordinates[N];
	};
	
	template<unsigned N>
	struct ReturnWrapper
	{
		NDimensionPoint<N> point;
		float func_value;
	};

	constexpr float bitstring_to_interval(float left, float right, unsigned number)
	{
		return left + number / (sizeof(unsigned long) == 4 ? two_at_32_m_1: two_at_64_m_1) * (right - left) ;
	}

	template<unsigned N>
	struct PerfTestFunction {
		const std::string name;

		const Interval search_domain;
		const float global_minimum;

		PerfTestFunction(const std::string name, const Interval domain, const float global_minimum)
			: name(std::move(name)), search_domain(domain), global_minimum(global_minimum) {}

		virtual float call(bitstring values[N]) = 0;
	};

	//FORMULA: 10n + SUM: FROM i=1 TO n [ pow(xi,2) - 10cos(2PIxi)]
	template<unsigned N>
	struct Rastrigin : PerfTestFunction<N> {
		Rastrigin() : PerfTestFunction<N>(std::string("rastrigin"), Interval{ -5.12f, 5.12f }, 0.0f) {}

		float call(bitstring values[N]) override {
			float sum = 0;
			for (unsigned i = 1; i <= N; ++i) {
				const Converter c{ values[i - 1].to_ulong() };

				const float xi = bitstring_to_interval(this->search_domain.left, this->search_domain.right, c.ul);
				sum += pow(xi, 2) - 10 * cos(2 * M_PI * xi);
			}

			const auto fx = 10 * N + sum;
			return fx;
		}
	};

	//FORMULA: 418.9829 * dim - SUM: FROM i=1 TO i=n [xi * sin(sqrt(abs(xi))) ]
	template<unsigned N>
	struct Schwefel : PerfTestFunction<N> {
		Schwefel() : PerfTestFunction<N>(std::string("schwefel"), Interval{ -500.0f, 500.0f }, 0.0f) {}

		float call(bitstring values[N]) override {
			float sum = 0;
			for (unsigned i = 1; i <= N; ++i) {
				const Converter c{ values[i - 1].to_ulong() };

				const float xi = bitstring_to_interval(this->search_domain.left, this->search_domain.right, c.ul);

				sum += xi * sin(sqrt(abs(xi)));
			}

			const auto fx = 418.9829f * N + sum;
			return fx;
		}
	};

	//FORMULA: SUM: FROM i=1 TO i=n [ exp(n,2) ]
	template<unsigned N>
	struct Sphere : PerfTestFunction<N> {
		Sphere() : PerfTestFunction<N>(std::string("sphere"), Interval{ -5.12f, 5.12f }, 0.0f) {}

		float call(bitstring values[N]) override {
			float sum = 0;
			for (unsigned i = 1; i <= N; ++i) {
				const Converter c{ values[i - 1].to_ulong() };

				const float xi = c.f;
				sum += pow(xi, 2);
			}

			return sum;
		}
	};

	//FORMULA: SUM: FROM i=1 TO i=n [100 * pow((x[i+1] - pow(xi, 2)),2) + pow((1 - xi),2) ]
	template<unsigned N>
	struct Rosenbrock : PerfTestFunction<N> {
		Rosenbrock() : PerfTestFunction<N>(std::string("rosenbrock"), Interval{ -10.0f, 10.0f }, 0.0f) {}

		float call(bitstring values[N]) override {
			float sum = 0;

			Converter c{ values[0].to_ulong() };
			float xip1;
			bitstring_to_interval(this->search_domain.left, this->search_domain.right, c.ul);

			for (unsigned i = 1; i < N; ++i) {
				const float xi = xip1;
				c.ul = values[i].to_ulong();
				xip1 = bitstring_to_interval(this->search_domain.left, this->search_domain.right, c.ul);

				sum += 100 * pow((xip1 - pow(xi, 2)), 2) + pow((1 - xi), 2);
			}

			return sum;
		}
	};

	template<unsigned N>
	struct PerfTestFunctor {
		PerfTestFunction<N> &function;

		float operator()(bitstring values[N]) { return function.call(values); }
	};

}