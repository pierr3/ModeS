#include "stdafx.h"
#include "ModeSDisplay.h"

CModeSDisplay::CModeSDisplay(const CModeSCodes & msc) :
	ModeSCodes(msc)
{}

void CModeSDisplay::OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area)
{
	if (FunctionId == ItemCodes::TAG_FUNC_ASSIGNMODEAS) {
		if (!GetPlugIn()->ControllerMyself().IsValid() || !GetPlugIn()->ControllerMyself().IsController())
			return;

		EuroScopePlugIn::CFlightPlan FlightPlan = GetPlugIn()->FlightPlanSelectASEL();
		if (!FlightPlan.IsValid())
			return;

		if (!strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V")) {
			StartTagFunction(FlightPlan.GetCallsign(), nullptr, 0, "", nullptr, EuroScopePlugIn::TAG_ITEM_FUNCTION_SQUAWK_POPUP, Pt, Area);
			return;
		}

		if (ModeSCodes.isAcModeS(FlightPlan) && ModeSCodes.isApModeS(FlightPlan.GetFlightPlanData().GetDestination()))
			FlightPlan.GetControllerAssignedData().SetSquawk(::mode_s_code);
		else
			StartTagFunction(FlightPlan.GetCallsign(), nullptr, 0, "", nullptr, EuroScopePlugIn::TAG_ITEM_FUNCTION_SQUAWK_POPUP, Pt, Area);
	}
}
