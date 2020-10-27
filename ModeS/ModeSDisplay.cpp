#include "stdafx.h"
#include "ModeSDisplay.h"

CModeSDisplay::CModeSDisplay(const CModeSCodes& msc) :
	ModeSCodes(msc)
{};

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
		else {
			if (PendingSquawks.find(FlightPlan.GetCallsign()) == PendingSquawks.end())
				PendingSquawks.insert(std::make_pair(std::string(FlightPlan.GetCallsign()), std::async(LoadWebSquawk)));
		}

	}
}

void CModeSDisplay::OnRefresh(HDC hDC, int Phase)
{
	for (auto it = PendingSquawks.begin(), next_it = it; it != PendingSquawks.end(); it = next_it)
	{
		bool must_delete = false;
		if (it->second.valid() && it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			EuroScopePlugIn::CFlightPlan FlightPlan = GetPlugIn()->FlightPlanSelect(it->first.c_str());
			FlightPlan.GetControllerAssignedData().SetSquawk(it->second.get().c_str());

			must_delete = true;
		}

		++next_it;
		if (must_delete)
		{
			PendingSquawks.erase(it);
		}
	}
}
