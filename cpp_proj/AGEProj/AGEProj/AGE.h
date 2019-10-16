#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <cmath>
#include <time.h>

struct Interval {
	float left;
	float right;
};

template<unsigned N>
struct NDimensionPoint
{
	float coordinates[N];
};

template<unsigned N>
NDimensionPoint<N> get_random_point(Interval interval[N])
{
	NDimensionPoint<N> point;
	
	srand(time(NULL));
	for (int i=0; i < N; ++i)
	{
		point.coordinates[i] = interval[i].left + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (interval[i].right - interval[i].left)));
	}

	return point;
}