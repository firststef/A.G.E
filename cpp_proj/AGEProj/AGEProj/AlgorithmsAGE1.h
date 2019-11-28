#pragma once
#include "FunctionsAGE1.h"
#include "json.hpp"
#include <ctime>
#include <chrono>
#include <random>

#ifndef CONCAT2
#define CONCAT2(x, y) x##y
#endif
#ifndef CONCAT3
#define CONCAT3(x, y, z) x##y##z
#endif
#ifndef CONCAT4
#define CONCAT4(x, y, z, w) x##y##z##w
#endif

#undef REGISTER_DIMENSION
#define REGISTER_DIMENSION(N)\
Function<N> CONCAT3(f, _dim_, N); \
Problem<N> CONCAT3(problem, _dim_, N) { {CONCAT3(f, _dim_, N)} }; \

#undef REGISTER_FUNCTION
#define REGISTER_FUNCTION(function, N)\
HeuristicRunWrapper<N> CONCAT4(run_,function,_,N);\
for (unsigned repeat = 0; repeat < 30; ++repeat)\
{\
	auto begin = std::chrono::system_clock::now();\
\
	const auto res = function<N>(CONCAT3(problem,_dim_,N));\
	CONCAT4(run_,function,_,N).func_values[repeat] = res.func_value;\
	for (unsigned i = 0; i < 2; ++i)\
	{\
		Converter c;\
		c.ul = res.point.coordinates[i].to_ulong();\
		CONCAT4(run_,function,_,N).coordinates[repeat][i] = c.f;\
	}\
\
	auto finish = std::chrono::system_clock::now();\
	CONCAT4(run_,function,_,N).deltas[repeat] = std::chrono::duration_cast<std::chrono::microseconds>(finish - begin).count();\
}\
std::string CONCAT4(stringified_run_,function,_,N)[30];\
for (unsigned repeat = 0; repeat < 30; ++repeat)\
{\
	CONCAT4(stringified_run_,function,_,N)[repeat] = std::to_string(CONCAT4(run_,function,_,N).func_values[repeat]);\
}

