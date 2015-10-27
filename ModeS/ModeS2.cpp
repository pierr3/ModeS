#include "stdafx.h"
#include "ModeS2.h"
#include <string>

vector<char>	EQUIPEMENT_CODES = { 'H', 'L', 'E', 'G', 'W', 'Q', 'S' };
vector<char *>	ICAO_MODES = { "EB", "EL", "LS", "ET", "ED", "LF", "EH", "LK", "LO", "LIM, LIR" };

CModeS::CModeS(void):CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
	"Mode S PlugIn",
	"1.0.0e32",
	"Pierre Ferran",
	"GPL v3")
{
	RegisterTagItemType("Transponder type", TAG_ITEM_ISMODES);
	RegisterTagItemType("Mode S: Reported Heading", TAG_ITEM_MODESHDG); 
	RegisterTagItemType("Mode S: Roll Angle", TAG_ITEM_MODESROLLAGL);
	RegisterTagItemType("Mode S: Reported GS", TAG_ITEM_MODESREPGS);

	RegisterTagItemFunction("Assign mode S squawk", TAG_FUNC_ASSIGNMODES);
}

CModeS :: ~CModeS(void)
{
}

void CModeS::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int * pColorCode, COLORREF * pRGB, double * pFontSize)
{

	if (ItemCode == TAG_ITEM_ISMODES) {
		if (!FlightPlan.IsValid())
			return;

		if (isAcModeS(FlightPlan)) {
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

		if (isAcModeS(FlightPlan))
		{
			string rhdg = std::to_string(RadarTarget.GetPosition().GetReportedHeading());
			strcpy_s(sItemString, 16, rhdg.c_str());
		}
	}

	if (ItemCode == TAG_ITEM_MODESROLLAGL)
	{
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
			return;

		if (isAcModeS(FlightPlan))
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

		if (isAcModeS(FlightPlan) && FlightPlan.GetCorrelatedRadarTarget().IsValid())
		{
			strcpy_s(sItemString, 16, std::to_string(RadarTarget.GetPosition().GetReportedGS()).c_str());
		}

	}

}

void CModeS::OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area)
{
	CFlightPlan FlightPlan = FlightPlanSelectASEL();

	if (!FlightPlan.IsValid())
		return;

	if (FunctionId == TAG_FUNC_ASSIGNMODES && isAcModeS(FlightPlan)) {
		const char * Dest = FlightPlan.GetFlightPlanData().GetDestination();

		for (auto &zone : ICAO_MODES) // access by reference to avoid copying
		{
			if (startsWith(zone, Dest)) {
				FlightPlan.GetControllerAssignedData().SetSquawk(mode_s_code);
				break;
			}
		}
	}

}

void CModeS::OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan)
{
	if (!FlightPlan.IsValid())
		return;

	const char * assr = FlightPlan.GetControllerAssignedData().GetSquawk();

	if (strlen(assr) == 0 ||
		strcmp(assr, "0000") == 0 ||
		strcmp(assr, "2000") == 0 ||
		strcmp(assr, "1200") == 0 ||
		strcmp(assr, "2200") == 0)
	{
		if (isAcModeS(FlightPlan))
		{
			for (auto &zone : ICAO_MODES) // access by reference to avoid copying
			{
				if (startsWith(zone, FlightPlan.GetFlightPlanData().GetDestination())) {
					FlightPlan.GetControllerAssignedData().SetSquawk(mode_s_code);
					break;
				}
			}
		}
	}
}

bool CModeS::isAcModeS(CFlightPlan FlightPlan) {
	char transponder_type = FlightPlan.GetFlightPlanData().GetCapibilities();

	for (auto &code : EQUIPEMENT_CODES) {
		if (transponder_type == code)
		{
			return true;
		}
	}
	return false;
}
