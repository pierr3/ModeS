#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <EuroScopePlugIn.h>

inline std::string padWithZeros(int padding, int s)
{
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(padding) << s;
	return ss.str();
}

inline std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

inline std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}
