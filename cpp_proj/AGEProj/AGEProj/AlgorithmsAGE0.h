#pragma once
#include "FunctionsAGE0.h"
#include <queue>
#include <ctime>
#include <string>
#include "json.hpp"
#include <chrono>
#include <fstream>

namespace AGE0 {

	template<unsigned N>
	struct Problem
	{
		PerfTestFunctor<N> functor;
		float precision;
	};

	template<unsigned N>
	struct CandidateSolutionNode
	{
		Interval interval[N];
		float func_value;
		unsigned depth;

		CandidateSolutionNode<N>() = default;
		CandidateSolutionNode<N>(Interval& init)
		{
			for (unsigned i = 0; i < N; ++i)
			{
				interval[i] = init;
			}

			func_value = std::numeric_limits<float>::infinity();
			depth = 0;
		}
	};

	template<unsigned N>
	float lower_bound_function(PerfTestFunctor<N>& function, CandidateSolutionNode<N>& node)
	{
		float point[N];

		for (unsigned i = 0; i < N; ++i)
		{
			point[i] = (node.interval[i].left + node.interval[i].right) / 2;
		}

		return function(point);
	}

	template<unsigned N>
	struct ReturnWrapper
	{
		NDimensionPoint<N> point;
		float func_value;
	};

	//DAGO: Deterministic Algorithm for Global Optimization 
	template<unsigned N>
	ReturnWrapper<N> dago_algorithm(Problem<N>& problem)
	{
		float problem_lower_bound = std::numeric_limits<float>::infinity();

		std::queue<CandidateSolutionNode<N>> candidate_queue;
		CandidateSolutionNode<N> root(problem.functor.function.search_domain);
		candidate_queue.push(root);

		CandidateSolutionNode<N> best_solution_node;

		auto max_depth = unsigned(log2f(abs(problem.functor.function.search_domain.left -
			problem.functor.function.search_domain.right) / problem.precision));

		while (!candidate_queue.empty()) {
			CandidateSolutionNode<N> candidate_solution = candidate_queue.front();
			candidate_queue.pop();

			if (candidate_solution.depth >= max_depth)//leaf node
			{
				if (candidate_solution.func_value <= problem_lower_bound)
				{
					problem_lower_bound = candidate_solution.func_value;
					best_solution_node = candidate_solution;
				}
			}
			else
			{
				//left
				CandidateSolutionNode<N> left;
				for (unsigned i = 0; i < N; ++i)
				{
					left.interval[i] = Interval{
						candidate_solution.interval[i].left,
						(candidate_solution.interval[i].left + candidate_solution.interval[i].right) / 2
					};
				}

				float l_value = lower_bound_function(problem.functor, left);
				if (l_value < problem_lower_bound)
				{
					left.func_value = l_value;
					left.depth = candidate_solution.depth + 1;
					candidate_queue.push(left);
				}

				//right
				CandidateSolutionNode<N> right;
				for (unsigned i = 0; i < N; ++i)
				{
					right.interval[i] = Interval{
						(candidate_solution.interval[i].left + candidate_solution.interval[i].right) / 2,
						candidate_solution.interval[i].right,
					};
				}
				float r_value = lower_bound_function(problem.functor, right);
				if (r_value < problem_lower_bound)
				{
					right.func_value = r_value;
					right.depth = candidate_solution.depth + 1;
					candidate_queue.push(right);
				}
			}
		}

		//best_solution_node
		NDimensionPoint<N> point;
		for (unsigned i = 0; i < N; ++i)
		{
			point.coordinates[i] = (best_solution_node.interval[i].left + best_solution_node.interval[i].right) / 2;
		}

		return { point, problem_lower_bound };
	}

