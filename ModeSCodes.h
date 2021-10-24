#pragma once
#include <EuroScopePlugIn.h>
#include <vector>
#include <string>
#include <regex>

class CModeSCodes
{
public:
	explicit CModeSCodes(const DefaultCodes && dc = DefaultCodes());
	~CModeSCodes();

	bool isAcModeS(const EuroScopePlugIn::CFlightPlan & FlightPlan) const;
	bool isEHS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;
	bool isApModeS(const std::string & icao) const;
	//bool isFlightModeS(const EuroScopePlugIn::CFlightPlan & FlightPlan) const;

	void SetEquipementCodes(std::vector<std::string> && EQUIPEMENT_CODES);
	void SetICAOModeS(std::vector<std::string> && ICAO_MODES);

private:
	std::vector<std::string> EQUIPEMENT_CODES;
	std::vector<std::string> EQUIPEMENT_CODES_ICAO;
	std::vector<std::string> EQUIPEMENT_CODES_EHS;
	std::vector<std::string> ICAO_MODES;

	//static bool startsWith(const char * pre, const char * str);
	static bool startsWith(const std::string & zone, const std::string & icao);
};
