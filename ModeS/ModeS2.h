#pragma once
#include <vector>
#include <string>
#include <regex>
#include <future>
#include <thread>
#include <exception>
#include <cstdio>
#include <EuroScopePlugIn.h>
#include "ModeSDisplay.h"
#include "ModeSCodes.h"
#include "Helpers.h"

using namespace std;
using namespace EuroScopePlugIn;

class CModeS :
	public EuroScopePlugIn::CPlugIn
{
public:
	explicit CModeS(PluginData p = PluginData());
	~CModeS();

	void OnGetTagItem(CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget,
					  int ItemCode,
					  int TagData,
					  char sItemString[16],
					  int * pColorCode,
					  COLORREF * pRGB,
					  double * pFontSize);

	void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);
	void OnFlightPlanDisconnect(CFlightPlan FlightPlan);

	void OnFunctionCall(int FunctionId,
						const char * sItemString,
						POINT Pt,
						RECT Area);

	void OnTimer(int Counter);
	CRadarScreen * OnRadarScreenCreated(const char * sDisplayName,
										bool NeedRadarContent,
										bool GeoReferenced,
										bool CanBeSaved,
										bool CanBeCreated);

private:
	future<string> fUpdateString;
	vector<string> ProcessedFlightPlans;
	CModeSCodes msc;
	const PluginData pluginData;
	
	void AutoAssignMSCC();
	void DoInitialLoad(future<string> & message);
	bool IsFlightPlanProcessed(CFlightPlan & FlightPlan);
	
};
