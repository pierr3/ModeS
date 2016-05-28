#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <exception>
#include <WinInet.h>
#include "version.h"

std::string LoadUpdateString(const PluginData & p);

inline std::string padWithZeros(int padding, int s)
{
	std::ostringstream ss;
	ss << std::setfill('0') << std::setw(padding) << s;
	return ss.str();
}

inline std::vector<std::string> split(const std::string & s, char delim)
{
	std::istringstream ss(s);
	std::string item;
	std::vector<std::string> elems;

	while (std::getline(ss, item, delim))
		elems.push_back(item);
	return elems;
}

class modesexception
	: public std::exception
{
public:
	explicit modesexception(const char * what) : std::exception { what } {}
	virtual inline const long icon() const = 0;
};

class error
	: public modesexception
{
public:
	explicit error(const char * what) : modesexception { what } {}
	inline const long icon() const
	{
		return MB_ICONERROR;
	}
};

class warning
	: public modesexception
{
public:
	explicit warning(const char * what) : modesexception { what } {}
	inline const long icon() const
	{
		return MB_ICONWARNING;
	}
};
