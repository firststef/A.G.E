#include "AlgorithmsAGE0.h"
#include "AlgorithmsAGE1.h"
#include "AlgorithmsAGE1p.h"
#include "AlgorithmsAGE2.h"
#include "AlgorithmsAGE3.h"

int main(int argc, char** argv) {

	printf("================== A.G.E. source ==================\n");
	printf("### 2019 - Petrovici Stefan ###\n");

	if (argc < 2)
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
	else if (std::string(argv[1]) == std::string("age1t"))
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
	else if (std::string(argv[1]) == std::string("age1p"))
	{
		AGE1p::GenericFunction<2> f_dim_2; AGE1p::GenericFunction<2> problem_dim_2({ f_dim_2 });

		if (std::string(argv[2]) == std::string("generic"))
		{
			AGE1p::AlgorythmAnalyzerAGE1p<AGE1p::GenericFunction> analyzer_genr;
			analyzer_genr.create_output_file();
		}
		if (std::string(argv[2]) == std::string("trace"))
		{
			AGE1p::AlgorythmTracerAGE1p<AGE1p::GenericFunction> tracer_genr;
			tracer_genr.trace_hc();
		}
		else
		{
			printf("Function not supported\n");
		}
	}
	else if (std::string(argv[1]) == std::string("age2"))
	{
		if (std::string(argv[2]) == std::string("rastrigin"))
		{
			AGE2::AlgorythmAnalyzerAGE2<AGE2::Rastrigin> analyzer;
			analyzer.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("schwefel"))
		{
			AGE2::AlgorythmAnalyzerAGE2<AGE2::Schwefel> analyzer;
			analyzer.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("rosenbrock"))
		{
			AGE2::AlgorythmAnalyzerAGE2<AGE2::Rosenbrock> analyzer;
			analyzer.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("sphere"))
		{
			AGE2::AlgorythmAnalyzerAGE2<AGE2::Sphere> analyzer;
			analyzer.create_output_file();
		}
		else
		{
			printf("Function not supported\n");
		}
	}
	else if (std::string(argv[1]) == std::string("age2p"))
	{
		if (std::string(argv[2]) == std::string("rastrigin"))
		{
			//AGE2::AlgorythmAnalyzerAGE2<AGE2::Rastrigin> analyzer1;
			//analyzer1.create_output_file();
			AGE2::AlgorythmAnalyzerAGE2p<AGE2::Rastrigin> analyzer2;
			analyzer2.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("schwefel"))
		{
			//AGE2::AlgorythmAnalyzerAGE2<AGE2::Schwefel> analyzer1;
			//analyzer1.create_output_file();
			AGE2::AlgorythmAnalyzerAGE2p<AGE2::Schwefel> analyzer2;
			analyzer2.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("rosenbrock"))
		{
			//AGE2::AlgorythmAnalyzerAGE2<AGE2::Rosenbrock> analyzer1;
			//analyzer1.create_output_file();
			AGE2::AlgorythmAnalyzerAGE2p<AGE2::Rosenbrock> analyzer2;
			analyzer2.create_output_file();
		}
		else if (std::string(argv[2]) == std::string("sphere"))
		{
			//AGE2::AlgorythmAnalyzerAGE2<AGE2::Sphere> analyzer1;
			//analyzer1.create_output_file();
			AGE2::AlgorythmAnalyzerAGE2p<AGE2::Sphere> analyzer2;
			analyzer2.create_output_file();
		}
		else
		{
			printf("Function not supported\n");
		}
	}
	else if (std::string(argv[1]) == std::string("age3"))
	{
		const auto start = std::chrono::system_clock::now();
		auto file = parse_file(argv[2]);
		auto x = AGE3::sat_solver(file);
		//auto y = AGE3::random_search(file);
	}
	else
	{

		printf("Command not recognized\n");
	}

	printf("Analysis finished\n");

	return 1;
}
