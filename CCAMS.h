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
#include "Helpers.h"

using namespace std;
using namespace EuroScopePlugIn;

class CCAMS :
	public EuroScopePlugIn::CPlugIn
{
public:
	explicit CCAMS(PluginData p = PluginData(), const DefaultCodes&& dc = DefaultCodes());
	virtual ~CCAMS();

	bool OnCompileCommand(const char* command);
	void OnGetTagItem(CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget,
					  int ItemCode,
					  int TagData,
					  char sItemString[16],
					  int * pColorCode,
					  COLORREF * pRGB,
					  double * pFontSize);

	void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);
	void OnFlightPlanDisconnect(CFlightPlan FlightPlan);

	void OnRefreshFpListContent(CFlightPlanList AcList);
	void OnFunctionCall(int FunctionId,
						const char * sItemString,
						POINT Pt,
						RECT Area);

	void OnTimer(int Counter);

	bool Help(const char* Command);

private:
	future<string> fUpdateString;
	vector<string> ProcessedFlightPlans;
	CFlightPlanList FpListEHS;
	const PluginData pluginData;
	const char* squawkVFR;
	bool acceptEquipmentICAO;
	bool acceptEquipmentFAA;
	bool autoAssignMSCC;
	bool autoAssign;
	int APTcodeMaxGS;
	int APTcodeMaxDist;
	std::vector<std::string> EQUIPMENT_CODES;
	std::vector<std::string> EQUIPMENT_CODES_ICAO;
	std::vector<std::string> EQUIPMENT_CODES_EHS;
	std::vector<std::string> ICAO_MODES;

	void AutoAssignMSCC();
	void AssignAutoSquawk(CFlightPlan& FlightPlan);
	void AssignPendingSquawks();
	void DoInitialLoad(future<string> & message);
	bool IsFlightPlanProcessed(CFlightPlan & FlightPlan);
	bool isAcModeS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;
	bool isApModeS(const std::string& icao) const;
	bool isEHS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;
	bool isADEPvicinity(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;
	bool isEligibleSquawkModeS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;
	bool hasValidSquawk(const EuroScopePlugIn::CFlightPlan& FlightPlan);

	std::map<const char*, std::future<std::string>> PendingSquawks;
	std::vector<const char*> collectUsedCodes(const EuroScopePlugIn::CFlightPlan& FlightPlan);
};
