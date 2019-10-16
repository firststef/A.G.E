#pragma once
#include <string>
#include <AGE.h>

template<unsigned N>
struct PerfTestFunction {
	std::string name;
	
	Interval search_domain;
	float global_minimum;

	PerfTestFunction(std::string name, Interval domain, float global_minimum)
		:name(name), search_domain(domain), global_minimum(global_minimum)
	{}

	virtual float call(float values[N]) = 0;
};

//FORMULA: 10n + SUM: FROM i=1 TO n [ pow(xi,2) - 10cos(2PIxi)]
template<unsigned N>
struct Rastrigin : PerfTestFunction<N>
{
	Rastrigin() : PerfTestFunction<N>(std::string("rastrigin"), Interval{ -5.12f, 5.12f }, 0.0f) {}

	float call (float values[N]) override{
		float sum = 0;
		for (unsigned i = 1; i <= N; ++i) {
			const auto xi = values[i - 1];

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

	float call (float values[N]) override {
		float sum = 0;
		for (unsigned i = 1; i <= N; ++i) {
			const auto xi = values[i - 1];

			sum += xi * sin(sqrt(abs( xi)));
		}

		const auto fx = 418.9829 * N + sum;
		return fx;
	}
};

//FORMULA: SUM: FROM i=1 TO i=n [ exp(n,2) ]
template<unsigned N>
struct Sphere : PerfTestFunction<N> {
	Sphere() : PerfTestFunction<N>(std::string("sphere"), Interval{ -5.12f, 5.12f }, 0.0f) {}

	float call (float values[N]) override {
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
struct Rosenbrock : PerfTestFunction<N> {
	Rosenbrock() : PerfTestFunction<N>(std::string("rosenbrock"), Interval{ -10.0f, 10.0f }, 0.0f) {}

	float call (float values[N]) override {
		float sum = 0;
		for (unsigned i = 1; i < N; ++i) {
			const auto xi = values[i - 1];
			const auto xip1 = values[i];

			sum += 100 * pow((xip1 - pow(xi, 2)), 2) + pow((1 - xi), 2);
		}

		return sum;
	}
};

template<unsigned N>
struct PerfTestFunctor
{
	PerfTestFunction<N>& function;
	float operator()(float values[N]) { return function.call(values); }
};