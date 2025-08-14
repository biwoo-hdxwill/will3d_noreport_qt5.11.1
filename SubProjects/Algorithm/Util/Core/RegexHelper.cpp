#include "RegexHelper.h"
#include <regex>
#include <sstream>
using namespace std;

std::string RegexHelper::trim(const std::string& s, const std::string& searchPattern){
	string searchPattern2 = replace(searchPattern, "(.*)", "(^$1)|($1$)");
	return replace(s, searchPattern2, "");
}
std::string RegexHelper::trimHard(const std::string& s, const std::string& searchPattern){
	return replace(s, searchPattern, "");
}
std::string RegexHelper::replace(const std::string& s, const std::string& searchPattern, const std::string& replacePattern){
	regex reg_s(searchPattern);
	stringstream ss;
	regex_replace(ostreambuf_iterator<char>(ss), s.begin(), s.end(), reg_s, replacePattern);
	return ss.str();
}
bool RegexHelper::next(
	std::string& matched,
	int& begin,
	int& length,
	const std::string& s,
	const int& from,
	const std::string& tokenPattern
){
	matched.clear();
	regex reg(tokenPattern);
	for (auto it = sregex_token_iterator(s.begin()+from, s.end(), reg, -1);
		it != sregex_token_iterator(); it++){
		const auto& submatch = *it; // ssub_match
		if (!regex_match(submatch.str(), regex("\\s*"))){
			matched = submatch.str();
			begin = std::distance(s.begin(), submatch.first);
			length = submatch.length();
			return true;
		}
	}
	begin = -1;
	length = 0;
	return false;
}
std::vector<std::string> RegexHelper::split(const std::string& s, const std::string& tokenPattern){
	vector<string> res;
	regex reg(tokenPattern);
	for (sregex_token_iterator it = sregex_token_iterator(s.begin(), s.end(), reg, -1);
		it != sregex_token_iterator(); it++
		){
		if (!regex_match(it->str(), regex("\\s*"))){
			string seg = it->str();
			res.push_back(trim(seg));
		}
	}
	return res;
}
bool RegexHelper::search(std::vector<std::string>& submatchs, int& begin, int& length, const std::string& s, const std::string& searchPattern){
	submatchs.clear();
	regex reg(searchPattern);
	smatch m;
	regex_search(s, m, reg);
	for (const auto& submatch : m){
		submatchs.push_back(submatch.str());
	}
	begin = m.ready() ? m.position() : -1;
	length = m.length();
	return m.ready();
}
bool RegexHelper::searchAll(std::vector<std::vector<std::string>>& matchs, const std::string& s, const std::string& searchPattern){
	matchs.clear();
	regex reg(searchPattern);
	for (sregex_iterator it = sregex_iterator(s.begin(), s.end(), reg);
		it != sregex_iterator(); it++){
		vector<string> submatchs;
		for (const auto& submatch : *it){
			submatchs.push_back(submatch.str());
		}
		matchs.push_back(submatchs);
	}
	return !matchs.empty();
}
std::vector<std::string> RegexHelper::searchAll(const std::string& s, const std::string& searchPattern){
	vector<vector<string>> matchs;
	searchAll(matchs, s, searchPattern);
	vector<string> res;
	for (const auto& match : matchs){
		res.push_back(match[0]);
	}
	return res;
}
bool RegexHelper::match(const std::string& s, const std::string& matchPattern){
	return match(vector<string>(), s, matchPattern);
}

bool RegexHelper::match(std::vector<std::string>& submatchs, const std::string& s, const std::string& matchPatter){
	regex reg(matchPatter);
	smatch m;
	regex_match(s, m, reg);
	submatchs.clear();
	for (const auto& submatch : m){
		submatchs.push_back(submatch.str());
	}
	return !submatchs.empty();
}

//bool RegexHelper::match(const std::string& s, const std::string& matchPatter){
//	regex reg(matchPatter);
//	smatch m;
//	regex_match(s, m, reg);
//	return !m.empty();
//}

// example
/* 
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <iterator>
#include <sstream>
#include "RegexHelper.h"
using namespace std;
template<class T>
ostream& operator << (ostream& o, const vector<T>& vec){
	o << "[";
	for (int i = 0; i < vec.size(); i++){
		if (i < vec.size() - 1){
			o << vec[i] << ",";
		}
		else{
			o << vec[i];
		}
	}
	o << "]";
	return o;
}
int main(){
	string s = " 12 3; 4 56\n 7 89;1 01 \n ";
	cout << RegexHelper::split(s, ";|\\n") << endl;
	cout << RegexHelper::searchAll(s, "\\d{2}") << endl;
	cout << RegexHelper::replace("\\1", "(.*)", "(^$1)|($1$)") << endl;
	return 0;
}
*/
