#pragma once
#include "FunctionsAGE1p.h"
#include "json.hpp"
#include <ctime>
#include <chrono>
#include <random>

#define TRACE_MODE

#ifndef CONCAT3 
#define CONCAT3(x, y, z) x##y##z
#endif
#ifndef CONCAT4
#define CONCAT4(x, y, z, w) x##y##z##w
#endif

#define REGISTER_DIMENSION_P(N)\
Function<N> CONCAT3(f, _dim_, N); \
PerfTestFunctor<N> CONCAT3(f, _perf_dim_, N) = {CONCAT3(f, _dim_, N)};\
MaximizationProblem<N> CONCAT3(problem, _dim_, N) ( CONCAT3(f, _perf_dim_, N)); \

#define REGISTER_FUNCTION_P(function, Ex, Man, N)\
HeuristicRunWrapper<N> CONCAT4(run_,function,_,N);\
for (unsigned repeat = 0; repeat < 30; ++repeat)\
{\
	auto begin = std::chrono::system_clock::now();\
\
	const auto res = function<Ex, Man, N>(CONCAT3(problem,_dim_,N));\
	CONCAT4(run_,function,_,N).func_values[repeat] = res.func_value;\
	for (unsigned i = 0; i < 2; ++i)\
	{\
		CONCAT4(run_,function,_,N).coordinates[repeat][i] = res.point.coordinates[i].to_float(); \
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

#ifndef DUMP_ANALYSIS_TO_JSON
#define DUMP_ANALYSIS_TO_JSON(function, N)\
{\
	{"algorythm-name",#function},\
	{"dimension", N},\
	{"results", CONCAT4(stringified_run_,function,_,N)},\
	{"deltas", CONCAT4(run_,function,_,N).deltas},\
	{"coordinates", CONCAT4(run_,function,_,N).coordinates}\
}\

#endif

namespace AGE1p
{
	template<unsigned N>
	struct Problem
	{
		PerfTestFunctor<N> functor;
		float starting_value = 0;

		Problem(PerfTestFunctor<N> functor, float start)
			:functor(functor),starting_value(start)
		{}

		virtual bool is_new_optimal(const float old_var,const float new_var) { return false; };
	};

	template<unsigned N>
	struct MaximizationProblem : Problem<N>
	{
		MaximizationProblem(PerfTestFunctor<N> functor)
			:Problem<N>(functor, -std::numeric_limits<float>::infinity())
		{}
		
		bool is_new_optimal(const float old_var, const float new_var) override
		{
			return new_var > old_var;
		}
	};

	template<unsigned N>
	struct MinimizationProblem : Problem<N>
	{
		MinimizationProblem(PerfTestFunctor<N> functor)
			:Problem<N>(functor, std::numeric_limits<float>::infinity())
		{}
		
		bool is_new_optimal(const float old_var, const float new_var) override
		{
			return new_var < old_var;
		}
	};

	template<unsigned Ex, unsigned Man, unsigned N>
	NDimensionPoint<Ex, Man, N> get_random_point()
	{
		NDimensionPoint<Ex, Man, N> point;
		srand(std::chrono::system_clock::now().time_since_epoch().count());

		for (unsigned i = 0; i < N; ++i)
		{
			unsigned random[3] = { rand(), rand(), rand() };
			unsigned r = random[0] + (random[1] << 14ul) + (random[2] << 28ul);

			point.coordinates[i].value = r;
		}

		return point;
	}

	template<unsigned Ex, unsigned Man, unsigned N>
	ReturnWrapper<Ex, Man, N> hillclimb_best_improve(Problem<N>& problem)
	{
		NDimensionPoint<Ex, Man, N> best_solution;
		auto best_value = problem.starting_value;

		const unsigned T_MAX = 30;
		unsigned Temperature = 0;
		const auto start = std::chrono::system_clock::now();

		do
		{
			bool local = false;
			auto random_solution = get_random_point<Ex, Man, N>();

			float coordinates_r[N];
			for (unsigned x = 0; x < N; ++x)
			{
				coordinates_r[x] = random_solution.coordinates[x].to_float();
			}
			auto current_best_value = problem.functor(coordinates_r);

#ifdef TRACE_MODE
			printf(">>>Chosen random solution:");
			for (unsigned x = 0; x < N; ++x)
			{
				printf(" %s:%f ", random_solution.coordinates[x].value.to_string().c_str(), coordinates_r[x]);
			}
			printf(" Value at %f\n", current_best_value);

			int count = 0;
#endif

			do
			{
				NDimensionPoint<Ex, Man, N> neighbor_best;
				float neighbor_best_value = problem.starting_value;

#ifdef TRACE_MODE
				count++;
#endif

				for (unsigned i = 0; i < N; ++i)
				{
					for (unsigned l = 0; l < Ex + Man; ++l)
					{
						auto neighbor = random_solution;
						neighbor.coordinates[i].value[l].flip();

						float coordinates[N];
						for (unsigned x = 0; x< N; ++x)
						{
							coordinates[x] = neighbor.coordinates[x].to_float();
						}

						auto neighbor_value = problem.functor(coordinates);

#ifdef TRACE_MODE
						for (int tab=0;tab<count;++tab)
						{
							printf("\t");
						}
						
						printf("Neighbor solution:");
						for (unsigned x = 0; x < N; ++x)
						{
							printf(" %s:%f ", neighbor.coordinates[x].value.to_string().c_str(), coordinates[x]);
						}
						printf(" Value at %f\n", neighbor_value);
#endif

						if (problem.is_new_optimal(neighbor_best_value, neighbor_value))
						{
							neighbor_best = neighbor;
							neighbor_best_value = neighbor_value;
						}
					}
				}

				if (problem.is_new_optimal(current_best_value, neighbor_best_value))
				{
					random_solution = neighbor_best;
					current_best_value = neighbor_best_value;
				}
				else
				{
					local = true;
				}

#ifdef TRACE_MODE
				for (unsigned x = 0; x < N; ++x)
				{
					coordinates_r[x] = random_solution.coordinates[x].to_float();
				}
				printf(">>>Local value at %f", current_best_value);
				for (unsigned x = 0; x < N; ++x)
				{
					printf(" %s:%f ", random_solution.coordinates[x].value.to_string().c_str(), coordinates_r[x]);
				}
				printf("\n");
#endif

			} while (!local);

			if (problem.is_new_optimal(best_value, current_best_value))
			{
				best_solution = random_solution;
				best_value = current_best_value;
			}

			Temperature += 1;
		} while (Temperature < T_MAX && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < 6 * N);

#ifdef TRACE_MODE
		float coordinates_r[N];
		for (unsigned x = 0; x < N; ++x)
		{
			coordinates_r[x] = best_solution.coordinates[x].to_float();
		}
		printf(">>>Final value at %f Final solution", best_value);
		for (unsigned x = 0; x < N; ++x)
		{
			printf(" %s:%f \n\n", best_solution.coordinates[x].value.to_string().c_str(), coordinates_r[x]);
		}
#endif


		return ReturnWrapper<Ex, Man, N>{best_solution, best_value};
	}

	template<unsigned Ex, unsigned Man, unsigned N>
	ReturnWrapper<Ex, Man, N> hillclimb_first_improve(Problem<N>& problem)
	{
		NDimensionPoint<Ex, Man, N> best_solution;
		auto best_minimum = problem.starting_value;

		const unsigned T_MAX = 30;
		unsigned Temperature = 0;
		const auto start = std::chrono::system_clock::now();

		do
		{
			bool local = false;
			auto random_solution = get_random_point<Ex, Man, N>();

			float coordinates_r[N];
			for (unsigned x = 0; x < N; ++x)
			{
				coordinates_r[x] = random_solution.coordinates[x].to_float();
			}
			auto current_minimum = problem.functor(coordinates_r);

			do
			{
				NDimensionPoint<Ex, Man, N> neighbor_best;
				float neighbor_best_minimum = problem.starting_value;

				bool found_min = false;

				unsigned random_array[N*(Ex + Man)][2];
				for (unsigned i = 0; i < N; ++i)
				{
					for (unsigned l = 0; l < Ex + Man; ++l)
					{
						random_array[i*(Ex+ Man) + l][0] = i;
						random_array[i*(Ex + Man) + l][1] = l;
					}
				}

				std::shuffle(&random_array[0], &random_array[N*(Ex + Man) - 1], std::mt19937(std::random_device()()));

				for (unsigned i = 0; i < N*(Ex + Man); ++i)
				{
					auto neighbor = random_solution;
					neighbor.coordinates[random_array[i][0]].value[random_array[i][1]].flip();

					float coordinates[N];
					for (unsigned x = 0; x < N; ++x)
					{
						coordinates[x] = neighbor.coordinates[x].to_float();
					}

					auto neighbor_minimum = problem.functor(coordinates);

					if (problem.is_new_optimal(neighbor_best_minimum, neighbor_minimum))
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

				//if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() > 300)
				//	break;

			} while (!local);

			if (problem.is_new_optimal(best_minimum, current_minimum))
			{
				best_solution = random_solution;
				best_minimum = current_minimum;
			}

			Temperature += 1;
		} while (Temperature < T_MAX && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < 2 * N);


		return ReturnWrapper<Ex, Man, N>{best_solution, best_minimum};
	}

	template<unsigned Ex, unsigned Man, unsigned N>
	ReturnWrapper<Ex, Man, N> simulated_annealing(Problem<N>& problem)
	{
		srand(time(nullptr));

		NDimensionPoint<Ex, Man, N> best_solution;
		auto best_minimum = problem.starting_value;

		double Temperature = 1;
		const auto term_time = std::chrono::system_clock::now();

		do
		{
			//Random init
			auto random_solution = get_random_point<Ex,Man,N>();

			float coordinates_r[N];
			for (unsigned x = 0; x < N; ++x)
			{
				coordinates_r[x] = random_solution.coordinates[x].to_float();
			}
			auto current_minimum = problem.functor(coordinates_r);

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
				auto l_location = rand() % (Ex + Man);

				auto neighbor = random_solution;
				neighbor.coordinates[n_location].value[l_location].flip();

				float coordinates[N];
				for (unsigned x = 0; x < N; ++x)
				{
					coordinates[x] = neighbor.coordinates[x].to_float();
				}

				auto neighbor_minimum = problem.functor(coordinates);

				const auto random_sub = double((unsigned(rand()) % 32767) / double(32767));
				if ((problem.is_new_optimal(current_minimum, neighbor_minimum)) || (random_sub < double(exp(-abs(abs(neighbor_minimum) - abs(current_minimum)) / Temperature)) / 1.5))
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

			if (problem.is_new_optimal(best_minimum, current_minimum))
			{
				best_solution = random_solution;
				best_minimum = current_minimum;
			}

			Temperature *= 0.9;
		} while (Temperature > 0.001 && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - term_time).count() < 3);//Terminating cond

		return ReturnWrapper<Ex, Man, N>{best_solution, best_minimum};
	}

	template<unsigned N>
	struct HeuristicRunWrapper
	{
		float coordinates[30][N];
		float func_values[30];
		unsigned deltas[30];
	};

	template<template <unsigned> class Function>
	struct AlgorythmAnalyzerAGE1p
	{
		Function<2> instance;

		std::string create_output()
		{
			auto j = create_json();
			return std::string(j.dump());
		}

		nlohmann::json create_json()
		{
			REGISTER_DIMENSION_P(2);
			REGISTER_DIMENSION_P(5);
			REGISTER_DIMENSION_P(30);

			//HC - best
			printf("Started HC-Best on %s\n", instance.name.c_str());
			REGISTER_FUNCTION_P(hillclimb_best_improve, 5, 0, 2);
			REGISTER_FUNCTION_P(hillclimb_best_improve, 5, 0, 5);
			REGISTER_FUNCTION_P(hillclimb_best_improve, 5, 0, 30);
			printf("Ended HC-Best on %s\n", instance.name.c_str());

			//HC - first
			printf("Started HC - first on %s\n", instance.name.c_str());
			REGISTER_FUNCTION_P(hillclimb_first_improve, 5, 0, 2);
			REGISTER_FUNCTION_P(hillclimb_first_improve, 5, 0, 5);
			REGISTER_FUNCTION_P(hillclimb_first_improve, 5, 0, 30);
			printf("Ended HC - first on %s\n", instance.name.c_str());

			/*printf("Started SA on %s\n", instance.name.c_str());
			REGISTER_FUNCTION_P(simulated_annealing, 5, 0, 2);
			REGISTER_FUNCTION_P(simulated_annealing, 5, 0, 5);
			REGISTER_FUNCTION_P(simulated_annealing, 5, 0, 30);
			printf("Ended SA on %s\n", instance.name.c_str());*/

			nlohmann::json j = {
				{"function-name", instance.name.c_str()},
				{"global_minimum", std::to_string(instance.global_minimum)},
				{"analysis",	nlohmann::json::array({
				DUMP_ANALYSIS_TO_JSON(hillclimb_best_improve, 2),
				DUMP_ANALYSIS_TO_JSON(hillclimb_best_improve, 5),
				DUMP_ANALYSIS_TO_JSON(hillclimb_best_improve, 30),
				DUMP_ANALYSIS_TO_JSON(hillclimb_first_improve, 2),
				DUMP_ANALYSIS_TO_JSON(hillclimb_first_improve, 5),
				DUMP_ANALYSIS_TO_JSON(hillclimb_first_improve, 30),
				/*DUMP_ANALYSIS_TO_JSON(simulated_annealing, 2),
				DUMP_ANALYSIS_TO_JSON(simulated_annealing, 5),
				DUMP_ANALYSIS_TO_JSON(simulated_annealing, 30),*/
				})
			} };

			return j;
		}

		void create_output_file()
		{
			auto j = create_json();
			std::string name("output_");
			name += instance.name.c_str();
			name += ".json";
			std::ofstream out(name.c_str());
			out << j << std::endl;
		}
	};

	template<template <unsigned> class Function>
	struct AlgorythmTracerAGE1p
	{
		void trace_hc()
		{
			Function<1> f_dim_1;
			PerfTestFunctor<1> f_perf_dim_1 = { f_dim_1 };
			MaximizationProblem<1> problem_dim_1 = f_perf_dim_1;
			
			ReturnWrapper<5, 0, 1> res = hillclimb_best_improve<5, 0, 1>(problem_dim_1);
			int x = 0;
		}
	};
}
