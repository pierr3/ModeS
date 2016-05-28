#include "stdafx.h"
#include "ModeS2.h"

CModeS::CModeS(PluginData && pd) :
	CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
			pd.PLUGIN_NAME,
			pd.PLUGIN_VERSION,
			pd.PLUGIN_AUTHOR,
			pd.PLUGIN_LICENSE),
	pluginData(move(pd))
{
	RegisterTagItemType("Transponder type", ItemCodes::TAG_ITEM_ISMODES);
	RegisterTagItemType("Mode S: Reported Heading", ItemCodes::TAG_ITEM_MODESHDG);
	RegisterTagItemType("Mode S: Roll Angle", ItemCodes::TAG_ITEM_MODESROLLAGL);
	RegisterTagItemType("Mode S: Reported GS", ItemCodes::TAG_ITEM_MODESREPGS);

	RegisterTagItemFunction("Assign mode S squawk", ItemCodes::TAG_FUNC_ASSIGNMODEAS);
	RegisterTagItemFunction("Assign mode S/A squawk", ItemCodes::TAG_FUNC_ASSIGNMODEAS);

	// Display to reach StartTagFunction from the normal plugin
	RegisterDisplayType("ModeS Function Relay (no display)", false, false, false, false);

	// Start new thread to get the version file from the server
	fUpdateString = async(LoadUpdateString, pluginData);
}

CModeS::~CModeS()
{}

void CModeS::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int * pColorCode, COLORREF * pRGB, double * pFontSize)
{
	if (ItemCode == ItemCodes::TAG_ITEM_ISMODES) {
		if (!FlightPlan.IsValid())
			return;

		if (msc.isAcModeS(FlightPlan)) {
			strcpy_s(sItemString, 16, "S");
		}
		else {
			strcpy_s(sItemString, 16, "A");
		}
	}

	if (ItemCode == ItemCodes::TAG_ITEM_MODESHDG) {
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (msc.isAcModeS(FlightPlan)) {
			string rhdg = padWithZeros(3, RadarTarget.GetPosition().GetReportedHeading());
			strcpy_s(sItemString, 16, rhdg.c_str());
		}
	}

	if (ItemCode == ItemCodes::TAG_ITEM_MODESROLLAGL) {
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (msc.isAcModeS(FlightPlan)) {
			int rollb = RadarTarget.GetPosition().GetReportedBank();
			string roll = "L";
			if (rollb < 0) {
				roll = "R";
			}
			roll += to_string(abs(rollb));

			strcpy_s(sItemString, 16, roll.c_str());
		}
	}

	if (ItemCode == ItemCodes::TAG_ITEM_MODESREPGS) {
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (msc.isAcModeS(FlightPlan) && FlightPlan.GetCorrelatedRadarTarget().IsValid())
			strcpy_s(sItemString, 16, to_string(RadarTarget.GetPosition().GetReportedGS()).c_str());
	}
}

void CModeS::OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan)
{
	if (!FlightPlan.GetTrackingControllerIsMe())
		ProcessedFlightPlans.erase(remove(ProcessedFlightPlans.begin(), ProcessedFlightPlans.end(), FlightPlan.GetCallsign()),
								   ProcessedFlightPlans.end());
}

void CModeS::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	ProcessedFlightPlans.erase(remove(ProcessedFlightPlans.begin(), ProcessedFlightPlans.end(), FlightPlan.GetCallsign()),
							   ProcessedFlightPlans.end());
}

void CModeS::OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area)
{
	if (FunctionId == ItemCodes::TAG_FUNC_ASSIGNMODES) {
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

	if (RadarTarget.GetPosition().IsFPTrackPosition() ||
		RadarTarget.GetPosition().GetFlightLevel() < 24500)
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

	//Check if we already processed this FlightPlan
	for (auto& pfp : ProcessedFlightPlans)
		if (pfp.compare(FlightPlan.GetCallsign()) == 0)
			return;
	ProcessedFlightPlans.push_back(FlightPlan.GetCallsign());

	auto pssr = RadarTarget.GetPosition().GetSquawk();
	if ((strlen(assr) == 0 ||
		 strcmp(assr, pssr) != 0 ||
		 strcmp(assr, "0000") == 0 ||
		 strcmp(assr, "2000") == 0 ||
		 strcmp(assr, "1200") == 0 ||
		 strcmp(assr, "2200") == 0)) {
		string destination { FlightPlan.GetFlightPlanData().GetDestination() };
		if (msc.isApModeS(destination)) {
#ifndef NDEBUG
			string message { "Code 1000 assigned to " + string { FlightPlan.GetCallsign() } };
			DisplayUserMessage("Mode S", "Debug", message.c_str(), true, false, false, false, false);
#endif
			FlightPlan.GetControllerAssignedData().SetSquawk(::mode_s_code);
		}
	}
}

void CModeS::OnTimer(int Counter)
{
	if (fUpdateString.valid()) {
		if (fUpdateString.wait_for(0ms) == future_status::ready) {
			try {
				DoInitialLoad(fUpdateString.get());
			}
			catch (modesexception &e) {
				MessageBox(NULL, e.what(), "Mode S", MB_OK | e.icon());
			}
			catch (exception &e) {
				MessageBox(NULL, e.what(), "Mode S", MB_OK | MB_ICONERROR);
			}
			fUpdateString = future<string>();
		}
	}
}

CRadarScreen * CModeS::OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	return new CModeSDisplay(msc);
}

void CModeS::DoInitialLoad(const string & message)
{
	if (regex_match(message, regex("^([A-z,]+)[|]([A-z,]+)[|]([0-9]{1,3})$"))) {
		vector<string> data = split(message, '|');
		if (data.size() != 3)
			throw error { "The mode S plugin couldn't parse the server data" };

		msc.SetEquipementCodes(split(data.front(), ','));
		msc.SetICAOModeS(split(data.at(1), ','));

		int new_v = stoi(data.back(), nullptr, 0);
		if (new_v > pluginData.VERSION_CODE)
			throw warning { "A new version of the mode S plugin is available, please update it" };
	}
	else
		throw error { "The mode S plugin couldn't parse the server data" };
}
