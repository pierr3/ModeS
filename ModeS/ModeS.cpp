#include "stdafx.h"
#include "ModeS.h"

CModeS  * gpMyPlugin = NULL;

//---EuroScopePlugInInit-----------------------------------------------

void    __declspec (dllexport)    EuroScopePlugInInit(EuroScopePlugIn::CPlugIn ** ppPlugInInstance)
{
	// create the instance
	*ppPlugInInstance = gpMyPlugin = new CModeS(PluginData());
}


//---EuroScopePlugInExit-----------------------------------------------

void    __declspec (dllexport)    EuroScopePlugInExit(void)
{
	// delete the instance
	delete gpMyPlugin;
}
