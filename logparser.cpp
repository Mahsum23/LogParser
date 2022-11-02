#include <iostream>
#include <regex>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace fs = std::filesystem;
using namespace std::literals;

int SearchRegexInFile(std::ifstream& in, const std::regex& reg, std::ofstream& out)
{
	int cnt = 0;
	std::string line;
	int progress = 0;
	while (getline(in, line))
	{
		if (std::regex_search(line, reg))
		{
			++cnt;
			out << line << '\n';
		}
	}
	return cnt;
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cerr << "Usage: ./logparser [logfile] [regex]" << std::endl;
		return 1;
	}

	std::time_t t = time(0);
	std::tm* now = std::localtime(&t);
	std::stringstream timestamp;
	timestamp << std::setw(2) << std::setfill('0') << now->tm_mday << "-"
		      << now->tm_mon + 1 << "-" 
			  << now->tm_year + 1900 << "-" 
			  << std::setw(2) << std::setfill('0') << now->tm_hour << "-" 
			  << std::setw(2) << std::setfill('0') << now->tm_min;

	std::ofstream output("log-search-results"s + timestamp.str() + ".log"s);
	if (!output)
	{
		std::cerr << "Can't create a file" << std::endl;
		return 2;
	}

	int lines_found = 0, files_found = 0, filenum = 0;
	if (fs::is_directory(fs::path(argv[1])))
	{
		for (const auto& file : fs::directory_iterator(fs::path(argv[1])))
		{
			std::ifstream input(file.path().string());
			if (!input)
			{
				std::cerr << "Can't open file " << file << std::endl; 
				continue;
			}
			int cur_found = SearchRegexInFile(input, std::regex(argv[2]), output);
			std::cout << file << ": found " << cur_found << " matches\n"; 
			lines_found += cur_found;
			files_found += (cur_found > 0);
			++filenum;
		}
	}
	else
	{
		std::ifstream input(argv[1]);
		if (!input)
		{
			std::cerr << "Can't open file " << argv[1] << std::endl;
			return 2;
		}
		lines_found = SearchRegexInFile(input, std::regex(argv[2]), output);
		if (lines_found > 0)
			files_found = 1;
		filenum = 1;
	}
	std::cout << "Files scanned: " << filenum << std::endl
		      << "Files matched: " << files_found << std::endl
		      << "Lines matched: " << lines_found << std::endl;			  
	return 0;
}
