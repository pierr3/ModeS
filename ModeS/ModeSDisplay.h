#pragma once
#include <vector>
#include <string>
#include "EuroScopePlugIn.h"
#include "Helpers.h"

// Class for calling Tag Item Functions, to be able to open the standard
// squawk assignment menu

using namespace std;
using namespace EuroScopePlugIn;

class CModeSDisplay :
	public EuroScopePlugIn::CRadarScreen
{
public:
	const int TAG_FUNC_ASSIGNMODEAS = 870;
	vector<string>* EQUIPEMENT_CODES;
	vector<string>* ICAO_MODES;

	CModeSDisplay(vector<string>* EQUIPEMENT_CODES, vector<string>*	ICAO_MODES);
	virtual ~CModeSDisplay(){}
	inline void OnAsrContentToBeClosed()
	{
		delete this;
	}
	void OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area);
};
