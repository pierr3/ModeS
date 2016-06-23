#include "stdafx.h"
#include "ModeS2.h"

CModeS::CModeS(PluginData pd) :
	CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
			pd.PLUGIN_NAME,
			pd.PLUGIN_VERSION,
			pd.PLUGIN_AUTHOR,
			pd.PLUGIN_LICENSE),
	pluginData(pd)
{
	RegisterTagItemType("Transponder type", ItemCodes::TAG_ITEM_ISMODES);
	RegisterTagItemType("Mode S: Reported Heading", ItemCodes::TAG_ITEM_MODESHDG);
	RegisterTagItemType("Mode S: Roll Angle", ItemCodes::TAG_ITEM_MODESROLLAGL);
	RegisterTagItemType("Mode S: Reported GS", ItemCodes::TAG_ITEM_MODESREPGS);

	RegisterTagItemFunction("Assign mode S squawk", ItemCodes::TAG_FUNC_ASSIGNMODES);
	RegisterTagItemFunction("Assign mode S/A squawk", ItemCodes::TAG_FUNC_ASSIGNMODEAS);

	// Display to reach StartTagFunction from the normal plugin
	RegisterDisplayType("ModeS Function Relay (no display)", false, false, false, false);

	// Start new thread to get the version file from the server
	fUpdateString = async(LoadUpdateString, pd);
}

CModeS::~CModeS()
{}

void CModeS::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int * pColorCode, COLORREF * pRGB, double * pFontSize)
{
	if (ItemCode == ItemCodes::TAG_ITEM_ISMODES) {
		if (!FlightPlan.IsValid())
			return;

		if (msc.isAcModeS(FlightPlan))
			strcpy_s(sItemString, 16, "S");
		else
			strcpy_s(sItemString, 16, "A");
	}

	else if (ItemCode == ItemCodes::TAG_ITEM_MODESHDG) {
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (msc.isAcModeS(FlightPlan))
			snprintf(sItemString, 16, "%03i", RadarTarget.GetPosition().GetReportedHeading());
	}

	else if (ItemCode == ItemCodes::TAG_ITEM_MODESROLLAGL) {
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (msc.isAcModeS(FlightPlan)) {
			auto rollb = RadarTarget.GetPosition().GetReportedBank();
			snprintf(sItemString, 16, "%c%i", rollb < 0 ? 'R' : 'L', abs(rollb));
		}
	}

	else if (ItemCode == ItemCodes::TAG_ITEM_MODESREPGS) {
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
		if (!ControllerMyself().IsValid() || !ControllerMyself().IsController())
			return;

		CFlightPlan FlightPlan = FlightPlanSelectASEL();
		if (!FlightPlan.IsValid())
			return;

		if (!strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V"))
			return;

		if (msc.isAcModeS(FlightPlan) && msc.isApModeS(FlightPlan.GetFlightPlanData().GetDestination()))
			FlightPlan.GetControllerAssignedData().SetSquawk(::mode_s_code);
	}
}

void CModeS::OnTimer(int Counter)
{
	if (fUpdateString.valid() && fUpdateString.wait_for(0ms) == future_status::ready)
		DoInitialLoad(fUpdateString);

	if (ControllerMyself().IsValid() && ControllerMyself().IsController())
		AutoAssignMSCC();
}

CRadarScreen * CModeS::OnRadarScreenCreated(const char * sDisplayName,
											bool NeedRadarContent,
											bool GeoReferenced,
											bool CanBeSaved,
											bool CanBeCreated)
{
	return new CModeSDisplay(msc);
}

void CModeS::AutoAssignMSCC()
{
	for (CRadarTarget RadarTarget = RadarTargetSelectFirst();
		 RadarTarget.IsValid();
		 RadarTarget = RadarTargetSelectNext(RadarTarget)) {

		if (RadarTarget.GetPosition().IsFPTrackPosition() ||
			RadarTarget.GetPosition().GetFlightLevel() < 24500)
			return;

		CFlightPlan FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
		if (!FlightPlan.IsValid() || !FlightPlan.GetTrackingControllerIsMe())
			return;

		//Check if FlightPlan is already processed
		if (IsFlightPlanProcessed(FlightPlan))
			return;

		if (strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V") == 0)
			return;

		if (!msc.isAcModeS(FlightPlan) ||
			!msc.isApModeS(FlightPlan.GetFlightPlanData().GetDestination()))
			return;

		auto assr = FlightPlan.GetControllerAssignedData().GetSquawk();
		if (strcmp(::mode_s_code, assr) == 0)
			return;

		auto pssr = RadarTarget.GetPosition().GetSquawk();
		if ((strlen(assr) == 0 ||
			 strcmp(assr, pssr) != 0 ||
			 strcmp(assr, "0000") == 0 ||
			 strcmp(assr, "2000") == 0 ||
			 strcmp(assr, "1200") == 0 ||
			 strcmp(assr, "2200") == 0)) {
			FlightPlan.GetControllerAssignedData().SetSquawk(::mode_s_code);

			// Debug message, to be removed
			string message { "Code 1000 assigned to " + string { FlightPlan.GetCallsign() } };
			DisplayUserMessage("Mode S", "Debug", message.c_str(), true, false, false, false, false);
		}
	}
}

void CModeS::DoInitialLoad(future<string> & fmessage)
{
	try {
		string message = fmessage.get();
		regex update_string { "^([A-z,]+)[|]([A-z,]+)[|]([0-9]{1,3})$" };
		smatch match;
		if (regex_match(message, match, update_string)) {
			msc.SetEquipementCodes(split(match[1].str(), ','));
			msc.SetICAOModeS(split(match[2].str(), ','));

			int new_v = stoi(match[3].str(), nullptr, 0);
			if (new_v > pluginData.VERSION_CODE)
				throw warning { "A new version of the mode S plugin is available, please update it" };
		}
		else
			throw error { "The mode S plugin couldn't parse the server data" };
	}
	catch (modesexception & e) {
		MessageBox(NULL, e.what(), "Mode S", MB_OK | e.icon());
	}
	catch (exception & e) {
		MessageBox(NULL, e.what(), "Mode S", MB_OK | MB_ICONERROR);
	}
	fmessage = future<string>();
}

inline bool CModeS::IsFlightPlanProcessed(CFlightPlan & FlightPlan)
{
	string callsign { FlightPlan.GetCallsign() };
	for (auto &pfp : ProcessedFlightPlans)
		if (pfp.compare(callsign) == 0)
			return true;

	ProcessedFlightPlans.push_back(FlightPlan.GetCallsign());
	return false;
}
