#include "stdafx.h"
#include "HttpHelper.hpp"

std::string LoadUpdateString(std::string url)
{
	HINTERNET connect = InternetOpen("MyBrowser", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!connect)
		throw("Connection Failed");

	HINTERNET OpenAddress = InternetOpenUrl(connect, url.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE, 0);
	if (!OpenAddress)
	{
		InternetCloseHandle(connect);
		std::string err = "Failed to open URL. Error: " + GetLastError();
		throw(err);
	}

	char DataReceived[4096];
	DWORD NumberOfBytesRead = 0;
	std::string answer {};
	while (InternetReadFile(OpenAddress, DataReceived, 100, &NumberOfBytesRead) && NumberOfBytesRead)
	{
		answer.append(DataReceived, NumberOfBytesRead);
	}

	InternetCloseHandle(OpenAddress);
	InternetCloseHandle(connect);
	return answer;
}