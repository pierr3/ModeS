#include "stdafx.h"
#include "ModeS2.h"


const string updateUrl = "http://www.cherryx.de/modes/modes.txt";
const int VERSION_CODE = 902;
const char PLUGIN_VERSION[] = "1.3.3e32";

vector<string>	EQUIPEMENT_CODES = { "H", "L", "E", "G", "W", "Q", "S" };
vector<string>	ICAO_MODES = { "EB", "EL", "LS", "ET", "ED", "LF", "EH", "LK", "LO", "LIM", "LIR" };

HttpHelper *httpHelper = NULL;
bool initData = true;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

void doInitialLoad(void * arg)
{
	string message;
	message.assign(httpHelper->downloadStringFromURL(updateUrl));

	// Message format is {equip_codes}|{icao_modes}|{version}
	if (regex_match(message, std::regex("^([A-z,]+)[|]([A-z,]+)[|]([0-9]{1,3})$")))
	{
		vector<string> data = split(message, '|');

		EQUIPEMENT_CODES = split(data.front(), ',');
		ICAO_MODES = split(data.at(1), ',');

		int new_v = std::stoi(data.back(), nullptr, 0);

		if (new_v > VERSION_CODE)
		{
			AFX_MANAGE_STATE(AfxGetStaticModuleState());
				AfxMessageBox("A new version of the mode S plugin is available, please update it.");
		}
	} else
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
			AfxMessageBox("The mode S plugin couldn't parse the server data, please update the plugin.");
	}
}

CModeS::CModeS():CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
	"Mode S PlugIn",
	PLUGIN_VERSION,
	"Pierre Ferran / Oliver Grützmann",
	"GPL v3")
{
	if (httpHelper == NULL)
		httpHelper = new HttpHelper();

	RegisterTagItemType("Transponder type", TAG_ITEM_ISMODES);
	RegisterTagItemType("Mode S: Reported Heading", TAG_ITEM_MODESHDG); 
	RegisterTagItemType("Mode S: Roll Angle", TAG_ITEM_MODESROLLAGL);
	RegisterTagItemType("Mode S: Reported GS", TAG_ITEM_MODESREPGS);

	RegisterTagItemFunction("Assign mode S squawk", TAG_FUNC_ASSIGNMODES);
	
	// Display to reach StartTagFunction from the normal plugin
	RegisterDisplayType("ModeS Function Relay (no display)", false, false, false, false);
	
	// Function to assign code 1000 or open the assign squawk popup.
	RegisterTagItemFunction("Assign mode S/A squawk", TAG_FUNC_ASSIGNMODEAS);

	delayedStart = clock();
}

CModeS::~CModeS()
{
	delete httpHelper;
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

		if (isAcModeS(FlightPlan, EQUIPEMENT_CODES))
		{
			string rhdg = padWithZeros(3, RadarTarget.GetPosition().GetReportedHeading());
			strcpy_s(sItemString, 16, rhdg.c_str());
		}
	}

	if (ItemCode == TAG_ITEM_MODESROLLAGL)
	{
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (isAcModeS(FlightPlan, EQUIPEMENT_CODES))
		{
			int rollb = RadarTarget.GetPosition().GetReportedBank();
			string roll = "L";
			if (rollb < 0)
			{
				roll = "R";
			}
			roll += std::to_string(abs(rollb));

			strcpy_s(sItemString, 16, roll.c_str());
		}
	}

	if (ItemCode == TAG_ITEM_MODESREPGS)
	{
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

//void CModeS::OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan)
//{
	// don't use for now
	//if (!FlightPlan.IsValid())
	//	return;

	//if (ControllerMyself().IsValid() && ControllerMyself().IsController()) {

	//	if (((clock() - delayedStart) / CLOCKS_PER_SEC) < 15)
	//		return;

	//	// Here we assign squawk 1000 to ground aircrafts

	//	if (FlightPlan.GetFlightPlanData().IsAmended())
	//		return;

	//	const char * assr = FlightPlan.GetControllerAssignedData().GetSquawk();

	//	CRadarTarget rt = RadarTargetSelect(FlightPlan.GetCallsign());
	//	
	//	if (!rt.IsValid() || !rt.GetPosition().IsValid())
	//		return;

	//	if (rt.GetPosition().GetReportedGS() > 20)
	//		return;

	//	if (strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V") == 0)
	//		return;

	//	string origin { FlightPlan.GetFlightPlanData().GetOrigin() };
	//	string destination { FlightPlan.GetFlightPlanData().GetDestination() };
	//	string controllerCallsign { ControllerMyself().GetCallsign() };

	//	if (controllerCallsign.compare(0, 4, origin, 0, 4))
	//		return;
	//		
	//	if (isAcModeS(FlightPlan) && 
	//		isApModeS(destination) && 
	//		isApModeS(origin))
	//		FlightPlan.GetControllerAssignedData().SetSquawk(mode_s_code);
	//}
	//else {
	//	delayedStart = clock();
	//}	
//}

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
	if (initData) {
		DisplayUserMessage("Message", "Mode S", "Downloading configuration...", true, false, false, false, false);
		// Download the configuration
		_beginthread(doInitialLoad, 0, NULL);
		initData = false;
	}
}

CRadarScreen * CModeS::OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	return new CModeSDisplay(&EQUIPEMENT_CODES, &ICAO_MODES);
}
