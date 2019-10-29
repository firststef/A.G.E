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

		const unsigned T_MAX = 100000;
		unsigned Temperature = 0;

		do
		{
			bool local = false;
			auto random_solution = get_random_point<N>();
			auto current_minimum = problem.functor(random_solution.coordinates);

			do
			{
				NDimensionPoint<N> neighbor_best;
				float neighbor_best_minimum = std::numeric_limits<float>::infinity();;
				
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
}
