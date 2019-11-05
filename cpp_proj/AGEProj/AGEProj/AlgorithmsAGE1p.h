#pragma once
#include "FunctionsAGE1p.h"
#include "json.hpp"
#include <ctime>
#include <chrono>

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

#define REGISTER_FUNCTION_P(function, Sgn, Ex, Man, N)\
HeuristicRunWrapper<N> CONCAT4(run_,function,_,N);\
for (unsigned repeat = 0; repeat < 30; ++repeat)\
{\
	auto begin = std::chrono::system_clock::now();\
\
	const auto res = function<Sgn, Ex, Man, N>(CONCAT3(problem,_dim_,N));\
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

	template<bool Sgn, unsigned Ex, unsigned Man, unsigned N>
	struct ReturnWrapper
	{
		static_assert(Sgn + Ex + Man > 0, "At least one bit is required");
		static_assert(N > 0, "At least one dimension is required");
		NDimensionPoint<Sgn, Ex, Man, N> point;
		float func_value;
	};

	template<bool Sgn, unsigned Ex, unsigned Man, unsigned N>
	NDimensionPoint<Sgn, Ex, Man, N> get_random_point()
	{
		NDimensionPoint<Sgn, Ex, Man, N> point;
		srand(std::chrono::system_clock::now().time_since_epoch().count());

		for (unsigned i = 0; i < N; ++i)
		{
			unsigned random[3] = { rand(), rand(), rand() };
			unsigned r = random[0] + (random[1] << 14ul) + (random[2] << 28ul);

			point.coordinates[i].value = r;
		}

		return point;
	}

	template<bool Sgn, unsigned Ex, unsigned Man, unsigned N>
	ReturnWrapper<Sgn, Ex, Man, N> hillclimb_best_improve(Problem<N>& problem)
	{
		NDimensionPoint<Sgn, Ex, Man, N> best_solution;
		auto best_value = problem.starting_value;

		const unsigned T_MAX = 30;
		unsigned Temperature = 0;
		const auto start = std::chrono::system_clock::now();

		do
		{
			bool local = false;
			auto random_solution = get_random_point<Sgn, Ex, Man, N>();

			float coordinates_r[N];
			for (unsigned x = 0; x < N; ++x)
			{
				coordinates_r[x] = random_solution.coordinates[x].to_float();
			}
			auto current_best_value = problem.functor(coordinates_r);

			do
			{
				NDimensionPoint<Sgn, Ex, Man, N> neighbor_best;
				float neighbor_best_value = problem.starting_value;

				for (unsigned i = 0; i < N; ++i)
				{
					for (unsigned l = 0; l < Sgn + Ex + Man; ++l)
					{
						auto neighbor = random_solution;
						neighbor.coordinates[i].value[l].flip();

						float coordinates[N];
						for (unsigned x = 0; x< N; ++x)
						{
							coordinates[x] = neighbor.coordinates[x].to_float();
						}

						auto neighbor_value = problem.functor(coordinates);

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

			} while (!local);

			if (problem.is_new_optimal(best_value, current_best_value))
			{
				best_solution = random_solution;
				best_value = current_best_value;
			}

			Temperature += 1;
		} while (Temperature < T_MAX && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < 6 * N);

		return ReturnWrapper<Sgn, Ex, Man, N>{best_solution, best_value};
	}

	template<bool Sgn, unsigned Ex, unsigned Man, unsigned N>
	ReturnWrapper<Sgn, Ex, Man, N> hillclimb_first_improve(Problem<N>& problem)
	{
		NDimensionPoint<Sgn, Ex, Man, N> best_solution;
		auto best_minimum = problem.starting_value;

		const unsigned T_MAX = 30;
		unsigned Temperature = 0;
		const auto start = std::chrono::system_clock::now();

		do
		{
			bool local = false;
			auto random_solution = get_random_point<Sgn, Ex, Man, N>();

			float coordinates_r[N];
			for (unsigned x = 0; x < N; ++x)
			{
				coordinates_r[x] = random_solution.coordinates[x].to_float();
			}
			auto current_minimum = problem.functor(coordinates_r);

			do
			{
				NDimensionPoint<Sgn, Ex, Man, N> neighbor_best;
				float neighbor_best_minimum = problem.starting_value;

				bool found_min = false;
				unsigned random_array[N*(Ex + Man)][2];
				for (unsigned i = 0; i < N; ++i)
				{
					for (unsigned l = 0; l < Ex + Man; ++l)
					{
						random_array[i*(Ex + Man) + l][0] = i;
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


		return ReturnWrapper<Sgn, Ex, Man, N>{best_solution, best_minimum};
	}

	template<bool Sgn, unsigned Ex, unsigned Man, unsigned N>
	ReturnWrapper<Sgn, Ex, Man, N> simulated_annealing(Problem<N>& problem)
	{
		srand(time(nullptr));

		NDimensionPoint<Sgn, Ex, Man, N> best_solution;
		auto best_minimum = problem.starting_value;

		double Temperature = 1;
		const auto term_time = std::chrono::system_clock::now();

		do
		{
			//Random init
			auto random_solution = get_random_point<Sgn,Ex,Man,N>();

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
				auto l_location = rand() % (Sgn + Ex + Man);

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

		return ReturnWrapper<Sgn, Ex, Man, N>{best_solution, best_minimum};
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
			REGISTER_FUNCTION_P(hillclimb_best_improve, false, 5, 0, 2);
			REGISTER_FUNCTION_P(hillclimb_best_improve, false, 5, 0, 5);
			REGISTER_FUNCTION_P(hillclimb_best_improve, false, 5, 0, 30);
			printf("Ended HC-Best on %s\n", instance.name.c_str());

			//HC - first
			printf("Started HC - first on %s\n", instance.name.c_str());
			REGISTER_FUNCTION_P(hillclimb_first_improve, false, 5, 0, 2);
			REGISTER_FUNCTION_P(hillclimb_first_improve, false, 5, 0, 5);
			REGISTER_FUNCTION_P(hillclimb_first_improve, false, 5, 0, 30);
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
			name += instance.name;
			name += ".json";
			std::ofstream out(name.c_str());
			out << j << std::endl;
		}
	};

	template<bool Sgn, unsigned Ex, unsigned Man, unsigned N>
	void compute_all_candidates(NDimensionPoint <Sgn, Ex, Man, N> current, NDimensionPoint < Sgn, Ex, Man, N>* points, unsigned level)
	{
		static unsigned current_index = 0;
		if (level == (Sgn + Ex + Man)*N)
		{
			points[current_index] = current;
			current_index++;
		}
		else
		{
			current.coordinates[level / (Sgn + Ex + Man)].value[level % (Sgn + Ex + Man)] = 0;
			compute_all_candidates<Sgn, Ex, Man, N>(current, points, level + 1);
			current.coordinates[level / (Sgn + Ex + Man)].value[level % (Sgn + Ex + Man)] = 1;
			compute_all_candidates<Sgn, Ex, Man, N>(current, points, level + 1);
		}
	}

	template<bool Sgn, unsigned Ex, unsigned Man, unsigned N, unsigned ID>
	NDimensionPoint < Sgn, Ex, Man, N> get_next_candidate()
	{
		static NDimensionPoint < Sgn, Ex, Man, N> points[1<<(Sgn + Ex + Man)*N];

		static unsigned current_p = 0;
		
		static bool initialized = false;
		if (initialized)
		{
			if (current_p >= (1 << (Sgn + Ex + Man)*N))
			{
				return points[0];
			}
			current_p++;
			return points[current_p];
		}
		else
		{
			initialized = true;
			NDimensionPoint < Sgn, Ex, Man, N> point;
			compute_all_candidates(point, points, 0);

			return points[0];
		}
	}

	template<bool Sgn, unsigned Ex, unsigned Man, unsigned N>
	nlohmann::json hillclimb_best_improve_tracer(Problem<N>& problem)
	{
		nlohmann::json j;

		for (unsigned idx = 0; idx < (1 << (Sgn + Ex + Man)*N); ++idx)
		{
			bool is_local = false;
			auto root_candidate = get_next_candidate<Sgn, Ex, Man, N, 0>();

			std::string coordinates_r_s[N];
			for (unsigned x = 0; x < N; ++x)
			{
				coordinates_r_s[x] = root_candidate.coordinates[x].value.to_string();
			}
			float coordinates_r_f[N];
			for (unsigned x = 0; x < N; ++x)
			{
				coordinates_r_f[x] = root_candidate.coordinates[x].to_float();
			}
			auto root_value = problem.functor(coordinates_r_f);

			std::string root_name;
			for (unsigned i = 0; i < N; ++i)
			{
				root_name += coordinates_r_s[i];
			}

			//Search for neighbors
			NDimensionPoint<Sgn, Ex, Man, N> neighbor_best;
			float neighbor_best_value = problem.starting_value;

			for (unsigned i = 0; i < N; ++i)
			{
				for (unsigned l = 0; l < Sgn + Ex + Man; ++l)
				{
					auto neighbor = root_candidate;
					neighbor.coordinates[i].value[l].flip();

					float coordinates[N];
					for (unsigned x = 0; x < N; ++x)
					{
						coordinates[x] = neighbor.coordinates[x].to_float();
					}

					auto neighbor_value = problem.functor(coordinates);

					if (problem.is_new_optimal(neighbor_best_value, neighbor_value))
					{
						neighbor_best = neighbor;
						neighbor_best_value = neighbor_value;
					}
				}
			}

			if (!problem.is_new_optimal(root_value, neighbor_best_value))
			{
				is_local = true;
			}

			float coordinates_bn_f[N];
			for (unsigned x = 0; x < N; ++x)
			{
				coordinates_bn_f[x] = neighbor_best.coordinates[x].to_float();
			}
			std::string coordinates_bn_s[N];
			for (unsigned x = 0; x < N; ++x)
			{
				coordinates_bn_s[x] = neighbor_best.coordinates[x].value.to_string();
			}

			std::string best_neighbor_name;
			for (unsigned i = 0; i < N; ++i)
			{
				best_neighbor_name += coordinates_bn_s[i];
			}

			j["improvements"][root_name] = {
					{"coordinates_float", *reinterpret_cast<std::array<float, N>*>(&coordinates_r_f)},
					{"coordinates_string", *reinterpret_cast<std::array<std::string, N>*>(&coordinates_r_s)},
					{"value", root_value}
			};

			if (!is_local) {
				j["improvements"][root_name]["best_neighbor_coordinates_float"] = *reinterpret_cast<std::array<float, N>*>(&coordinates_bn_f);
				j["improvements"][root_name]["best_neighbor_coordinates_string"] = *reinterpret_cast<std::array<std::string, N>*>(&coordinates_bn_s);
				j["improvements"][root_name]["best_neighbor_value"] = neighbor_best_value;
			}
			else
			{
				j["improvements"][root_name]["best_neighbor_coordinates_float"] = nlohmann::json::array();
				j["improvements"][root_name]["best_neighbor_coordinates_string"] = nlohmann::json::array();
				j["improvements"][root_name]["best_neighbor_value"] = root_value;
			}
		}

		return j;
	}

	template<template <unsigned> class Function>
	struct AlgorythmTracerAGE1p
	{
		void trace_hc()
		{
			Function<1> f_dim_1;
			PerfTestFunctor<1> f_perf_dim_1 = { f_dim_1 };
			MaximizationProblem<1> problem_dim_1 = f_perf_dim_1;
			
			nlohmann::json res = hillclimb_best_improve_tracer<false, 5, 0, 1>(problem_dim_1);
			std::string name("output_trace_hc.json");
			std::ofstream out(name.c_str());
			out << res << std::endl;
		}
	};
}
