#pragma once
#include "FunctionsAGE2.h"
#include "constexpr_functions.h"
#include "json.hpp"

#include <iostream>
#include <bitset>

using std::bitset;
using std::iota;
using std::shuffle;
using std::max_element;
using std::min_element;
using std::numeric_limits;
using std::vector;

#ifndef CONCAT2
#define CONCAT2(x, y) x##y
#endif
#ifndef CONCAT3
#define CONCAT3(x, y, z) x##y##z
#endif
#ifndef CONCAT4
#define CONCAT4(x, y, z, w) x##y##z##w
#endif

#define REGISTER_ALGORITHM(algorithm, N)\
HeuristicRunWrapper<N> CONCAT4(run_,algorithm,_,N);\
for (unsigned repeat = 0; repeat < 30; ++repeat)\
{\
	auto begin = std::chrono::system_clock::now();\
\
	CONCAT4(run_,algorithm,_,N).func_values[repeat] = algorithm<N, Function>();\
	auto finish = std::chrono::system_clock::now();\
	CONCAT4(run_,algorithm,_,N).deltas[repeat] = std::chrono::duration_cast<std::chrono::microseconds>(finish - begin).count();\
}\
std::string CONCAT4(stringified_run_,algorithm,_,N)[30];\
for (unsigned repeat = 0; repeat < 30; ++repeat)\
{\
	CONCAT4(stringified_run_,algorithm,_,N)[repeat] = std::to_string(CONCAT4(run_,algorithm,_,N).func_values[repeat]);\
}\

#define TRACE_ALGORITHM(algorithm, V, filter_size)\
vector<float> CONCAT4(run_evolution_,algorithm,_,V) = CONCAT2(traceable_, algorithm)<V, Function>();\
vector<float> CONCAT4(less_run_evolution_, algorithm, _, V);\
if (CONCAT4(run_evolution_, algorithm, _, V).size() >= filter_size){\
	for (unsigned i = 0; i < CONCAT4(run_evolution_, algorithm, _, V).size(); i++)\
		if (i % (CONCAT4(run_evolution_, algorithm, _, V).size() / filter_size) == 0)\
			CONCAT4(less_run_evolution_, algorithm, _, V).push_back(CONCAT4(run_evolution_, algorithm, _, V)[i]);\
} else {\
	CONCAT4(less_run_evolution_,algorithm,_,V) = CONCAT4(run_evolution_,algorithm,_,V);\
}

