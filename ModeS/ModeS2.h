#pragma once
#include <vector>
#include <string>
#include <regex>
#include <future>
#include <thread>
#include <exception>
#include <map>
#include <cstdio>
#include <EuroScopePlugIn.h>
//#include "ModeSDisplay.h"
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

	bool ICAO();
	bool FAA();

private:
	future<string> fUpdateString;
	vector<string> ProcessedFlightPlans;
	CModeSCodes msc;
	const PluginData pluginData;
	const char* squawkVFR;
	bool acceptEquipmentICAO;
	bool acceptEquipmentFAA;

	void AutoAssignMSCC();
	void AssignPendingSquawks();
	void DoInitialLoad(future<string> & message);
	bool IsFlightPlanProcessed(CFlightPlan & FlightPlan);
	
	std::map<const char*, std::future<std::string>> PendingSquawks;
};
