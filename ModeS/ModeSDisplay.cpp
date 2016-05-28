#include "stdafx.h"
#include "ModeSDisplay.h"

CModeSDisplay::CModeSDisplay(const CModeSCodes& msc)
	: msc(msc)
{}

void CModeSDisplay::OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area)
{
	if (FunctionId == ItemCodes::TAG_FUNC_ASSIGNMODEAS) {
		CFlightPlan FlightPlan = GetPlugIn()->FlightPlanSelectASEL();

		if (!FlightPlan.IsValid())
			return;

		if (!GetPlugIn()->ControllerMyself().IsValid() || !GetPlugIn()->ControllerMyself().IsController())
			return;

		if (!strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V"))
			return;

		string Dest { FlightPlan.GetFlightPlanData().GetDestination() };
		if (msc.isAcModeS(FlightPlan) && msc.isApModeS(Dest))
			FlightPlan.GetControllerAssignedData().SetSquawk(::mode_s_code);
		else
			StartTagFunction(FlightPlan.GetCallsign(), nullptr, 0, "", nullptr, TAG_ITEM_FUNCTION_SQUAWK_POPUP, Pt, Area);
	}
}
