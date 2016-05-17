#include "stdafx.h"
#include "ModeSDisplay.h"

CModeSDisplay::CModeSDisplay(vector<string>* EQUIPEMENT_CODES, vector<string>*	ICAO_MODES)
	: EQUIPEMENT_CODES(EQUIPEMENT_CODES), ICAO_MODES(ICAO_MODES)
{}

void CModeSDisplay::OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area)
{
	if (FunctionId == TAG_FUNC_ASSIGNMODEAS) {
		CFlightPlan FlightPlan = GetPlugIn()->FlightPlanSelectASEL();

		if (!FlightPlan.IsValid())
			return;

		if (!GetPlugIn()->ControllerMyself().IsValid() || !GetPlugIn()->ControllerMyself().IsController())
			return;

		if (!strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V"))
			return;

		string Dest { FlightPlan.GetFlightPlanData().GetDestination() };
		if (isAcModeS(FlightPlan, *EQUIPEMENT_CODES) && isApModeS(Dest, *ICAO_MODES))
			FlightPlan.GetControllerAssignedData().SetSquawk(mode_s_code);
		else
			StartTagFunction(FlightPlan.GetCallsign(), nullptr, 0, "", nullptr, TAG_ITEM_FUNCTION_SQUAWK_POPUP, Pt, Area);
	}
}
