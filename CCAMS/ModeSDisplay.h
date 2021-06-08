#pragma once
#include <vector>
#include <string>
#include <future>
#include <thread>
#include <map>
#include <cstdio>
#include "EuroScopePlugIn.h"
#include "ModeSCodes.h"
#include "Helpers.h"

// Class for calling Tag Item Functions, to be able to open the standard
// squawk assignment menu

class CModeSDisplay :
	public EuroScopePlugIn::CRadarScreen
{
public:
	explicit CModeSDisplay(const CModeSCodes & msc);
	~CModeSDisplay() {}
	inline void OnAsrContentToBeClosed() { delete this; }
	void OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area);
	void OnRefresh(HDC hDC, int Phase);

private:
	const CModeSCodes &ModeSCodes;

	std::map<const char *, std::future<std::string>> PendingSquawks;
};
