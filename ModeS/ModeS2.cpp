#include "stdafx.h"
#include "ModeS2.h"


const string updateUrl = "http://www.cherryx.de/modes/modes.txt";
const int VERSION_CODE = 902;
const char PLUGIN_VERSION[] = "1.3.3e32";

vector<string>	EQUIPEMENT_CODES = { "H", "L", "E", "G", "W", "Q", "S" };
vector<string>	ICAO_MODES = { "EB", "EL", "LS", "ET", "ED", "LF", "EH", "LK", "LO", "LIM", "LIR" };

void doInitialLoad(string message)
{
	if (regex_match(message, std::regex("^([A-z,]+)[|]([A-z,]+)[|]([0-9]{1,3})$")))
	{
		vector<string> data = split(message, '|');

		EQUIPEMENT_CODES = split(data.front(), ',');
		ICAO_MODES = split(data.at(1), ',');

		int new_v = std::stoi(data.back(), nullptr, 0);

		if (new_v > VERSION_CODE) 
		{
			MessageBox(NULL, "A new version of the mode S plugin is available, please update it", "Mode S", MB_OK);
		}
	}	
	else
	{
		MessageBox(NULL, "The mode S plugin couldn't parse the server data, please update the plugin", "Mode S", MB_OK);
	}
}

CModeS::CModeS():CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
	"Mode S PlugIn",
	PLUGIN_VERSION,
	"Pierre Ferran / Oliver Grützmann",
	"GPL v3")
{

	RegisterTagItemType("Transponder type", TAG_ITEM_ISMODES);
	RegisterTagItemType("Mode S: Reported Heading", TAG_ITEM_MODESHDG); 
	RegisterTagItemType("Mode S: Roll Angle", TAG_ITEM_MODESROLLAGL);
	RegisterTagItemType("Mode S: Reported GS", TAG_ITEM_MODESREPGS);

	RegisterTagItemFunction("Assign mode S squawk", TAG_FUNC_ASSIGNMODES);
	
	// Display to reach StartTagFunction from the normal plugin
	RegisterDisplayType("ModeS Function Relay (no display)", false, false, false, false);
	
	// Function to assign code 1000 or open the assign squawk popup.
	RegisterTagItemFunction("Assign mode S/A squawk", TAG_FUNC_ASSIGNMODEAS);

	fUpdateString = std::async(LoadUpdateString, updateUrl);
}

CModeS::~CModeS()
{
}

void CModeS::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int * pColorCode, COLORREF * pRGB, double * pFontSize)
{

	if (ItemCode == TAG_ITEM_ISMODES) {
		if (!FlightPlan.IsValid())
			return;

		if (isAcModeS(FlightPlan, EQUIPEMENT_CODES)) {
			strcpy_s(sItemString, 16, "S");
		}
		else {
			strcpy_s(sItemString, 16, "A");
		}
	}

	if (ItemCode == TAG_ITEM_MODESHDG)
	{
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (isAcModeS(FlightPlan, EQUIPEMENT_CODES)) {
			string rhdg = padWithZeros(3, RadarTarget.GetPosition().GetReportedHeading());
			strcpy_s(sItemString, 16, rhdg.c_str());
		}
	}

	if (ItemCode == TAG_ITEM_MODESROLLAGL)
	{
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (isAcModeS(FlightPlan, EQUIPEMENT_CODES)) {
			int rollb = RadarTarget.GetPosition().GetReportedBank();
			string roll = "L";
			if (rollb < 0) {
				roll = "R";
			}
			roll += std::to_string(abs(rollb));

			strcpy_s(sItemString, 16, roll.c_str());
		}
	}

	if (ItemCode == TAG_ITEM_MODESREPGS) {
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (isAcModeS(FlightPlan, EQUIPEMENT_CODES) && FlightPlan.GetCorrelatedRadarTarget().IsValid())
			strcpy_s(sItemString, 16, std::to_string(RadarTarget.GetPosition().GetReportedGS()).c_str());
	}
}

void CModeS::OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area)
{
	CFlightPlan FlightPlan = FlightPlanSelectASEL();

	if (!FlightPlan.IsValid() || !FlightPlan.GetTrackingControllerIsMe())
		return;

	if (!ControllerMyself().IsValid() || !ControllerMyself().IsController())
		return;

	if (FunctionId == TAG_FUNC_ASSIGNMODES && isAcModeS(FlightPlan, EQUIPEMENT_CODES)) {
		string Dest = FlightPlan.GetFlightPlanData().GetDestination();

		if (isApModeS(Dest, ICAO_MODES))
			AssignModeSCode(FlightPlan, " (manual)");
	}
	if (FunctionId == TAG_FUNC_ASSIGNMODEAS) {
		string Dest = FlightPlan.GetFlightPlanData().GetDestination();
		
		if (isAcModeS(FlightPlan, EQUIPEMENT_CODES) && isApModeS(Dest, ICAO_MODES))
			AssignModeSCode(FlightPlan, " (manual)");
	}
}

void CModeS::OnRadarTargetPositionUpdate(CRadarTarget RadarTarget)
{
	if (!ControllerMyself().IsValid() || !ControllerMyself().IsController())
		return;

	if (!RadarTarget.IsValid() || RadarTarget.GetPosition().IsFPTrackPosition())
		return;
	
	if (RadarTarget.GetPosition().GetFlightLevel() < 24500)
		return;

	CFlightPlan flightplan = RadarTarget.GetCorrelatedFlightPlan();
	if (!flightplan.IsValid() || !flightplan.GetTrackingControllerIsMe())
		return;
	
	if (!strcmp(mode_s_code, flightplan.GetControllerAssignedData().GetSquawk()) || !isAcModeS(flightplan, EQUIPEMENT_CODES))
		return;

	string destination { flightplan.GetFlightPlanData().GetDestination() };
	if (isApModeS(destination, ICAO_MODES))
		AssignModeSCode(flightplan, " (auto/airborne)");
}

void CModeS::AssignModeSCode(CFlightPlan& flightplan, string mode)
{
	bool success{ flightplan.GetControllerAssignedData().SetSquawk(mode_s_code) };
	
	string message { "Code 1000 assignment to " };
	message += flightplan.GetCallsign() + mode;
	if (success)
		DisplayUserMessage("ModeS", "Debug", message.c_str(), true, false, false, false, false);
	else {
		message += " failed";
		DisplayUserMessage("ModeS", "Debug", message.c_str(), true, false, false, false, false);
	}
}

void CModeS::OnTimer(int Counter)
{
	if (fUpdateString.valid() && fUpdateString.wait_for(chrono::milliseconds(0)) == future_status::ready)
	{
		try
		{
			string UpdateString = fUpdateString.get();
			doInitialLoad(UpdateString);
		}
		catch (exception& e)
		{
			DisplayUserMessage("Message", "Mode S", e.what(), true, false, false, false, false);
		}
	}
}

CRadarScreen * CModeS::OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	return new CModeSDisplay(&EQUIPEMENT_CODES, &ICAO_MODES);
}
