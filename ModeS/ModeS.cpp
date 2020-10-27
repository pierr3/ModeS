#include "stdafx.h"
#include "ModeS.h"

CModeS * gpMyPlugin = NULL;

//---EuroScopePlugInInit-----------------------------------------------

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn ** ppPlugInInstance)
{
	// create the instance
	*ppPlugInInstance = gpMyPlugin = new CModeS();
}


//---EuroScopePlugInExit-----------------------------------------------

void __declspec (dllexport) EuroScopePlugInExit()
{
	// delete the instance
	delete gpMyPlugin;
}
