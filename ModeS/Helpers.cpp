#include "stdafx.h"
#include "Helpers.h"

std::string LoadUpdateString(const PluginData& p)
{
	const std::string AGENT { "EuroScopeModeS/" + std::string { p.PLUGIN_VERSION } };
	HINTERNET connect = InternetOpen(AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!connect)
		throw error { "Connection Failed. Error: " + GetLastError() };

	HINTERNET OpenAddress = InternetOpenUrl(connect, p.UPDATE_URL, NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
	if (!OpenAddress) {
		InternetCloseHandle(connect);
		throw error { "Failed to load URL. Error: " + GetLastError() };
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
