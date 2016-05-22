#include "stdafx.h"
#include "ModeSCodes.h"

CModeSCodes::CModeSCodes()
{
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
	{
		if (equipement_suffix == code) {
			return true;
		}
	}
	return false;
}

bool CModeSCodes::isApModeS(const std::string& icao) const
{
	for (auto& zone : ICAO_MODES)
	{
		if (startsWith(zone, icao)) {
			return true;
		}
	}
	return false;
}

const char * CModeSCodes::ModeSCode() const
{
	return mode_s_code;
}

void CModeSCodes::SetEquipementCodes(vector<string>&& equipement_codes)
{
	EQUIPEMENT_CODES = std::move(equipement_codes);
}

void CModeSCodes::SetICAOModeS(vector<string>&& icao_modes)
{
	ICAO_MODES = std::move(icao_modes);
}

bool CModeSCodes::startsWith(const char * pre, const char * str)
{
	size_t lenpre = strlen(pre),
		lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

bool CModeSCodes::startsWith(const string& zone, const string& icao)
{
	auto sz = zone.length();
	if (zone.compare(0, sz, icao, 0, sz) == 0)
		return true;
	return false;
}
