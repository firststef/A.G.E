#pragma once
#define _USE_MATH_DEFINES
#include "AGE.h"
#include <bitset>
#include <string>
#include <math.h>
#include <cmath>

namespace AGE1p
{
	template<unsigned Ex, unsigned Man>
	struct fixed_point_number
	{
		std::bitset<Ex + Man> value;

		float to_float() const
		{
			float number = 0.0f;
			
			for (int i=1; i<=Ex; ++i)
			{
				number += value[Ex - i] * pow(2, i-1);
			}
			for (int i=0; i<Man; ++i)
			{
				number += value[Ex + i] * pow(2, -i);//de verificat daca merge
			}

			return number;
		}
	};

	struct Interval {
		const float left;
		const float right;
	};

	template<unsigned Ex, unsigned Man, unsigned N>
	struct NDimensionPoint
	{
		fixed_point_number<Ex,Man> coordinates[N];
	};

	template<unsigned Ex, unsigned Man, unsigned N>
	struct ReturnWrapper
	{
		NDimensionPoint<Ex, Man, N> point;
		float func_value;
	};

	constexpr float bitstring_to_interval(float left, float right, std::uint32_t number)
	{
		return left + float((double(number)) / double(two_at_32_m_1) * double(right - left));
	}

	template<unsigned N>
	struct PerfTestFunction {
		const std::string name;

		const Interval search_domain;
		const float global_minimum;

		PerfTestFunction(std::string name, const Interval domain, const float global_minimum)
			: name(name), search_domain(domain), global_minimum(global_minimum) {}

		virtual float call(float values[N]) = 0;
	};

	//FORMULA: x^3 - 60x ^ 2 + 900*x + 100
	template<unsigned N>
	struct GenericFunction : PerfTestFunction<N> {
		GenericFunction() : PerfTestFunction<N>(std::string("generic"), Interval{ 0.0f, 31.0f }, 0.0f) {}

		float call(float values[N]) override {
			float sum = 0;
			for (unsigned i = 0; i < N; ++i) {
				const auto xi = values[i];
				
				sum += pow(xi, 3) - 60 * pow(xi, 2) + 900 * xi + 100;
			}

			return sum;
		}
	};

	template<unsigned N>
	struct PerfTestFunctor {
		PerfTestFunction<N> &function;

		float operator()(float values[N]) { return function.call(values); }
	};

}