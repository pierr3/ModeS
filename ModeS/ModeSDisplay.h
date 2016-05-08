#pragma once
#include "EuroScopePlugIn.h"

// Class for calling Tag Item Functions, to be able to open the standard
// squawk assignment menu

class CModeSDisplay :
	public EuroScopePlugIn::CRadarScreen
{
public:
	CModeSDisplay(){}
	virtual ~CModeSDisplay(){}
	void OnAsrContentToBeClosed(){}
};

