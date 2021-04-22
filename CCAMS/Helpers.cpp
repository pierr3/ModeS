#include "stdafx.h"
#include "Helpers.h"


string LoadUpdateString(PluginData p)
{
	const std::string AGENT { "EuroScope CCAMS/" + std::string { p.PLUGIN_VERSION } };
	HINTERNET connect = InternetOpen(AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!connect) {
		throw error { std::string {"Connection failed to verify the plugin version. Error: " + std::to_string(GetLastError()) } };
	}

	HINTERNET OpenAddress = InternetOpenUrl(connect, p.UPDATE_URL, NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
	if (!OpenAddress) {
		InternetCloseHandle(connect);
		throw error { std::string { "Failed to load plugin version verification file. Error: " + std::to_string(GetLastError()) } };
	}

	char DataReceived[256];
	DWORD NumberOfBytesRead { 0 };
	std::string answer;
	while (InternetReadFile(OpenAddress, DataReceived, 256, &NumberOfBytesRead) && NumberOfBytesRead)
		answer.append(DataReceived, NumberOfBytesRead);

	InternetCloseHandle(OpenAddress);
	InternetCloseHandle(connect);
	return answer;
}

string LoadWebSquawk(EuroScopePlugIn::CFlightPlan FP, EuroScopePlugIn::CController ATCO, vector<const char*> usedCodes, bool vicinityADEP)
{
	PluginData p;
	const std::string AGENT{ "EuroScope CCAMS/" + std::string { p.PLUGIN_VERSION } };
	HINTERNET connect = InternetOpen(AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!connect) {
		return "0000";
	}

	string codes;
	for (size_t i = 0; i < usedCodes.size(); i++)
	{
		if (i > 0)
			codes += ",";
		codes += usedCodes[i];
	}

	string build_url = "https://ccams.kilojuliett.ch/squawk?callsign=" + string(ATCO.GetCallsign());
	if (FP.IsValid())
	{
		if (vicinityADEP)
		{
			build_url += "&orig=" + string(FP.GetFlightPlanData().GetOrigin());
		}
		build_url += "&dest=" + string(FP.GetFlightPlanData().GetDestination()) +
			"&flightrules=" + string(FP.GetFlightPlanData().GetPlanType());
		if (FP.GetFPState() == FLIGHT_PLAN_STATE_SIMULATED)
		{
			build_url += "&sim";
		}
	}
	build_url += "&codes=" + codes;

	HINTERNET OpenAddress = InternetOpenUrl(connect, build_url.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
	if (!OpenAddress) {
		InternetCloseHandle(connect);
#ifdef _DEBUG
		throw error{ std::string { "Failed reach the CCAMS server. Error: " + std::to_string(GetLastError()) } };

#endif // DEBUG

		return "0000";
	}

	char DataReceived[256];
	DWORD NumberOfBytesRead{ 0 };
	std::string answer;
	while (InternetReadFile(OpenAddress, DataReceived, 256, &NumberOfBytesRead) && NumberOfBytesRead)
		answer.append(DataReceived, NumberOfBytesRead);

	InternetCloseHandle(OpenAddress);
	InternetCloseHandle(connect);

	trim(answer);

	return answer;
}