#include "AlgorithmsAGE0.h"
#include "AlgorithmsAGE1.h"

int main(int argc, char** argv) {

	printf("================== A.G.E. source ==================\n");
	printf("### 2019 - Petrovici Stefan ###\n");

	if (argc <= 1)
	{
		printf("\nHELP: pass => age0/age1 for running source + the name of the function you want to analyze.\n");
		printf("\n  ex: AGEProj.exe age1 rastrigin\n");
		return 0;
	}

	printf("Analyzer is running, please wait.\n");
	printf("The output file (.json format) will be generated in this directory.\n");

	if (std::string(argv[1]) == std::string("age0"))
	{
		if (std::string(argv[2]) == std::string("rastrigin"))
		{
			AGE0::AlgorythmAnalyzerAGE0<AGE0::Rastrigin> analyzer_rastr;
			analyzer_rastr.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("schwefel"))
		{
			AGE0::AlgorythmAnalyzerAGE0<AGE0::Schwefel> analyzer_sch;
			analyzer_sch.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("rosenbrock"))
		{
			AGE0::AlgorythmAnalyzerAGE0<AGE0::Rosenbrock> analyzer_rsn;
			analyzer_rsn.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("sphere"))
		{
			AGE0::AlgorythmAnalyzerAGE0<AGE0::Sphere> analyzer_sph;
			analyzer_sph.create_output_file();
		}
		else
		{
			printf("Function not supported\n");
		}
	}
	else if (std::string(argv[1]) == std::string("age1"))
	{
		if (std::string(argv[2]) == std::string("rastrigin"))
		{
			AGE1::AlgorythmAnalyzerAGE1<AGE1::Rastrigin> analyzer_rastr;
			analyzer_rastr.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("schwefel"))
		{
			AGE1::AlgorythmAnalyzerAGE1<AGE1::Schwefel> analyzer_sch;
			analyzer_sch.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("rosenbrock"))
		{
			AGE1::AlgorythmAnalyzerAGE1<AGE1::Rosenbrock> analyzer_rsn;
			analyzer_rsn.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("sphere"))
		{
			AGE1::AlgorythmAnalyzerAGE1<AGE1::Sphere> analyzer_sph;
			analyzer_sph.create_output_file();
		}
		else
		{
			printf("Function not supported\n");
		}
	}
	else
	{
		printf("Command not recognized\n");
	}

	printf("Analysis finished\n");

	return 1;
}
