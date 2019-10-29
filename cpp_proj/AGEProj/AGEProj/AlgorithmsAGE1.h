#pragma once
#include "FunctionsAGE1.h"
#include "json.hpp"
#include <ctime>
#include <chrono>
#include <random>

namespace AGE1
{
	template<unsigned N>
	struct Problem
	{
		PerfTestFunctor<N> functor;
		float precision;
	};

	template<unsigned N>
	NDimensionPoint<N> get_random_point()
	{
		NDimensionPoint<N> point;
		srand(std::chrono::system_clock::now().time_since_epoch().count());
		
		for (unsigned i = 0; i < N; ++i)
		{
			unsigned random[3] = { rand(), rand(), rand()};
			std::uint32_t r = random[0] + (random[1] << 14ul) + (random[2] << 28ul);
			
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
		} while (Temperature < T_MAX);

		
		return ReturnWrapper<N>{best_solution, best_minimum};
	}

	template<unsigned N>
	ReturnWrapper<N> hillclimb_first_improve(Problem<N>& problem)
	{
		NDimensionPoint<N> best_solution;
		auto best_minimum = std::numeric_limits<float>::infinity();

		const unsigned T_MAX = 30;
		unsigned Temperature = 0;

		do
		{
			bool local = false;
			auto random_solution = get_random_point<N>();
			auto current_minimum = problem.functor(random_solution.coordinates);

			do
			{
				NDimensionPoint<N> neighbor_best;
				float neighbor_best_minimum = std::numeric_limits<float>::infinity();

				auto search_neighbours = [&]()->bool
				{
					for (unsigned i = 0; i < N; ++i)
					{
						for (unsigned l = 0; l < L; ++l)
						{
							auto neighbor = random_solution;
							neighbor.coordinates[i][l].flip();

							auto neighbor_minimum = problem.functor(neighbor.coordinates);

							if (neighbor_minimum < neighbor_best_minimum)
							{
								neighbor_best = neighbor;
								neighbor_best_minimum = neighbor_minimum;
								return true;
							}
						}
					}
					return false;
				};

				if (search_neighbours())
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
		} while (Temperature < T_MAX);


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
			unsigned repeats = 0;
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
		} while (Temperature > 0.001 && std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - term_time).count() < 2);//Terminating cond

		return ReturnWrapper<N>{best_solution, best_minimum};
	}
}
