#include "stdafx.h"
#include "ESPlugin.h"

CCAMS * gpMyPlugin = NULL;

//---EuroScopePlugInInit-----------------------------------------------

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn ** ppPlugInInstance)
{
	// create the instance
	*ppPlugInInstance = gpMyPlugin = new CCAMS();
}


//---EuroScopePlugInExit-----------------------------------------------

void __declspec (dllexport) EuroScopePlugInExit()
{
	// delete the instance
	delete gpMyPlugin;
}