#undef DUMP_ANALYSIS_TO_JSON
#define DUMP_ANALYSIS_TO_JSON(function, N)\
{\
	{"algorythm-name",#function},\
	{"dimension", N},\
	{"results", CONCAT4(stringified_run_,function,_,N)},\
	{"deltas", CONCAT4(run_,function,_,N).deltas},\
	{"coordinates", CONCAT4(run_,function,_,N).coordinates}\
}\

#define TRACE_ALGORITHM(algorithm, V)\
std::vector<float> CONCAT4(run_evolution_,algorithm,_,V) = CONCAT2(traceable_, algorithm)<V>(CONCAT3(problem, _dim_, V));

#define DUMP_TRACE_TO_JSON(algorithm, V, run_)\
{CONCAT2(#run_, #algorithm), CONCAT4(run_evolution_,algorithm,_,V)}

namespace AGE1
{
	template<unsigned N>
	struct Problem
	{
		PerfTestFunctor<N> functor;
	};

	template<unsigned N>
	NDimensionPoint<N> get_random_point()
	{
		NDimensionPoint<N> point;
		srand(time(nullptr));
		
		for (unsigned i = 0; i < N; ++i)
		{
			Converter c;
			c.f = std::numeric_limits<float>::infinity();
			
			std::uint32_t r = std::numeric_limits<unsigned>::infinity();

			while (isnan<float>(c.f) || isinf<float>(c.f))
			{
				unsigned random[3] = { rand(), rand(), rand() };
				r = random[0] + (random[1] << 14ul) + (random[2] << 28ul);
				c.ul = r;
			}
			
			point.coordinates[i] = r;
		}

		return point;
	}
	
	template<unsigned N>
	ReturnWrapper<N> hillclimb_best_improve(Problem<N>& problem)
	{
		NDimensionPoint<N> best_solution;
		auto best_minimum = std::numeric_limits<float>::infinity();

		const unsigned T_MAX = 30;
		unsigned Temperature = 0;
		const auto start = std::chrono::system_clock::now();

		do
		{
			bool local = false;
			auto random_solution = get_random_point<N>();
			auto current_minimum = problem.functor(random_solution.coordinates);

			do
			{
				NDimensionPoint<N> neighbor_best;
				float neighbor_best_minimum = std::numeric_limits<float>::infinity();
				
				for (unsigned i = 0; i < N; ++i)
				{
					for (unsigned l = 0; l < L; ++l)
					{
						auto neighbor = random_solution;
						neighbor.coordinates[i][l].flip();

						//Safety check for bit modification
						Converter c;
						c.ul = neighbor.coordinates[i].to_ulong();
						if (isinf<float>(c.f) || isnan<float>(c.f))
							continue;

						auto neighbor_minimum = problem.functor(neighbor.coordinates);

						if (neighbor_minimum < neighbor_best_minimum)
						{
							neighbor_best = neighbor;
							neighbor_best_minimum = neighbor_minimum;
						}
					}
				}

				if (neighbor_best_minimum < current_minimum)
				{
					random_solution = neighbor_best;
					current_minimum = neighbor_best_minimum;
				}
				else
				{
					local = true;
				}

			} while (!local);

			if (current_minimum < best_minimum)
			{
				best_solution = random_solution;
				best_minimum = current_minimum;
			}

            Temperature += 1;
		} while (Temperature < T_MAX && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < 6 * N);

		
		return ReturnWrapper<N>{best_solution, best_minimum};
	}

	template<unsigned N>
	ReturnWrapper<N> hillclimb_first_improve(Problem<N>& problem)
	{
		NDimensionPoint<N> best_solution;
		auto best_minimum = std::numeric_limits<float>::infinity();

		const unsigned T_MAX = 30;
		unsigned Temperature = 0;
		const auto start = std::chrono::system_clock::now();

		do
		{
			bool local = false;
			auto random_solution = get_random_point<N>();
			auto current_minimum = problem.functor(random_solution.coordinates);

			do
			{
				NDimensionPoint<N> neighbor_best;
				float neighbor_best_minimum = std::numeric_limits<float>::infinity();

				bool found_min = false;

				unsigned random_array[N*L][2];
				for (unsigned i = 0; i < N; ++i)
				{
					for (unsigned l = 0; l < L; ++l)
					{
						random_array[i*L + l][0] = i;
						random_array[i*L + l][1] = l;
					}
				}

				std::shuffle(&random_array[0], &random_array[N*L - 1], std::mt19937(std::random_device()()));

				for (unsigned i = 0; i < N*L; ++i)
				{
					auto neighbor = random_solution;
					neighbor.coordinates[random_array[i][0]][random_array[i][1]].flip();

					//Safety check for bit modification
					Converter c;
					c.ul = neighbor.coordinates[i].to_ulong();
					if (isinf<float>(c.f) || isnan<float>(c.f))
						continue;

					auto neighbor_minimum = problem.functor(neighbor.coordinates);

					if (neighbor_minimum < neighbor_best_minimum)
					{
						neighbor_best = neighbor;
						neighbor_best_minimum = neighbor_minimum;
						found_min = true;
						break;
					}
				}
				
				if (found_min)
				{
					random_solution = neighbor_best;
					current_minimum = neighbor_best_minimum;
				}
				else
				{
					local = true;
				}

				if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() > 300)
					break;

			} while (!local);

			if (current_minimum < best_minimum)
			{
				best_solution = random_solution;
				best_minimum = current_minimum;
			}

			Temperature += 1;
		} while (Temperature < T_MAX && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < 2 * N);


		return ReturnWrapper<N>{best_solution, best_minimum};
	}

	template<unsigned N>
	ReturnWrapper<N> simulated_annealing(Problem<N>& problem)
	{
		srand(time(nullptr));
		
		NDimensionPoint<N> best_solution;
		auto best_minimum = std::numeric_limits<float>::infinity();

		double Temperature = 1;
		const auto term_time = std::chrono::system_clock::now();

		do
		{
			//Random init
			auto random_solution = get_random_point<N>();
			auto current_minimum = problem.functor(random_solution.coordinates);

			//Halting condition
			bool local = false;
			int repeats = 0;
			const unsigned MAX_REPEATS = 30;
			float last_minimum = -1;
			const auto halt_time = std::chrono::system_clock::now();
			
			do
			{
				if (current_minimum == last_minimum)
				{
					repeats++;
				}
				else
				{
					repeats = 0;
				}
				
				//Local init
				auto n_location = rand() % N;
				auto l_location = rand() % L;

				auto neighbor = random_solution;
				neighbor.coordinates[n_location][l_location].flip();

				//Safety check for bit modification
				Converter c;
				c.ul = neighbor.coordinates[n_location].to_ulong();
				if (isinf<float>(c.f) || isnan<float>(c.f))
				{
					repeats -= 1;
					continue;
				}

				auto neighbor_minimum = problem.functor(neighbor.coordinates);

				const auto random_sub = double((unsigned(rand())%32767) / double(32767));
				if ((neighbor_minimum < current_minimum) || (random_sub < double(exp(-abs(abs(neighbor_minimum) - abs(current_minimum)) / Temperature)) / 1.5))
				{
					random_solution = neighbor;
					current_minimum = neighbor_minimum;
				}
				//Halting cond
				else if (repeats > MAX_REPEATS || std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - halt_time).count() > 500)
				{
					local = true;
				}

			} while (!local);

			if (current_minimum < best_minimum)
			{
				best_solution = random_solution;
				best_minimum = current_minimum;
			}

			Temperature *= 0.9;
		} while (Temperature > 0.001 && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - term_time).count() < 3);//Terminating cond

		return ReturnWrapper<N>{best_solution, best_minimum};
	}

	template<unsigned N>
	std::vector<float> traceable_hillclimb_best_improve(Problem<N>& problem)
	{
		NDimensionPoint<N> best_solution;
		auto best_minimum = std::numeric_limits<float>::infinity();

		const unsigned T_MAX = 30;
		unsigned Temperature = 0;
		const auto start = std::chrono::system_clock::now();

		std::vector<float> run_evolution;

		do
		{
			bool local = false;
			auto random_solution = get_random_point<N>();
			auto current_minimum = problem.functor(random_solution.coordinates);

			do
			{
				NDimensionPoint<N> neighbor_best;
				float neighbor_best_minimum = std::numeric_limits<float>::infinity();

				for (unsigned i = 0; i < N; ++i)
				{
					for (unsigned l = 0; l < L; ++l)
					{
						auto neighbor = random_solution;
						neighbor.coordinates[i][l].flip();

						//Safety check for bit modification
						Converter c;
						c.ul = neighbor.coordinates[i].to_ulong();
						if (isinf<float>(c.f) || isnan<float>(c.f))
							continue;

						auto neighbor_minimum = problem.functor(neighbor.coordinates);

						if (neighbor_minimum < neighbor_best_minimum)
						{
							neighbor_best = neighbor;
							neighbor_best_minimum = neighbor_minimum;
						}
					}
				}

				if (neighbor_best_minimum < current_minimum)
				{
					random_solution = neighbor_best;
					current_minimum = neighbor_best_minimum;
					static unsigned entry = 0;
					entry++;
					if (entry % 10 == 0 && run_evolution.size() < 100)
						run_evolution.push_back(neighbor_best_minimum);
				}
				else
				{
					local = true;
				}

			} while (!local);

			if (current_minimum < best_minimum)
			{
				best_solution = random_solution;
				best_minimum = current_minimum;
			}

			Temperature += 1;
		} while (Temperature < T_MAX && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < 6 * N);


		return run_evolution;
	}

	template<unsigned N>
	std::vector<float> traceable_simulated_annealing(Problem<N>& problem)
	{
		srand(time(nullptr));

		NDimensionPoint<N> best_solution;
		auto best_minimum = std::numeric_limits<float>::infinity();

		double Temperature = 1;
		const auto term_time = std::chrono::system_clock::now();

		std::vector<float> run_evolution;

		do
		{
			//Random init
			auto random_solution = get_random_point<N>();
			auto current_minimum = problem.functor(random_solution.coordinates);

			//Halting condition
			bool local = false;
			int repeats = 0;
			const unsigned MAX_REPEATS = 30;
			float last_minimum = -1;
			const auto halt_time = std::chrono::system_clock::now();

			do
			{
				if (current_minimum == last_minimum)
				{
					repeats++;
				}
				else
				{
					repeats = 0;
				}

				//Local init
				auto n_location = rand() % N;
				auto l_location = rand() % L;

				auto neighbor = random_solution;
				neighbor.coordinates[n_location][l_location].flip();

				//Safety check for bit modification
				Converter c;
				c.ul = neighbor.coordinates[n_location].to_ulong();
				if (isinf<float>(c.f) || isnan<float>(c.f))
				{
					repeats -= 1;
					continue;
				}

				auto neighbor_minimum = problem.functor(neighbor.coordinates);

				const auto random_sub = double((unsigned(rand()) % 32767) / double(32767));
				if ((neighbor_minimum < current_minimum) || (random_sub < double(exp(-abs(abs(neighbor_minimum) - abs(current_minimum)) / Temperature)) / 1.5))
				{
					random_solution = neighbor;
					current_minimum = neighbor_minimum;
					static unsigned entry = 0;
					entry++;
					if (entry % 10 == 0 && run_evolution.size() < 100)
						run_evolution.push_back(neighbor_minimum);
				}
				//Halting cond
				else if (repeats > MAX_REPEATS || std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - halt_time).count() > 500)
				{
					local = true;
				}

			} while (!local);

			if (current_minimum < best_minimum)
			{
				best_solution = random_solution;
				best_minimum = current_minimum;
			}

			Temperature *= 0.9;
		} while (Temperature > 0.001 && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - term_time).count() < 3);//Terminating cond

		return run_evolution;
	}

	template<unsigned N>
	struct HeuristicRunWrapper
	{
		float coordinates[30][N];
		float func_values[30];
		unsigned deltas[30];
	};

	template<template <unsigned> class Function>
	struct AlgorythmAnalyzerAGE1
	{
		Function<2> instance;
		
		std::string create_output()
		{
			auto j = create_json();
			return std::string(j.dump());
		}

		nlohmann::json create_json()
		{
			REGISTER_DIMENSION(5);
			REGISTER_DIMENSION(10);
			REGISTER_DIMENSION(30);
			
			//HC - best
			printf("Started HC-Best on %s\n", instance.name.c_str());
			REGISTER_FUNCTION(hillclimb_best_improve, 5);
			REGISTER_FUNCTION(hillclimb_best_improve, 10);
			REGISTER_FUNCTION(hillclimb_best_improve, 30);
			printf("Ended HC-Best on %s\n", instance.name.c_str());

			//HC - first
			//printf("Started HC - first on %s\n", instance.name.c_str());
			//REGISTER_FUNCTION(hillclimb_first_improve, 2);
			//REGISTER_FUNCTION(hillclimb_first_improve, 5);
			//REGISTER_FUNCTION(hillclimb_first_improve, 30);
			//printf("Ended HC - first on %s\n", instance.name.c_str());

			//SA
			printf("Started SA on %s\n", instance.name.c_str());
			REGISTER_FUNCTION(simulated_annealing, 5);
			REGISTER_FUNCTION(simulated_annealing, 10);
			REGISTER_FUNCTION(simulated_annealing, 30);
			printf("Ended SA on %s\n", instance.name.c_str());

			TRACE_ALGORITHM(hillclimb_best_improve, 30);
			TRACE_ALGORITHM(simulated_annealing, 30);

			nlohmann::json j = {
				{"function-name", instance.name.c_str()},
				{"global_minimum", std::to_string(instance.global_minimum)},
				{"analysis",	nlohmann::json::array({
				DUMP_ANALYSIS_TO_JSON(hillclimb_best_improve, 5),
				DUMP_ANALYSIS_TO_JSON(hillclimb_best_improve, 10),
				DUMP_ANALYSIS_TO_JSON(hillclimb_best_improve, 30),
				//DUMP_ANALYSIS_TO_JSON(hillclimb_first_improve, 2),
				//DUMP_ANALYSIS_TO_JSON(hillclimb_first_improve, 5),
				//DUMP_ANALYSIS_TO_JSON(hillclimb_first_improve, 30),
				DUMP_ANALYSIS_TO_JSON(simulated_annealing, 5),
				DUMP_ANALYSIS_TO_JSON(simulated_annealing, 10),
				DUMP_ANALYSIS_TO_JSON(simulated_annealing, 30),
				DUMP_TRACE_TO_JSON(hillclimb_best_improve, 30, run_),
				DUMP_TRACE_TO_JSON(simulated_annealing, 30, run_)
				})
			} };

			return j;
		}

		void create_output_file()
		{
			auto j = create_json();
			std::string name("output_age1_");
			name += instance.name.c_str();
			name += ".json";
			std::ofstream out(name.c_str());
			out << j << std::endl;
		}
	};
}
