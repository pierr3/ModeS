#pragma once
#include <EuroScopePlugIn.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <regex>
#include <iostream>
#include "HttpHelper.hpp"
#include "ModeSDisplay.h"

using namespace std;
using namespace EuroScopePlugIn;

class CModeS :
	public EuroScopePlugIn::CPlugIn
{
public:
	CModeS();
	~CModeS();

	const int TAG_ITEM_ISMODES = 501;
	const int TAG_ITEM_MODESHDG = 502;
	const int TAG_ITEM_MODESROLLAGL = 503;
	const int TAG_ITEM_MODESREPGS = 504;

	const int TAG_FUNC_ASSIGNMODES = 869;
	const int TAG_FUNC_ASSIGNMODEAS = 870;

	const char* mode_s_code = "1000";

	clock_t delayedStart;
	
	CModeSDisplay * function_relay;


	void OnGetTagItem(CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget,
		int ItemCode,
		int TagData,
		char sItemString[16],
		int * pColorCode,
		COLORREF * pRGB,
		double * pFontSize);

	void OnFunctionCall(int FunctionId,
		const char * sItemString,
		POINT Pt,
		RECT Area);

	void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);

	void OnTimer(int Counter);

	CRadarScreen * OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);

	bool isAcModeS(CFlightPlan FlightPlan);
	inline bool startsWith(const char *pre, const char *str)
	{
		size_t lenpre = strlen(pre),
			lenstr = strlen(str);
		return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
	}

	inline string padWithZeros(int padding, int s)
	{
		stringstream ss;
		ss << setfill('0') << setw(padding) << s;
		return ss.str();
	}

};

