#include "stdafx.h"
#include "Helpers.h"

std::string LoadUpdateString(PluginData p)
{
	const std::string AGENT { "EuroScopeModeS/" + std::string { p.PLUGIN_VERSION } };
	HINTERNET connect = InternetOpen(AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!connect) {
		throw error { std::string {"Connection Failed. Error: " + std::to_string(GetLastError()) } };
	}

	HINTERNET OpenAddress = InternetOpenUrl(connect, p.UPDATE_URL, NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
	if (!OpenAddress) {
		InternetCloseHandle(connect);
		throw error { std::string { "Failed to load URL. Error: " + std::to_string(GetLastError()) } };
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

std::string LoadWebSquawk(std::string origin, std::string callsign)
{
	const std::string AGENT{ "EuroScopeModeS/EXP" };
	HINTERNET connect = InternetOpen(AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!connect) {
		throw error{ std::string {"Connection Failed. Error: " + std::to_string(GetLastError()) } };
	}

	std::string build_url = "https://kilojuliett.ch/webtools/api/ssrcode?orig=" + origin + "&callsign=" + callsign;

	HINTERNET OpenAddress = InternetOpenUrl(connect, build_url.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
	if (!OpenAddress) {
		InternetCloseHandle(connect);
		throw error{ std::string { "Failed to load URL. Error: " + std::to_string(GetLastError()) } };
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