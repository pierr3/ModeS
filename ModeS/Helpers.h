#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <exception>
#include <WinInet.h>
#include "version.h"

std::string LoadUpdateString();

inline std::string padWithZeros(int padding, int s)
{
	std::ostringstream ss;
	ss << std::setfill('0') << std::setw(padding) << s;
	return ss.str();
}

inline std::vector<std::string> split(const std::string &s, char delim)
{
	std::istringstream ss(s);
	std::string item;
	std::vector<std::string> elems;

	while (std::getline(ss, item, delim))
		elems.push_back(item);
	return elems;
}