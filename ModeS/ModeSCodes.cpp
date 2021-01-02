#include "stdafx.h"
#include "ModeSCodes.h"
#include "ModeS2.h"

CModeSCodes::CModeSCodes(const DefaultCodes && dc) :
	EQUIPEMENT_CODES(std::move(dc.EQUIPEMENT_CODES)),
	EQUIPEMENT_CODES_ICAO(std::move(dc.EQUIPEMENT_CODES_ICAO)),
	ICAO_MODES(std::move(dc.ICAO_MODES))
{}

CModeSCodes::~CModeSCodes() 
{}

bool CModeSCodes::isAcModeS(const EuroScopePlugIn::CFlightPlan & FlightPlan) const
{
	//check for ICAO suffix
	if (&CModeS::ICAO)
	{
		std::string actype = FlightPlan.GetFlightPlanData().GetAircraftInfo();
		std::regex icao_format("(.{2,4})\\/([LMHJ])-(.*)\\/(.*)", std::regex::icase);
		std::smatch acdata;
		if (std::regex_match(actype, acdata, icao_format) && acdata.size() == 5)
		{
			for (const auto& code : EQUIPEMENT_CODES_ICAO)
				if (acdata[4].str()._Starts_with(code))
					return true;
		}
	}

	//check for FAA suffix
	if (&CModeS::FAA)
	{
		std::string equipement_suffix{ FlightPlan.GetFlightPlanData().GetCapibilities() };
		if (equipement_suffix == "?")
			return false;

		for (auto& code : EQUIPEMENT_CODES)
			if (equipement_suffix == code)
				return true;
	}
	
	return false;
}

bool CModeSCodes::isApModeS(const std::string & icao) const
{
	for (auto& zone : ICAO_MODES)
		if (startsWith(zone, icao))
			return true;
	return false;
}

bool CModeSCodes::isFlightModeS(const EuroScopePlugIn::CFlightPlan & FlightPlan) const
{
	if (isAcModeS(FlightPlan) && isApModeS(FlightPlan.GetFlightPlanData().GetDestination()))
		return true;
	return false;
}

void CModeSCodes::SetEquipementCodes(std::vector<std::string> && equipement_codes)
{
	EQUIPEMENT_CODES = std::move(equipement_codes);
}

void CModeSCodes::SetICAOModeS(std::vector<std::string> && icao_modes)
{
	ICAO_MODES = std::move(icao_modes);
}

inline bool CModeSCodes::startsWith(const char * pre, const char * str)
{
	size_t lenpre = strlen(pre),
		lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

inline bool CModeSCodes::startsWith(const std::string & zone, const std::string & icao)
{
	if (zone.compare(0, zone.length(), icao, 0, zone.length()) == 0)
		return true;
	return false;
}
