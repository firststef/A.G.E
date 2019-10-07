#include <iostream>
#include "AlgorithmsAGE1.h"



int main(int argc, char** argv) {
	Sphere<2> func;
	Problem<2> problem{func, 0.000001f};

	NDimensionPoint<2> result = goda_algorithm(problem);


	return 0;
}
