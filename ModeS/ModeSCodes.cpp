#include "stdafx.h"
#include "ModeSCodes.h"

CModeSCodes::CModeSCodes()
{
	DefaultCodes dc;
	EQUIPEMENT_CODES = std::move(dc.EQUIPEMENT_CODES);
	ICAO_MODES = std::move(dc.ICAO_MODES);
}

CModeSCodes::CModeSCodes(vector<string>& EQUIPEMENT_CODES, vector<string>& ICAO_MODES)
	: EQUIPEMENT_CODES(EQUIPEMENT_CODES), ICAO_MODES(ICAO_MODES)
{
}

CModeSCodes::CModeSCodes(vector<string>&& EQUIPEMENT_CODES, vector<string>&& ICAO_MODES)
	: EQUIPEMENT_CODES(std::move(EQUIPEMENT_CODES)), ICAO_MODES(std::move(ICAO_MODES))
{
}

CModeSCodes::~CModeSCodes()
{
}

bool CModeSCodes::isAcModeS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const
{
	std::string equipement_suffix { FlightPlan.GetFlightPlanData().GetCapibilities() };
	if (equipement_suffix == "?")
		return false;

	for (auto &code : EQUIPEMENT_CODES)
		if (equipement_suffix == code)
			return true;
	return false;
}

bool CModeSCodes::isApModeS(const std::string& icao) const
{
	for (auto& zone : ICAO_MODES)
		if (startsWith(zone, icao))
			return true;
	return false;
}

void CModeSCodes::SetEquipementCodes(vector<string>&& equipement_codes)
{
	EQUIPEMENT_CODES = std::move(equipement_codes);
}

void CModeSCodes::SetICAOModeS(vector<string>&& icao_modes)
{
	ICAO_MODES = std::move(icao_modes);
}

inline bool CModeSCodes::startsWith(const char * pre, const char * str)
{
	size_t lenpre = strlen(pre),
		lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

inline bool CModeSCodes::startsWith(const string& zone, const string& icao)
{
	size_t len = zone.length();
	return zone.compare(0, len, icao, 0, len) == 0;
}
