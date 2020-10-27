#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <exception>
#include <WinInet.h>
#include "version.h"

std::string LoadUpdateString(PluginData p);

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
	explicit modesexception(std::string & what) : std::exception { what.c_str() } {}
	virtual inline const long icon() const = 0;
	inline void whatMessageBox()
	{
		MessageBox(NULL, what(), "Mode S", MB_OK | icon());
	}
};

class error
	: public modesexception
{
public:
	explicit error(std::string && what) : modesexception { what } {}
	inline const long icon() const
	{
		return MB_ICONERROR;
	}
};

class warning
	: public modesexception
{
public:
	explicit warning(std::string && what) : modesexception { what } {}
	inline const long icon() const
	{
		return MB_ICONWARNING;
	}
};
