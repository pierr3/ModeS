#pragma once

#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <regex>
#include <future>
#include <thread>
#include <EuroScopePlugIn.h>
#include "HttpHelper.hpp"
#include "ModeSDisplay.h"
#include "Helpers.h"

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

	future<string> fUpdateString;

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

	//void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);
	void OnRadarTargetPositionUpdate(CRadarTarget RadarTarget);

	void OnTimer(int Counter);

	CRadarScreen * OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);

	void AssignModeSCode(CFlightPlan& flightplan, string mode);

	inline string padWithZeros(int padding, int s)
	{
		stringstream ss;
		ss << setfill('0') << setw(padding) << s;
		return ss.str();
	}
};
