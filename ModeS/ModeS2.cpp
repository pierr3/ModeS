#include "stdafx.h"
#include "ModeS2.h"
#include "version.h"

vector<string>	EQUIPEMENT_CODES = { "H", "L", "E", "G", "W", "Q", "S" };
vector<string>	ICAO_MODES = { "EB", "EL", "ET", "ED", "LF", "EH", "LK", "LO", "LIM", "LIR" };

bool initialLoad { false };

void doInitialLoad(string message)
{
	if (regex_match(message, regex("^([A-z,]+)[|]([A-z,]+)[|]([0-9]{1,3})$")))
	{
		vector<string> data = split(message, '|');
		if (data.size() != 3)
		{
			MessageBox(NULL, "The mode S plugin couldn't parse the server data", "Mode S", MB_OK | MB_ICONWARNING);
			return;
		}

		EQUIPEMENT_CODES = split(data.front(), ',');
		ICAO_MODES = split(data.at(1), ',');

		int new_v = stoi(data.back(), nullptr, 0);

		if (new_v > VERSION_CODE) 
		{
			MessageBox(NULL, "A new version of the mode S plugin is available, please update it", "Mode S", MB_OK | MB_ICONWARNING);
		}
	}	
	else
	{
		MessageBox(NULL, "The mode S plugin couldn't parse the server data", "Mode S", MB_OK | MB_ICONWARNING);
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
	RegisterTagItemFunction("Assign mode S/A squawk", TAG_FUNC_ASSIGNMODEAS);
	
	// Display to reach StartTagFunction from the normal plugin
	RegisterDisplayType("ModeS Function Relay (no display)", false, false, false, false);

	fUpdateString = async(LoadUpdateString, updateUrl);
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
	if (FunctionId == TAG_FUNC_ASSIGNMODES) {
		CFlightPlan FlightPlan = FlightPlanSelectASEL();

		if (!FlightPlan.IsValid())
			return;

		if (!ControllerMyself().IsValid() || !ControllerMyself().IsController())
			return;

		if (!strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V"))
			return;

		string Dest { FlightPlan.GetFlightPlanData().GetDestination() };
		if (isAcModeS(FlightPlan, EQUIPEMENT_CODES) && isApModeS(Dest, ICAO_MODES))
				FlightPlan.GetControllerAssignedData().SetSquawk(mode_s_code);
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

	CFlightPlan FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
	if (!FlightPlan.IsValid() || !FlightPlan.GetTrackingControllerIsMe())
		return;

	if (!strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V"))
		return;

	if (!isAcModeS(FlightPlan, EQUIPEMENT_CODES))
		return;

	auto assr = FlightPlan.GetControllerAssignedData().GetSquawk();
	
	if (!strcmp(mode_s_code, assr))
		return;

	if ((strlen(assr) == 0 ||
		 strcmp(assr, "0000") == 0 ||
		 strcmp(assr, "2000") == 0 ||
		 strcmp(assr, "1200") == 0 ||
		 strcmp(assr, "2200") == 0))
	{
		string destination { FlightPlan.GetFlightPlanData().GetDestination() };
		
		if (isApModeS(destination, ICAO_MODES))
			FlightPlan.GetControllerAssignedData().SetSquawk(mode_s_code);
	}
}

void CModeS::OnTimer(int Counter)
{
	if (!initialLoad && fUpdateString.valid()) {
		if (fUpdateString.wait_for(chrono::milliseconds(0)) == future_status::ready) {
			initialLoad = true;
			try {
				string UpdateString = fUpdateString.get();
				doInitialLoad(UpdateString);
			}
			catch (std::exception& e) {
				MessageBox(NULL, e.what(), "Mode S", MB_OK | MB_ICONWARNING);
			}
			catch (...) {
				MessageBox(NULL, "Unhandled Exception while loading data from server", "Mode S", MB_OK | MB_ICONWARNING);
			}
		}
	}
}

CRadarScreen * CModeS::OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	return new CModeSDisplay(&EQUIPEMENT_CODES, &ICAO_MODES);
}
