#include "stdafx.h"
#include "ModeS2.h"

CModeS::CModeS():CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
						 ::PLUGIN_NAME,
						 ::PLUGIN_VERSION,
						 ::PLUGIN_AUTHOR,
						 ::PLUGIN_LICENSE)
{
	RegisterTagItemType("Transponder type", TAG_ITEM_ISMODES);
	RegisterTagItemType("Mode S: Reported Heading", TAG_ITEM_MODESHDG); 
	RegisterTagItemType("Mode S: Roll Angle", TAG_ITEM_MODESROLLAGL);
	RegisterTagItemType("Mode S: Reported GS", TAG_ITEM_MODESREPGS);

	RegisterTagItemFunction("Assign mode S squawk", TAG_FUNC_ASSIGNMODES);
	RegisterTagItemFunction("Assign mode S/A squawk", TAG_FUNC_ASSIGNMODEAS);
	
	// Display to reach StartTagFunction from the normal plugin
	RegisterDisplayType("ModeS Function Relay (no display)", false, false, false, false);

	fUpdateString = async(LoadUpdateString);
}

CModeS::~CModeS()
{
}

void CModeS::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int * pColorCode, COLORREF * pRGB, double * pFontSize)
{

	if (ItemCode == TAG_ITEM_ISMODES) {
		if (!FlightPlan.IsValid())
			return;

		if (msc.isAcModeS(FlightPlan)) {
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

		if (msc.isAcModeS(FlightPlan)) {
			string rhdg = padWithZeros(3, RadarTarget.GetPosition().GetReportedHeading());
			strcpy_s(sItemString, 16, rhdg.c_str());
		}
	}

	if (ItemCode == TAG_ITEM_MODESROLLAGL)
	{
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (msc.isAcModeS(FlightPlan)) {
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

		if (msc.isAcModeS(FlightPlan) && FlightPlan.GetCorrelatedRadarTarget().IsValid())
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
		if (msc.isAcModeS(FlightPlan) && msc.isApModeS(Dest))
				FlightPlan.GetControllerAssignedData().SetSquawk(::mode_s_code);
	}
}

void CModeS::OnRadarTargetPositionUpdate(CRadarTarget RadarTarget)
{
	if (!ControllerMyself().IsValid() || !ControllerMyself().IsController())
		return;

	if (RadarTarget.GetPosition().IsFPTrackPosition())
		return;

	if (RadarTarget.GetPosition().GetFlightLevel() < 24500)
		return;

	CFlightPlan FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
	if (!FlightPlan.IsValid() || !FlightPlan.GetTrackingControllerIsMe())
		return;

	if (strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V") == 0)
		return;

	if (!msc.isAcModeS(FlightPlan))
		return;

	auto assr = FlightPlan.GetControllerAssignedData().GetSquawk();
	
	if (strcmp(::mode_s_code, assr) == 0)
		return;

	if ((strlen(assr) == 0 ||
		 strcmp(assr, "0000") == 0 ||
		 strcmp(assr, "2000") == 0 ||
		 strcmp(assr, "1200") == 0 ||
		 strcmp(assr, "2200") == 0))
	{
		string destination { FlightPlan.GetFlightPlanData().GetDestination() };
		if (msc.isApModeS(destination)) {
			string message { "Code 1000 assigned to " };
			message.append(FlightPlan.GetCallsign());
			DisplayUserMessage("Mode S", "Debug", message.c_str(), true, false, false, false, false);
			FlightPlan.GetControllerAssignedData().SetSquawk(::mode_s_code);
		}
	}
}

void CModeS::OnTimer(int Counter)
{
	if (fUpdateString.valid()) {
		if (fUpdateString.wait_for(chrono::milliseconds(0)) == future_status::ready) {
			try {
				string UpdateString = fUpdateString.get();
				DoInitialLoad(UpdateString);
			}
			catch (std::exception& e) {
				MessageBox(NULL, e.what(), "Mode S", MB_OK | MB_ICONERROR);
			}
			catch (std::string &s) {
				MessageBox(NULL, s.c_str(), "Mode S", MB_OK | MB_ICONWARNING);
			}
			fUpdateString = future<string>();
		}
	}
}

CRadarScreen * CModeS::OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	return new CModeSDisplay(msc);
}

void CModeS::DoInitialLoad(string & message)
{
	if (regex_match(message, regex("^([A-z,]+)[|]([A-z,]+)[|]([0-9]{1,3})$")))
	{
		vector<string> data = split(message, '|');
		if (data.size() != 3)
			throw std::exception { "The mode S plugin couldn't parse the server data" };

		msc.SetEquipementCodes(split(data.front(), ','));
		msc.SetICAOModeS(split(data.at(1), ','));

		int new_v = stoi(data.back(), nullptr, 0);
		if (new_v > VERSION_CODE)
			throw std::string { "A new version of the mode S plugin is available, please update it" };
	}
	else
		throw std::exception { "The mode S plugin couldn't parse the server data" };
}
