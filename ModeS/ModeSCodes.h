#pragma once
#include <EuroScopePlugIn.h>
#include <vector>
#include <string>

using namespace std;

class CModeSCodes
{
public:
	CModeSCodes();
	CModeSCodes(vector<string>& EQUIPEMENT_CODES, vector<string>& ICAO_MODES);
	CModeSCodes(vector<string>&& EQUIPEMENT_CODES, vector<string>&& ICAO_MODES);
	virtual ~CModeSCodes();

	bool isAcModeS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;
	bool isApModeS(const std::string& icao) const;

	void SetEquipementCodes(vector<string>&& EQUIPEMENT_CODES);
	void SetICAOModeS(vector<string>&& ICAO_MODES);

private:
	vector<string> EQUIPEMENT_CODES;
	vector<string> ICAO_MODES;

	static bool startsWith(const char *pre, const char *str);
	static bool startsWith(const string& zone, const string& icao);
};