	//HAGO: Heuristic Algorithm for Global Optimization
	template<unsigned N>
	ReturnWrapper<N> hago_algorithm(Problem<N>& problem)
	{
		auto start = std::chrono::system_clock::now();

		Interval domain = problem.functor.function.search_domain;
		Interval point_intervals[N];
		for (unsigned i = 0; i < N; ++i)
		{
			point_intervals[i] = domain;
		}

		srand(time(nullptr));

		NDimensionPoint<N> minimum_point;
		for (unsigned i = 0; i < N; ++i)
		{
			minimum_point.coordinates[i] = domain.left + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (domain.right - domain.left)));
		}

		auto problem_minimum = std::numeric_limits<float>::infinity();

		const unsigned T_MAX = 1000000;
		unsigned T = 0;

		do
		{
			bool local = false;
			auto random_solution = get_random_point<N>(point_intervals);

			do
			{
				//break if too much time has passed
				auto end = std::chrono::system_clock::now();
				const auto run_time = std::chrono::duration_cast<std::chrono::minutes>(end - start).count();
				if (run_time >= 2)
					goto ret;//this is a mistake, i know

				bool found_improvement = false;
				unsigned i = 0;
				for (; i < N; ++i)
				{
					auto new_right_point = random_solution;
					new_right_point.coordinates[i] -= problem.precision;
					auto new_left_point = random_solution;
					new_left_point.coordinates[i] += problem.precision;

					auto value = problem.functor(new_right_point.coordinates);
					if (value < problem_minimum)
					{
						minimum_point = new_right_point;
						random_solution = new_right_point;
						problem_minimum = value;
						found_improvement = true;
					}

					value = problem.functor(new_left_point.coordinates);
					if (value < problem_minimum)
					{
						minimum_point = new_left_point;
						random_solution = new_left_point;
						problem_minimum = value;
						found_improvement = true;
					}

					if (found_improvement)
						break;
				}
				if (i == N)//a better solution does not exist
				{
					local = true;
				}

			} while (!local);

			T += 1;
		} while (T < T_MAX);
	ret:
		return ReturnWrapper<N>{minimum_point, problem_minimum};
	}

	template<unsigned N>
	struct HeuristicRunWrapper
	{
		float coordinates[30][N];
		float func_values[30];
		unsigned deltas[30];
	};

	template<template <unsigned> class Function>
	struct AlgorythmAnalyzerAGE0
	{
		Function<2> f1;
		Function<5> f2;
		Function<20> f3;

		std::string create_output()
		{
			auto j = create_json();
			return std::string(j.dump());
		}

		nlohmann::json create_json()
		{
			Problem<2> problem1{ {f1}, 0.01f };
			Problem<5> problem2{ {f2}, 0.01f };
			Problem<20> problem3{ {f3}, 0.01f };

			//DAGO - deterministic
			printf("Started D.A.G.O. on %s\n", problem1.functor.function.name.c_str());
			auto start = std::chrono::system_clock::now();
			auto det_r1 = dago_algorithm<2>(problem1);
			auto end = std::chrono::system_clock::now();
			auto delta1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

			start = std::chrono::system_clock::now();
			auto det_r2 = dago_algorithm<5>(problem2);
			end = std::chrono::system_clock::now();
			auto delta2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

			start = std::chrono::system_clock::now();
			auto det_r3 = dago_algorithm<20>(problem3);
			end = std::chrono::system_clock::now();
			auto delta3 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			printf("Ended D.A.G.O. on %s\n", problem1.functor.function.name.c_str());

			//HAGO - heuristic
			printf("Started H.A.G.O. on %s\n", problem1.functor.function.name.c_str());
			HeuristicRunWrapper<2> run1;
			for (unsigned repeat = 0; repeat < 30; ++repeat)
			{
				auto begin = std::chrono::system_clock::now();

				const auto res_1 = hago_algorithm<2>(problem1);
				run1.func_values[repeat] = res_1.func_value;
				for (unsigned i = 0; i < 2; ++i)
				{
					run1.coordinates[repeat][i] = res_1.point.coordinates[i];
				}

				auto finish = std::chrono::system_clock::now();
				run1.deltas[repeat] = std::chrono::duration_cast<std::chrono::microseconds>(finish - begin).count();
			}
			std::string stringified_run1_res[30];
			for (unsigned repeat = 0; repeat < 30; ++repeat)
			{
				stringified_run1_res[repeat] = std::to_string(run1.func_values[repeat]);
			}

			HeuristicRunWrapper<5> run2;
			for (unsigned repeat = 0; repeat < 30; ++repeat)
			{
				auto begin = std::chrono::system_clock::now();

				const auto res_2 = hago_algorithm<5>(problem2);
				run2.func_values[repeat] = res_2.func_value;
				for (unsigned i = 0; i < 5; ++i)
				{
					run2.coordinates[repeat][i] = res_2.point.coordinates[i];
				}

				auto finish = std::chrono::system_clock::now();
				run2.deltas[repeat] = std::chrono::duration_cast<std::chrono::microseconds>(finish - begin).count();
			}
			std::string stringified_run2_res[30];
			for (unsigned repeat = 0; repeat < 30; ++repeat)
			{
				stringified_run2_res[repeat] = std::to_string(run2.func_values[repeat]);
			}

			HeuristicRunWrapper<20> run3;
			for (unsigned repeat = 0; repeat < 30; ++repeat)
			{
				auto begin = std::chrono::system_clock::now();

				const auto res_3 = hago_algorithm<20>(problem3);
				run3.func_values[repeat] = res_3.func_value;
				for (unsigned i = 0; i < 20; ++i)
				{
					run3.coordinates[repeat][i] = res_3.point.coordinates[i];
				}

				auto finish = std::chrono::system_clock::now();
				run3.deltas[repeat] = std::chrono::duration_cast<std::chrono::microseconds>(finish - begin).count();
			}
			std::string stringified_run3_res[30];
			for (unsigned repeat = 0; repeat < 30; ++repeat)
			{
				stringified_run3_res[repeat] = std::to_string(run3.func_values[repeat]);
			}
			printf("Ended H.A.G.O. on %s\n", problem1.functor.function.name.c_str());

			nlohmann::json j = {
				{"function-name", f1.name.c_str()},
				{"global_minimum", std::to_string(f1.global_minimum)},
				{"analysis",	nlohmann::json::array({
				{
					{"dimension", 2},
					{"deterministic_results", {
						{"result", std::to_string(det_r1.func_value)},
						{"coordinates", nlohmann::json(det_r1.point.coordinates)},
						{"runtime", delta1}
					}},
					{"heuristic_results", {
						{"results", stringified_run1_res},
						{"deltas", run1.deltas},
						{"coordinates", run1.coordinates}
					}}
				},
				{
					{"dimension", 5},
					{"deterministic_results", {
						{"result", std::to_string(det_r2.func_value)},
						{"coordinates", nlohmann::json(det_r2.point.coordinates)},
						{"runtime", delta2}
					}},
					{"heuristic_results", {
						{"results", stringified_run2_res},
						{"deltas", run2.deltas},
						{"coordinates", run2.coordinates}
					}}
				},
				{
					{"dimension", 20},
					{"deterministic_results", {
						{"result", std::to_string(det_r3.func_value)},
						{"coordinates", nlohmann::json(det_r3.point.coordinates)},
						{"runtime", delta3}
					}},
					{"heuristic_results", {
						{"results", stringified_run3_res},
						{"deltas", run3.deltas},
						{"coordinates", run3.coordinates}
					}}
				},
			})
			} };

			return j;
		}

		void create_output_file()
		{
			auto j = create_json();
			std::string name("output_");
			name += f1.name.c_str();
			name += ".json";
			std::ofstream out(name.c_str());
			out << j << std::endl;
		}
	};

}