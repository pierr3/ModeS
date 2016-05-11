#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <EuroScopePlugIn.h>

inline bool startsWith(const char *pre, const char *str)
{
	size_t lenpre = strlen(pre),
		lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

inline bool isAcModeS(EuroScopePlugIn::CFlightPlan FlightPlan, std::vector<std::string>& EQUIPEMENT_CODES)
{
	std::string transponder_type { FlightPlan.GetFlightPlanData().GetCapibilities() };
	for (auto &code : EQUIPEMENT_CODES)
	{
		if (transponder_type == code)
		{
			return true;
		}
	}
	return false;
}

inline bool isApModeS(std::string icao, std::vector<std::string>& ICAO_MODES)
{
	for (auto& zone : ICAO_MODES)
	{
		if (startsWith(zone.c_str(), icao.c_str()))
			return true;
	}
	return false;
}

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
	while (std::getline(ss, item, delim))
	{
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