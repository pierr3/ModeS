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

#define MY_PLUGIN_NAME			"CCAMS"
#define MY_PLUGIN_VERSION		"1.7.8"
#define MY_PLUGIN_VERSIONCODE	978
#define MY_PLUGIN_UPDATE_URL	"https://raw.githubusercontent.com/kusterjs/CCAMS/master/CCAMS/ver.txt"
#define MY_PLUGIN_DEVELOPER		"Jonas Kuster, Pierre Ferran, Oliver Grützmann"
#define MY_PLUGIN_COPYRIGHT		"GPL v3"
//#define MY_PLUGIN_VIEW      "Standard ES radar screen"

//struct VersionCheck
//{
//	const char* UPDATE_URL{ "https://raw.githubusercontent.com/kusterjs/CCAMS/master/CCAMS/ver.txt" };
//	const int VERSION_CODE{ 977 };
//};

struct ItemCodes
{
	enum ItemTypes : int
	{
		TAG_ITEM_ISMODES = 501,
		TAG_ITEM_EHS_HDG,
		TAG_ITEM_EHS_ROLL,
		TAG_ITEM_EHS_GS,
		TAG_ITEM_ERROR_MODES_USE,
		TAG_ITEM_SQUAWK
	};

	enum ItemFunctions : int
	{
		TAG_FUNC_SQUAWK_POPUP = 869,
		TAG_FUNC_ASSIGN_SQUAWK,
		TAG_FUNC_ASSIGN_SQUAWK_AUTO,
		TAG_FUNC_ASSIGN_SQUAWK_MANUAL,
		TAG_FUNC_ASSIGN_SQUAWK_VFR,
		TAG_FUNC_ASSIGN_SQUAWK_MODES,
		TAG_FUNC_ASSIGN_SQUAWK_DISCRETE
	};
};

struct EquipmentCodes
{
	const std::vector<std::string> FAA{ "H", "L", "E", "G", "W", "Q", "S" };
	const std::vector<std::string> ICAO_MODE_S{ "E", "H", "I", "L", "S" };
	const std::vector<std::string> ICAO_EHS{ "E", "H", "L", "S" };
};

struct SquawkCodes
{
	const char* MODE_S{ "1000" };
	const char* VFR{ "7000" };
};

static const std::vector<std::string> MODE_S_AIRPORTS{ "EB", "ED", "EH", "EL", "EP", "ET", "LD", "LF", "LH", "LI", "LK", "LO", "LR", "LSZR", "LSZB", "LSZG", "LSGC", "LSZH", "LSGG", "LZ" };



class CCAMS :
	public EuroScopePlugIn::CPlugIn
{
public:
	explicit CCAMS(const EquipmentCodes&& ec = EquipmentCodes(), const SquawkCodes&& sc = SquawkCodes());
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

	bool PluginCommands(const char* Command);

	bool isADEPvicinity(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;

protected:


private:
	future<string> fUpdateString;
	vector<string> ProcessedFlightPlans;
	CFlightPlanList FpListEHS;
	//const PluginData pluginData;
	const char* squawkModeS;
	const char* squawkVFR;
	bool pluginVersionRestricted;
	bool acceptEquipmentICAO;
	bool acceptEquipmentFAA;
	bool autoAssign;
	int APTcodeMaxGS;
	int APTcodeMaxDist;
	//std::vector<std::string> EQUIPMENT_CODES;
	//std::vector<std::string> EQUIPMENT_CODES_ICAO;
	//std::vector<std::string> EQUIPMENT_CODES_EHS;
	//std::vector<std::string> ICAO_MODES;
	std::vector<std::string> EquipmentCodesFAA;
	std::vector<std::string> EquipmentCodesICAO;
	std::vector<std::string> EquipmentCodesICAOEHS;
	std::vector<std::string> ModeSAirports;

	//void AutoAssignMSCC();
	void AssignAutoSquawk(CFlightPlan& FlightPlan);
	void AssignPendingSquawks();
	void DoInitialLoad(future<string> & message);
	bool IsFlightPlanProcessed(CFlightPlan & FlightPlan);
	bool isAcModeS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;
	bool isApModeS(const std::string& icao) const;
	bool isEHS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;
	bool isEligibleSquawkModeS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const;
	bool hasValidSquawk(const EuroScopePlugIn::CFlightPlan& FlightPlan);

	std::map<const char*, std::future<std::string>> PendingSquawks;
	std::vector<const char*> collectUsedCodes(const EuroScopePlugIn::CFlightPlan& FlightPlan);
};
