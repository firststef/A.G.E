#pragma once
#include "FunctionsAGE1.h"
#include <memory>
#include <queue>

template<unsigned N>
struct NDimensionPoint
{
	float coordinates[N];
};

template<unsigned N>
struct Problem
{
	PerfTestFunctor<N> functor;
	float precision;
};

//template<int N>
//typedef NDimensionPoint<N>(*LowerBoundFunction)(PerfTestFunction<N>);

template<unsigned N>
struct CandidateSolutionNode
{
	Interval interval[N];
	float func_value;
	unsigned depth;

	CandidateSolutionNode<N>() = default;
	CandidateSolutionNode<N>(Interval& init)
	{
		for (unsigned i=0; i<N; ++i)
		{
			interval[i].left = init.left;
			interval[i].right = init.right;
		}

		func_value = std::numeric_limits<float>::infinity();
		depth = 0;
	}
};

template<unsigned N>
float lower_bound_function(PerfTestFunctor<N>& function, CandidateSolutionNode<N>& node)
{
	float point[N];

	for (unsigned i=0; i<N; ++i)
	{
		point[i] = (node.interval[i].left + node.interval[i].right) / 2;
	}

	return function(point);
}

//GODA: Global Optimization Deterministic Algorithm
template<unsigned N>
NDimensionPoint<N> goda_algorithm (Problem<N>& problem)
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
			for (unsigned i=0; i<N;++i)
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

	return point;
}