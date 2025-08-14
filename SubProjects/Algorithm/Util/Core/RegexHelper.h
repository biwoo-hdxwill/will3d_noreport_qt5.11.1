#pragma once
#include "util_global.h"
#include <vector>
#include <string>
#include <regex>

class UTIL_EXPORT RegexHelper{
public:
	//static std::string trim(
	//	const std::string& str,
	//	const std::string& searchPattern = "\\s"
	//	);
	static std::string trim(
		const std::string& str, 
		const std::string& searchPattern = "\\s"
		);

	//static std::string trimHard(
	//	const std::string& str,
	//	const std::string& searchPattern = "\\s"
	//	);
	static std::string trimHard(
		const std::string& str, 
		const std::string& searchPattern = "\\s"
		);

	//static std::string replace(
	//	const std::string& s,
	//	const std::string& searchPattern,
	//	const std::string& replacePattern
	//	);
	static std::string replace(
		const std::string& s, 
		const std::string& searchPattern, 
		const std::string& replacePattern
		);

	//static bool next(
	//	std::string& matched,
	//	int& begin,
	//	int& length,
	//	const std::string& s,
	//	const int& from,
	//	const std::string& tokenPattern = "\\s"
	//	);
	static bool next(
		std::string& matched,
		int& begin,
		int& length,
		const std::string& s,
		const int& from,
		const std::string& tokenPattern = "\\s"
		);

	//static std::vector<std::string> split(
	//	const std::string& str,
	//	const std::string& tokenPattern = "\\s"
	//	);
	static std::vector<std::string> split(
		const std::string& str, 
		const std::string& tokenPattern = "\\s"
		);

	//static bool search(
	//	std::vector<std::string>& submatchs,
	//	int& begin,
	//	int& length,
	//	const std::string& s,
	//	const std::string& searchPattern
	//	);
	static bool search(
		std::vector<std::string>& submatchs, 
		int& begin, 
		int& length,
		const std::string& s,
		const std::string& searchPattern
		);

	//static bool searchAll(
	//	std::vector<std::vector<std::string>>& matchs,
	//	const std::string& s,
	//	const std::string& searchPattern
	//	);
	static bool searchAll(
		std::vector<std::vector<std::string>>& matchs, 
		const std::string& s, 
		const std::string& searchPattern
		);

	//static std::vector<std::string> searchAll(
	//	const std::string& s,
	//	const std::string& searchPattern
	//	);
	static std::vector<std::string> searchAll(
		const std::string& s, 
		const std::string& searchPattern
		);

	//static bool match(
	//	std::vector<std::string>& submatchs,
	//	const std::string& s,
	//	const std::string& matchPattern
	//	);
	static bool match(
		std::vector<std::string>& submatchs, 
		const std::string& s, 
		const std::string& matchPattern
		);

	//static bool match(
	//	const std::string& s,
	//	const std::string& matchPattern
	//	);
	static bool match(
		const std::string& s, 
		const std::string& matchPattern
		);
	
};
