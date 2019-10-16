#include "AlgorithmsAGE1.h"
#include <thread>

int main(int argc, char** argv) {

	printf("================== A.G.E. source ==================\n");
	printf("### 2019 - Petrovici Stefan ###\n");

	if (argc <= 1)
	{
		printf("\nHELP: use age1 analysis for running source + the name of the function you want to analyze.\n");
		system("pause");
		return 0;
	}

	printf("Analyzer is running, please wait.\n");
	printf("The output file (.json format) will be generated in this directory.\n");

	if (std::string(argv[1]) == std::string("age1"))
	{
		if (std::string(argv[2]) == std::string("rastrigin"))
		{
			AlgorythmAnalyzerAGE1<Rastrigin> analyzer_rastr;
			analyzer_rastr.create_output_file();
		}
		if (std::string(argv[2]) == std::string("schwefel"))
		{
			AlgorythmAnalyzerAGE1<Schwefel> analyzer_sch;
			analyzer_sch.create_output_file();
		}
		if (std::string(argv[2]) == std::string("rosenbrock"))
		{
			AlgorythmAnalyzerAGE1<Rosenbrock> analyzer_rsn;
			analyzer_rsn.create_output_file();
		}
		if (std::string(argv[2]) == std::string("sphere"))
		{
			AlgorythmAnalyzerAGE1<Sphere> analyzer_sph;
			analyzer_sph.create_output_file();
		}
	}

	return 1;
}
