#include <iostream>
#include <regex>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <ctime>

#include "progress_bar.hpp"

namespace fs = std::filesystem;
using namespace std::literals;

struct ParseInfo
{
	ParseInfo operator+(ParseInfo other)
	{
		return ParseInfo{lines_found + other.lines_found, files_found + other.files_found, files_scanned + other.files_scanned};
	}
	ParseInfo& operator+=(ParseInfo other)
	{
		*this = *this + other;
		return *this;
	}

	int lines_found = 0;
	int files_found = 0;
	int files_scanned = 0;
};

int MeasureFile(std::ifstream& in)
{
	in.seekg(0, std::ios::end);
	int res = in.tellg();
	in.seekg(0, std::ios::beg);
	return res;
}

int Readline(std::ifstream& in, std::string& line, ProgressBar& bar)
{
	char ch;
	line.clear();
	int cnt = 0;
	while (in.get(ch) && ch != '\n')
	{
		line.push_back(ch);
		++cnt;
	}
	return cnt;
}

int SearchRegexInFile(std::ifstream& in, const std::regex& reg, std::ofstream& out)
{
	int cnt = 0;
	std::string line;
	int file_size = MeasureFile(in);
	ProgressBar bar(file_size);
	bar.SetFrequencyUpdate(file_size / 2);
	int progress = 0;
	while (int n = Readline(in, line, bar))
	{
		progress += n;
		bar.Progressed(progress);
		if (std::regex_search(line, reg))
		{
			++cnt;
			out << line << '\n';
		}
	}
	bar.Progressed(file_size);
	return cnt;
}

ParseInfo ParseLogInDir(const fs::path& dir, const std::regex& reg, std::ofstream& out)
{
	ParseInfo res;
	std::cout << "Directory: " << dir.string() << std::endl;
	for (const auto& file : fs::directory_iterator(dir))
	{
		std::ifstream input(file.path().string());
		if (!input)
		{
			std::cerr << "Can't open file " << file.path().string() << std::endl; 
			continue;
		}
		int cur_found = SearchRegexInFile(input, reg, out);
		std::cout << '\n' << file << ": found " << cur_found << " matches\n"; 
		res.lines_found += cur_found;
		res.files_found += (cur_found > 0);
		++res.files_scanned;
	}
	std::cout << std::endl;
	return res;
}

ParseInfo ParseLogFile(const fs::path& file, const std::regex& reg, std::ofstream& out)
{
	ParseInfo res;
	std::ifstream input(file.string());
	if (!input)
	{
		std::cerr << "Can't open file " << file.string() << std::endl;
		return res;
	}
	res.lines_found = SearchRegexInFile(input, reg, out);
	res.files_found = (res.lines_found > 0);
	res.files_scanned = 1;
	std::cout << '\n' << file.string() << ": found " << res.lines_found << " matches\n";
	return res;
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

	ParseInfo parse_results;
	for (const auto& file : fs::directory_iterator(fs::current_path()))
	{
		if (std::regex_search(file.path().string(), std::regex(argv[1])))
		{
			if (fs::is_directory(file.path()))
			{
				parse_results += ParseLogInDir(file.path(), std::regex(argv[2]), output);
			}
			else
			{
				parse_results += ParseLogFile(file.path(), std::regex(argv[2]), output);
			}
		}
	}

	std::cout << std::endl;
	std::cout << "Files scanned: " << parse_results.files_scanned << std::endl
		      << "Files matched: " << parse_results.files_found << std::endl
		      << "Lines matched: " << parse_results.lines_found << std::endl;			  
	return 0;
}