#undef DUMP_ANALYSIS_TO_JSON
#define DUMP_ANALYSIS_TO_JSON(algorithm, N, V)\
{\
	{"algorythm-name",#algorithm},\
	{"dimension", N},\
	{"results", CONCAT4(stringified_run_,algorithm,_,N)},\
	{"deltas", CONCAT4(run_,algorithm,_,N).deltas},\
}\

#define DUMP_TRACE_TO_JSON(algorithm, V, run_evolution_)\
{CONCAT2(#run_evolution_, #algorithm), CONCAT4(less_run_evolution_,algorithm,_,V)}

namespace AGE2
{
	template<unsigned L, unsigned N>
	struct NDimensionPoint
	{
		static unsigned seed;
		
		bitset<L> coordinates[N];

		void mutate(float prob)
		{
			for (unsigned i = 0; i < N; ++i) {
				for (unsigned j = 0; j < L; ++j)
				{
					if (rand() % 1000 < prob * 1000)
						coordinates[i][j].flip();
				}
			}
		}

		//Crossover
		void operator& (NDimensionPoint& other)
		{
			const unsigned ch_pos = rand() % (N * L);
			const unsigned ch_coord = ch_pos / (N*L);

			for (unsigned j = ch_pos % (N*L); j < L; ++j)
			{
				bool b = coordinates[ch_coord][j];
				coordinates[ch_coord][j] = other.coordinates[ch_coord][j];
				other.coordinates[ch_coord][j] = b;
			}

			for (unsigned i = ch_coord + 1; i < N; ++i)
			{
				std::swap(coordinates[i], other.coordinates[i]);
			}
		}

		array<float, N> to_float(float left, float right)
		{
			array<float, N> arr;
			for (unsigned i = 0; i < N; ++i)
			{
				arr[i] = left + coordinates[i].to_ulong() * (right - left) / (pow_c(2, L) - 1);
			}
			return arr;
		}
	};
	template<unsigned L, unsigned N>
	unsigned NDimensionPoint<L, N>::seed = []() {const auto seed = time(nullptr);  srand(seed); return seed; }();
	
	template<unsigned N, template<unsigned V> class Function>
	float genetic_algorithm() {

		//CONSTANTS
		constexpr unsigned pop_size = 100;
		constexpr unsigned gen_max = 1000;
		constexpr float p_m = 0.01f;
		constexpr float p_c = 0.3f;

		float best_fit = 1.5f;
		float average_fit = 1.1f;
		float worst_fit = 1.1f;
		
		using pressure_function = PerfTestFunction<N, Function>;
		auto engine = std::mt19937{ std::random_device{}() };
		
		constexpr auto interval = pressure_function::search_domain();
		constexpr unsigned L = ceil_log2((interval.right - interval.left) * pow_c(10, 5));

		//VARS
		float global_best = numeric_limits<float>::infinity();

		//RUN
		for (unsigned run = 0; run < 15; run++) {
			array<NDimensionPoint<L, N>, pop_size> pop;
			unsigned current_generation = 0;

			float best = numeric_limits<float>::infinity();

			auto stop_condition = [&]() ->bool
			{
				if (current_generation % 30 == 0)
				{
					best_fit += 1;
					if (worst_fit > 1.01)
						worst_fit -= 0.01;
				}
				return current_generation++ > gen_max;
			};

			//Init phase
			for (auto& c : pop)
			{
				c.mutate(0.5f);
			}

			do
			{
				array<unsigned, pop_size / 2> other_c_index;
				iota(other_c_index.begin(), other_c_index.end(), pop_size / 2);
				shuffle(other_c_index.begin(), other_c_index.end(), engine);

				//Mutation phase
				for (auto& c : pop)
				{
					c.mutate(p_m);
				}

				//Crossing phase
				shuffle(pop.begin(), pop.end(), engine);
				for (unsigned c = 0; c < pop_size / 2; ++c)
				{
					if (double(engine() % 1000) / 1000.0f < p_c)
						pop[c] & pop[other_c_index[c]];
				}

				//Selection phase
				float values[pop_size];
				float min = numeric_limits<float>::infinity();
				float max = -numeric_limits<float>::infinity();
				for (unsigned i = 0; i < pop_size; i++)
				{
					values[i] = pressure_function::call(pop[i].to_float(interval.left, interval.right));
					if (values[i] < min)
						min = values[i];
					else if (values[i] > max)
						max = values[i];
				}

				//WHEEL_OF_FORTUNE
				float fitness_values[pop_size];
				for (unsigned i = 0; i < pop_size; i++)
				{
					if (values[i] == max)
						fitness_values[i] = worst_fit * max - values[i];
					else if (values[i] == min)
						fitness_values[i] = best_fit * max - values[i];
					else
						fitness_values[i] = average_fit * max - values[i];
				}

				float wheel_values[pop_size];
				wheel_values[0] = fitness_values[0];
				for (unsigned i = 1; i < pop_size; i++)
				{
					wheel_values[i] = wheel_values[i - 1] + fitness_values[i];
				}

				decltype(pop) new_pop;
				for (unsigned i = 0; i < pop_size; ++i)
				{
					const auto stop_poz = float(engine() % 1000) / 1000.0f * wheel_values[pop_size - 1];

					unsigned idx = -1;
					for (unsigned j = 0; j < pop_size; j++)
					{
						if (stop_poz <= wheel_values[j])
						{
							idx = j;
							break;
						}
					}

					new_pop[i] = pop[idx];
				}
				pop = new_pop;
				best = min;
			} while (!stop_condition());

			if (best < global_best)
				global_best = best;
		}
		
		return global_best;
	}

	template<unsigned N, template<unsigned V> class Function>
	vector<float> traceable_genetic_algorithm() {

		//CONSTANTS
		constexpr unsigned pop_size = 100;
		constexpr unsigned gen_max = 1000;
		constexpr float p_m = 0.01f;
		constexpr float p_c = 0.3f;

		float best_fit = 1.5f;
		float average_fit = 1.1f;
		float worst_fit = 1.1f;

		using pressure_function = PerfTestFunction<N, Function>;
		auto engine = std::mt19937{ std::random_device{}() };

		constexpr auto interval = pressure_function::search_domain();
		constexpr unsigned L = ceil_log2((interval.right - interval.left) * pow_c(10, 5));

		//VARS
		vector<float> run_evolution;

		array<NDimensionPoint<L, N>, pop_size> pop;
		unsigned current_generation = 0;

		float best = numeric_limits<float>::infinity();

		auto stop_condition = [&]() ->bool
		{
			if (current_generation % 30 == 0)
			{
				best_fit += 1;
				if (worst_fit > 1)
					worst_fit -= 0.01;
			}
			return current_generation++ > gen_max;
		};

		//Init phase
		for (auto& c : pop)
		{
			c.mutate(0.5f);
		}

		do
		{
			array<unsigned, pop_size / 2> other_c_index;
			iota(other_c_index.begin(), other_c_index.end(), pop_size / 2);
			shuffle(other_c_index.begin(), other_c_index.end(), engine);

			//Mutation phase
			for (auto& c : pop)
			{
				c.mutate(p_m);
			}

			//Crossing phase
			shuffle(pop.begin(), pop.end(), engine);
			for (unsigned c = 0; c < pop_size / 2; ++c)
			{
				if (double(engine() % 1000) / 1000.0f < p_c)
					pop[c] & pop[other_c_index[c]];
			}

			//Selection phase
			float values[pop_size];
			float min = numeric_limits<float>::infinity();
			float max = -numeric_limits<float>::infinity();
			for (unsigned i = 0; i < pop_size; i++)
			{
				values[i] = pressure_function::call(pop[i].to_float(interval.left, interval.right));
				if (values[i] < min)
					min = values[i];
				else if (values[i] > max)
					max = values[i];
			}

			//WHEEL_OF_FORTUNE
			float fitness_values[pop_size];
			for (unsigned i = 0; i < pop_size; i++)
			{
				if (values[i] == max)
					fitness_values[i] = worst_fit * max - values[i];
				else if (values[i] == min)
					fitness_values[i] = best_fit * max - values[i];
				else
					fitness_values[i] = average_fit * max - values[i];
			}

			float wheel_values[pop_size];
			wheel_values[0] = fitness_values[0];
			for (unsigned i = 1; i < pop_size; i++)
			{
				wheel_values[i] = wheel_values[i - 1] + fitness_values[i];
			}

			decltype(pop) new_pop;
			for (unsigned i = 0; i < pop_size; ++i)
			{
				const auto stop_poz = float(engine() % 1000) / 1000.0f * wheel_values[pop_size - 1];

				unsigned idx = -1;
				for (unsigned j = 0; j < pop_size; j++)
				{
					if (stop_poz <= wheel_values[j])
					{
						idx = j;
						break;
					}
				}

				new_pop[i] = pop[idx];
			}
			pop = new_pop;
			best = min;

			run_evolution.push_back(best);
			
		} while (!stop_condition());

		return run_evolution;
	}

	template<unsigned N>
	struct HeuristicRunWrapper
	{
		array<float, 30> func_values;
		array<unsigned, 30> deltas;
	};

	template<template <unsigned> class Function>
	struct AlgorythmAnalyzerAGE2
	{
		nlohmann::json create_json()
		{
			//GA
			printf("Started GA on %s\n", Function<1>::name);
			//REGISTER_ALGORITHM(genetic_algorithm, 5);
			//REGISTER_ALGORITHM(genetic_algorithm, 10);
			//REGISTER_ALGORITHM(genetic_algorithm, 30);
			TRACE_ALGORITHM(genetic_algorithm, 30, 100);
			printf("Ended GA on %s\n", Function<1>::name);

			nlohmann::json j = {
				{"function-name", Function<1>::name},
				{"global_minimum", std::to_string(Function<1>::global_minimum)},
				{"analysis",	nlohmann::json::array({
				//DUMP_ANALYSIS_TO_JSON(genetic_algorithm, 5),
				//DUMP_ANALYSIS_TO_JSON(genetic_algorithm, 10),
				//DUMP_ANALYSIS_TO_JSON(genetic_algorithm, 30)
				})
				},DUMP_TRACE_TO_JSON(genetic_algorithm, 30, run_evolution_)
			};

			return j;
		}

		void create_output_file()
		{
			auto j = create_json();
			std::string name("output_age2_");
			name += Function<1>::name;
			name += ".json";
			std::ofstream out(name.c_str());
			out << j << std::endl;
		}
	};
}