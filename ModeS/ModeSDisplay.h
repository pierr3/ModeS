#pragma once
#include <vector>
#include <string>
#include "EuroScopePlugIn.h"
#include "ModeSCodes.h"

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

private:
	const CModeSCodes &ModeSCodes;
};
