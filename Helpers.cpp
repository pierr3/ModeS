#include "stdafx.h"
#include "Helpers.h"

string LoadUpdateString()
{
	const string AGENT { "EuroScope CCAMS/" + string { MY_PLUGIN_VERSION } };
	HINTERNET connect = InternetOpen(AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!connect) {
		throw error { string {"Connection failed to verify the plugin version. Error: " + to_string(GetLastError()) } };
	}

	HINTERNET OpenAddress = InternetOpenUrl(connect, MY_PLUGIN_UPDATE_URL, NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
	if (!OpenAddress) {
		InternetCloseHandle(connect);
		throw error { string { "Failed to load plugin version verification file. Error: " + to_string(GetLastError()) } };
	}

	char DataReceived[256];
	DWORD NumberOfBytesRead { 0 };
	string answer;
	while (InternetReadFile(OpenAddress, DataReceived, 256, &NumberOfBytesRead) && NumberOfBytesRead)
		answer.append(DataReceived, NumberOfBytesRead);

	InternetCloseHandle(OpenAddress);
	InternetCloseHandle(connect);
	return answer;
}

string LoadWebSquawk(EuroScopePlugIn::CFlightPlan FP, EuroScopePlugIn::CController ATCO, vector<const char*> usedCodes, bool vicinityADEP, int ConnectionType)
{
	//PluginData p;
	const string AGENT{ "EuroScope CCAMS/" + string { MY_PLUGIN_VERSION } };
	HINTERNET connect = InternetOpen(AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!connect) {
#ifdef _DEBUG
		//throw error { string { "Failed reach the CCAMS server. Error: " + to_string(GetLastError()) } };
#endif
		return string{ "E404" };
	}

	//int statusCode;
	char responseText[MAX_PATH]; // change to wchar_t for unicode
	DWORD responseTextSize = sizeof(responseText);

	//Check existance of page (for 404 error)
	if (HttpQueryInfo(connect, HTTP_QUERY_STATUS_CODE, &responseText, &responseTextSize, NULL))
	{
		//statusCode = ;
		if (atoi(responseText) != 200)
			return string{ "E" + string(responseText) };
	}

	//cpr::Response r = cpr::Get(cpr::Url{ "http://localhost/webtools/CCAMS/squawk" },
	//	cpr::Parameters{ {"callsign", ATCO.GetCallsign()} });
	//if (r.status_code != 200)
	//	return "CURL ERROR";


	string codes;
	for (size_t i = 0; i < usedCodes.size(); i++)
	{
		if (i > 0)
			codes += ",";
		codes += usedCodes[i];
	}

	//string build_url = "http://localhost/webtools/CCAMS/squawk?callsign=" + string(ATCO.GetCallsign());
	string build_url = "https://ccams.kilojuliett.ch/squawk?callsign=" + string(ATCO.GetCallsign());
	if (FP.IsValid())
	{
		if (vicinityADEP)
		{
			build_url += "&orig=" + string(FP.GetFlightPlanData().GetOrigin());
		}
		build_url += "&dest=" + string(FP.GetFlightPlanData().GetDestination()) +
			"&flightrule=" + string(FP.GetFlightPlanData().GetPlanType());
		if (ConnectionType > 2)
		{
			build_url += "&sim";
		}
	}
	if (codes.size() > 0)
	{
		build_url += "&codes=" + codes;
	}

	HINTERNET OpenAddress = InternetOpenUrl(connect, build_url.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
	if (!OpenAddress) {
		InternetCloseHandle(connect);
#ifdef _DEBUG
		//throw error{ string { "Failed reach the CCAMS server. Error: " + to_string(GetLastError()) } };
#endif

		return string{ "E406" };
	}

	char DataReceived[256];
	DWORD NumberOfBytesRead{ 0 };
	string answer;
	while (InternetReadFile(OpenAddress, DataReceived, 256, &NumberOfBytesRead) && NumberOfBytesRead)
		answer.append(DataReceived, NumberOfBytesRead);

	InternetCloseHandle(OpenAddress);
	InternetCloseHandle(connect);

	trim(answer);

	return answer;
}