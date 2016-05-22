#pragma once
#include <vector>
#include <string>
#include "EuroScopePlugIn.h"
#include "ModeSCodes.h"
#include "Helpers.h"

// Class for calling Tag Item Functions, to be able to open the standard
// squawk assignment menu

using namespace std;
using namespace EuroScopePlugIn;

class CModeSDisplay :
	public EuroScopePlugIn::CRadarScreen
{
public:
	explicit CModeSDisplay(const CModeSCodes& msc);
	virtual ~CModeSDisplay(){}
	inline void OnAsrContentToBeClosed()
	{
		delete this;
	}
	void OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area);

private:
	const int TAG_FUNC_ASSIGNMODEAS = 870;
	const CModeSCodes& msc;
};
