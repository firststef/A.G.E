#include <fstream>
#include <sstream>

struct ProblemInstance
{
	std::string format;
	int num_variables;
	int num_clauses;
	std::vector<std::vector<int>> clauses;
};

ProblemInstance parse_file(const std::string& path)
{
	ProblemInstance result;
	std::ifstream inputFile(path.c_str());
	char ch;
	std::string line;
	bool isHeader = true;
	while (isHeader)
	{
		ch = inputFile.get();
		switch (ch)
		{
		case 'c':// Comment
			std::getline(inputFile, line);
			break;
		case 'p': {
			inputFile >> result.format >> result.num_variables >> result.num_clauses;
			result.clauses.reserve(result.num_clauses);
			isHeader = false;
			break;
		}
		default:
			break;
		}
	}
	while (!inputFile.eof())
	{
		std::getline(inputFile, line);
		std::istringstream lineStream(line);
		std::vector<int> values;
		int number;
		while (lineStream >> number)
		{
			if (number == 0)
				break;
			values.push_back(number);
		}
		if (! values.empty())
		{
			result.clauses.push_back(values);
		}
	}
	inputFile.close();
	return result;
}