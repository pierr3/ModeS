#pragma once
#include <EuroScopePlugIn.h>
#include <vector>

using namespace std;
using namespace EuroScopePlugIn;

class CModeS :
	public EuroScopePlugIn::CPlugIn
{
public:
	CModeS();
	~CModeS();

	const int TAG_ITEM_ISMODES = 501;
	const int TAG_ITEM_MODESHDG = 502;
	const int TAG_ITEM_MODESROLLAGL = 503;
	const int TAG_ITEM_MODESREPGS = 504;

	const int TAG_FUNC_ASSIGNMODES = 869;

	const char* mode_s_code = "1000";

	void OnGetTagItem(CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget,
		int ItemCode,
		int TagData,
		char sItemString[16],
		int * pColorCode,
		COLORREF * pRGB,
		double * pFontSize);

	void OnFunctionCall(int FunctionId,
		const char * sItemString,
		POINT Pt,
		RECT Area);

	virtual void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);

	bool isAcModeS(CFlightPlan FlightPlan);
	inline bool startsWith(const char *pre, const char *str)
	{
		size_t lenpre = strlen(pre),
			lenstr = strlen(str);
		return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
	}


};

