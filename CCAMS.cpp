#include "stdafx.h"
#include "CCAMS.h"

CCAMS::CCAMS(const EquipmentCodes&& ec, const SquawkCodes&& sc) : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
	MY_PLUGIN_NAME,
	MY_PLUGIN_VERSION,
	MY_PLUGIN_DEVELOPER,
	MY_PLUGIN_COPYRIGHT),
	EquipmentCodesFAA(ec.FAA),
	EquipmentCodesICAO(ec.ICAO_MODE_S),
	EquipmentCodesICAOEHS(ec.ICAO_EHS),
	ModeSAirports(MODE_S_AIRPORTS),
	squawkModeS(sc.MODE_S),
	squawkVFR(sc.VFR)
{
	string DisplayMsg { "Version " + string { MY_PLUGIN_VERSION } + " loaded" };
#ifdef _DEBUG
	DisplayUserMessage(MY_PLUGIN_NAME, "Initialisation", ("DEBUG " + DisplayMsg).c_str(), true, false, false, false, false);
#else
	DisplayUserMessage(MY_PLUGIN_NAME, "Initialisation", DisplayMsg.c_str(), true, false, false, false, false);
#endif

	RegisterTagItemType("Transponder Type", ItemCodes::TAG_ITEM_ISMODES);
	RegisterTagItemType("EHS Heading", ItemCodes::TAG_ITEM_EHS_HDG);
	RegisterTagItemType("EHS Roll Angle", ItemCodes::TAG_ITEM_EHS_ROLL);
	RegisterTagItemType("EHS GS", ItemCodes::TAG_ITEM_EHS_GS);
	RegisterTagItemType("Mode S squawk error", ItemCodes::TAG_ITEM_ERROR_MODES_USE);
	RegisterTagItemType("Assigned squawk", ItemCodes::TAG_ITEM_SQUAWK);

	RegisterTagItemFunction("Auto assign squawk", ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_AUTO);
	RegisterTagItemFunction("Open SQUAWK assign popup", ItemCodes::TAG_FUNC_SQUAWK_POPUP);

	FpListEHS = RegisterFpList("Mode S EHS");
	if (FpListEHS.GetColumnNumber() == 0)
	{
		FpListEHS.AddColumnDefinition("C/S", 8, false, NULL, TAG_ITEM_TYPE_CALLSIGN, NULL, TAG_ITEM_FUNCTION_OPEN_FP_DIALOG, NULL, NULL);
		FpListEHS.AddColumnDefinition("HDG", 5, true, MY_PLUGIN_NAME, ItemCodes::TAG_ITEM_EHS_HDG, NULL, NULL, NULL, NULL);
		FpListEHS.AddColumnDefinition("Roll", 5, true, MY_PLUGIN_NAME, ItemCodes::TAG_ITEM_EHS_ROLL, NULL, NULL, NULL, NULL);
		FpListEHS.AddColumnDefinition("GS", 4, true, MY_PLUGIN_NAME, ItemCodes::TAG_ITEM_EHS_GS, NULL, NULL, NULL, NULL);
	}

	// Start new thread to get the version file from the server
	fUpdateString = async(LoadUpdateString);

	// Set default setting values
	acceptEquipmentICAO = true;
	acceptEquipmentFAA = true;
	pluginVersionRestricted = true;
#ifdef _DEBUG
	autoAssign = true;
#else
	autoAssign = false;
#endif
	APTcodeMaxGS = 50;
	APTcodeMaxDist = 3;
	
	// Overwrite setting values by plugin settings, if available
	try
	{
		const char* cstrSetting = GetDataFromSettings("codeVFR");
		if (cstrSetting != NULL)
		{
			if (regex_search(cstrSetting, std::regex("[0-7]{4}")))
			{
				squawkVFR = cstrSetting;
			}
		}

		cstrSetting = GetDataFromSettings("acceptFPLformatICAO");
		if (cstrSetting != NULL)
		{
			if (strcmp(cstrSetting, "0") == 0)
			{
				acceptEquipmentICAO = false;
			}
		}

		cstrSetting = GetDataFromSettings("acceptFPLformatFAA");
		if (cstrSetting != NULL)
		{
			if (strcmp(cstrSetting, "0") == 0)
			{
				acceptEquipmentFAA = false;
			}
		}

		cstrSetting = GetDataFromSettings("AutoAssign");
		if (cstrSetting != NULL)
		{
			if (strcmp(cstrSetting, "0") == 0)
			{
				autoAssign = false;
			}
			else if (strcmp(cstrSetting, "1") == 0)
			{
				autoAssign = true;
			}
		}
	}
	catch (std::runtime_error const& e)
	{
		DisplayUserMessage(MY_PLUGIN_NAME, "Plugin Error", (string("Error: ") + e.what()).c_str(), true, true, true, true, true);
	}
	catch (...)
	{
		DisplayUserMessage(MY_PLUGIN_NAME, "Plugin Error", ("Unexpected error: " + std::to_string(GetLastError())).c_str(), true, true, true, true, true);
	}

}

CCAMS::~CCAMS()
{}

bool CCAMS::OnCompileCommand(const char* command)
{
	string commandString(command);
	cmatch matches;

	if (_stricmp(command, ".help") == 0)
	{
		DisplayUserMessage("HELP", "HELP", ".HELP CCAMS | Centralised code assignment and management system Help", true, true, true, true, false);
		return NULL;
	}
	else if (_stricmp(command, ".help ccams") == 0)
	{
		// Display HELP
		DisplayUserMessage("HELP", MY_PLUGIN_NAME, ".CCAMS EHSLIST | Displays the flight plan list with EHS values of the currently selected aircraft.", true, true, true, true, false);
		DisplayUserMessage("HELP", MY_PLUGIN_NAME, ".CCAMS AUTO | Activates or deactivates automatic code assignment.", true, true, true, true, false);
#ifdef _DEBUG
		DisplayUserMessage("HELP", MY_PLUGIN_NAME, ".CCAMS RESET | Clears the list of flight plans which have been determined no longer applicable for automatic code assignment.", true, true, true, true, false);
		DisplayUserMessage("HELP", MY_PLUGIN_NAME, ".CCAMS [CALL SIGN] | Displays tracking and controller information for a specific flight (to support debugging of automatic code assignment).", true, true, true, true, false);
#endif
		return true;
	}
	else if (regex_match(command, matches, regex("\\.ccams\\s+(\\w+)", regex::icase)))
	{
		return PluginCommands(matches[1].str().c_str());
	}

	return false;
}

bool CCAMS::PluginCommands(const char* Command)
{
	//string sCommand(Command);
	if (_stricmp(Command, "ehslist") == 0)
	{
		FpListEHS.ShowFpList(true);
		return true;
	}
	else if (_stricmp(Command, "auto") == 0)
	{
		if (pluginVersionRestricted)
		{
			DisplayUserMessage(MY_PLUGIN_NAME, "Error", "Your plugin version is not up-to-date and the automatic code assignment therefore not available.", true, true, false, false, false);
		}
		else if (autoAssign)
		{
			autoAssign = false;
			SaveDataToSettings("AutoAssign", "Automatic assignment of squawk codes", "0");
			DisplayUserMessage(MY_PLUGIN_NAME, "Setting changed", "Automatic code assignment disabled", true, true, false, false, false);
		}
		else
		{
			autoAssign = true;
			SaveDataToSettings("AutoAssign", "Automatic assignment of squawk codes", "1");
			DisplayUserMessage(MY_PLUGIN_NAME, "Setting changed", "Automatic code assignment enabled", true, true, false, false, false);
		}
		return true;
	}
#ifdef _DEBUG
	else if (_stricmp(Command, "reset") == 0)
	{
		ProcessedFlightPlans.clear();
		return true;
	}
	else
	{
		for (CFlightPlan FlightPlan = FlightPlanSelectFirst(); FlightPlan.IsValid();
			FlightPlan = FlightPlanSelectNext(FlightPlan))
		{
			if (_stricmp(Command, FlightPlan.GetCallsign()) == 0)
			{
				string DisplayMsg = string{ FlightPlan.GetCallsign() } + ": Tracking Controller Len '" + to_string(strlen(FlightPlan.GetTrackingControllerCallsign())) + "', Minutes to entry " + to_string(FlightPlan.GetSectorEntryMinutes()) + ", TrackingMe: " + to_string(FlightPlan.GetTrackingControllerIsMe());
				DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
				CFlightPlanPositionPredictions Pos = FlightPlan.GetPositionPredictions();
				int min = 0;
				while (min < min(Pos.GetPointsNumber(), 15))
				{
					if (_stricmp(FlightPlan.GetPositionPredictions().GetControllerId(min),"--") != 0)
						break;

					min++;
				}
					
				DisplayMsg = string{ FlightPlan.GetCallsign() } + ": Next Controller '" + FlightPlan.GetPositionPredictions().GetControllerId(min) + "' in " + to_string(min) + " Minutes ";
				DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);

				AssignAutoSquawk(FlightPlan);
				return true;
			}
		}
	}
#endif
	return false;
}

void CCAMS::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int * pColorCode, COLORREF * pRGB, double * pFontSize)
{
	if (!FlightPlan.IsValid())
		return;

	if (ItemCode == ItemCodes::TAG_ITEM_ISMODES)
	{
		if (isAcModeS(FlightPlan))
			strcpy_s(sItemString, 16, "S");
		else
			strcpy_s(sItemString, 16, "A");
	}
	else if (ItemCode == TAG_ITEM_TYPE_SQUAWK && FlightPlan.GetTrackingControllerIsMe())
	{
		//AssignAutoSquawk(FlightPlan);
	}
	else
	{
		if (!RadarTarget.IsValid())
			return;

		if (ItemCode == ItemCodes::TAG_ITEM_EHS_HDG)
		{
			if (isEHS(FlightPlan))
			{
				//strncpy(sItemString, to_string(RadarTarget.GetPosition().GetReportedHeading()).c_str(), 16);
				sprintf_s(sItemString, 16, "%03i°", RadarTarget.GetPosition().GetReportedHeading() % 360);
	#ifdef _DEBUG
				string DisplayMsg{ to_string(RadarTarget.GetPosition().GetReportedHeading()) };
				//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
	#endif
			}
			else
			{
				strcpy_s(sItemString, 16, "N/A");
			}
		}
		else if (ItemCode == ItemCodes::TAG_ITEM_EHS_ROLL)
		{
			if (isEHS(FlightPlan))
			{
				auto rollb = RadarTarget.GetPosition().GetReportedBank();

				if (rollb == 0)
				{
					sprintf_s(sItemString, 16, "%i", abs(rollb));
				}
				else
				{
					sprintf_s(sItemString, 16, "%c%i°", rollb < 0 ? 'R' : 'L', abs(rollb));
				}
	#ifdef _DEBUG
				string DisplayMsg{ to_string(abs(rollb)) };
				//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
	#endif
			}
			else
			{
				strcpy_s(sItemString, 16, "N/A");
			}
		}
		else if (ItemCode == ItemCodes::TAG_ITEM_EHS_GS)
		{
			if (isEHS(FlightPlan) && FlightPlan.GetCorrelatedRadarTarget().IsValid())
			{
				snprintf(sItemString, 16, "%03i", RadarTarget.GetPosition().GetReportedGS());
	#ifdef _DEBUG
				string DisplayMsg{ to_string(RadarTarget.GetPosition().GetReportedGS()) };
				//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
	#endif
			}
			else
			{
				strcpy_s(sItemString, 16, "N/A");
			}
		}

		else if (ItemCode == ItemCodes::TAG_ITEM_ERROR_MODES_USE)
		{
			if (isEligibleSquawkModeS(FlightPlan)) return;

			auto assr = RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk();
			auto pssr = RadarTarget.GetPosition().GetSquawk();
			if (strcmp(assr, squawkModeS) != 0 &&
				strcmp(pssr, squawkModeS) != 0)
				return;

			*pColorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
			strcpy_s(sItemString, 16, "MSSQ");
		}
		else if (ItemCode == ItemCodes::TAG_ITEM_SQUAWK)
		{
			auto assr = RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk();
			auto pssr = RadarTarget.GetPosition().GetSquawk();

			if (!isEligibleSquawkModeS(FlightPlan) && (strcmp(assr, squawkModeS) == 0 || (strcmp(pssr, squawkModeS) == 0 && strlen(assr) == 0)))
			{
				// mode S code assigned, but not eligible
				*pColorCode = EuroScopePlugIn::TAG_COLOR_REDUNDANT;
			}
			else if(strcmp(assr, pssr) != 0)
			{
				// assigned squawk is not set
				*pColorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
			}
			strcpy_s(sItemString, 16, assr);
		}

	}
}

void CCAMS::OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan)
{
	if (FlightPlan.GetTrackingControllerIsMe())
	{
		if (autoAssign)
		{
#ifdef _DEBUG
			string DisplayMsg = string{ FlightPlan.GetCallsign() } + " is processed for automatic squawk assignment (due to flight plan update and controller is tracking)";
			DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, true, false, false, false);
#endif
			AssignAutoSquawk(FlightPlan);
		}
	}
	else
	{
		ProcessedFlightPlans.erase(remove(ProcessedFlightPlans.begin(), ProcessedFlightPlans.end(), FlightPlan.GetCallsign()), ProcessedFlightPlans.end());
	}
}

void CCAMS::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	ProcessedFlightPlans.erase(remove(ProcessedFlightPlans.begin(), ProcessedFlightPlans.end(), FlightPlan.GetCallsign()),
							   ProcessedFlightPlans.end());

	FpListEHS.RemoveFpFromTheList(FlightPlan);
}

void CCAMS::OnRefreshFpListContent(CFlightPlanList AcList)
{

	if (ControllerMyself().IsValid() && RadarTargetSelectASEL().IsValid())
	{
#ifdef _DEBUG
		string DisplayMsg{ "The following call sign was identified to be added to the EHS Mode S list: " + string { FlightPlanSelectASEL().GetCallsign() } };
		//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
		for (CFlightPlan FP = FlightPlanSelectFirst(); FP.IsValid(); FP = FlightPlanSelectNext(FP))
		{
			FpListEHS.RemoveFpFromTheList(FP);
		}
		FpListEHS.AddFpToTheList(FlightPlanSelectASEL());
	}
}

void CCAMS::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
	CFlightPlan FlightPlan = FlightPlanSelectASEL();
	if (!ControllerMyself().IsValid() || !ControllerMyself().IsController())
		return;

	if (!FlightPlan.IsValid() || FlightPlan.GetSimulated())
		return;

	if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerCallsign())>0)
		return;

	if (FunctionId == ItemCodes::TAG_FUNC_SQUAWK_POPUP)
	{
		OpenPopupList(Area, "Squawk", 1);
		AddPopupListElement("Auto assign", "", ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_AUTO);
		AddPopupListElement("Manual set", "", ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_MANUAL);
		AddPopupListElement("Discrete", "", ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_DISCRETE);
		AddPopupListElement("VFR", "", ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_VFR);
	}
	else if (FunctionId == ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_MANUAL)
	{
		OpenPopupEdit(Area, ItemCodes::TAG_FUNC_ASSIGN_SQUAWK, "");
	}
	else if (FunctionId == ItemCodes::TAG_FUNC_ASSIGN_SQUAWK)
	{
		FlightPlan.GetControllerAssignedData().SetSquawk(sItemString);
	}
	else if (FunctionId == ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_DISCRETE)
	{
		try
		{
			if (PendingSquawks.find(FlightPlan.GetCallsign()) == PendingSquawks.end())
			{
				PendingSquawks.insert(std::make_pair(FlightPlan.GetCallsign(), std::async(LoadWebSquawk,
					FlightPlan, ControllerMyself(), collectUsedCodes(FlightPlan), isADEPvicinity(FlightPlan), CCAMS::GetConnectionType())));
#ifdef _DEBUG
				if (CCAMS::GetConnectionType() > 2)
				{
					string DisplayMsg{ "A request for a simulated aircraft has been detected: " + string { FlightPlan.GetCallsign() } };
					DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
				}
#endif
			}
		}
		catch (std::runtime_error const& e)
		{
			DisplayUserMessage(MY_PLUGIN_NAME, "Error", e.what(), true, true, false, false, false);
		}
		catch (...)
		{
			DisplayUserMessage(MY_PLUGIN_NAME, "Error", std::to_string(GetLastError()).c_str(), true, true, false, false, false);
		}
	}
	else if (FunctionId == ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_AUTO)
	{
		if (isEligibleSquawkModeS(FlightPlan))
		{
			FlightPlan.GetControllerAssignedData().SetSquawk(squawkModeS);
			return;
		}

		try
		{
			if (PendingSquawks.find(FlightPlan.GetCallsign()) == PendingSquawks.end())
			{
				PendingSquawks.insert(std::make_pair(FlightPlan.GetCallsign(), std::async(LoadWebSquawk,
					FlightPlan, ControllerMyself(), collectUsedCodes(FlightPlan), isADEPvicinity(FlightPlan), CCAMS::GetConnectionType())));
#ifdef _DEBUG
				if (CCAMS::GetConnectionType() > 2)
				{
					string DisplayMsg{ "A request for a simulated aircraft has been detected: " + string { FlightPlan.GetCallsign() } };
					DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
				}
#endif
			}
		}
		catch (std::runtime_error const& e)
		{
			DisplayUserMessage(MY_PLUGIN_NAME, "Error", e.what(), true, true, false, false, false);
		}
		catch (...)
		{
			DisplayUserMessage(MY_PLUGIN_NAME, "Error", std::to_string(GetLastError()).c_str(), true, true, false, false, false);
		}

	}
	else if (FunctionId == ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_VFR)
	{
		FlightPlan.GetControllerAssignedData().SetSquawk(squawkVFR);
	}
}

void CCAMS::OnTimer(int Counter)
{
	if (fUpdateString.valid() && fUpdateString.wait_for(0ms) == future_status::ready)
		DoInitialLoad(fUpdateString);


	if (ControllerMyself().IsValid() && ControllerMyself().IsController())
	{
		AssignPendingSquawks();

		if (!(Counter % 10) && autoAssign)
		{
#ifdef _DEBUG
			//string DisplayMsg = string{ FlightPlan.GetCallsign() } + " has NOT a valid squawk code (ASSIGNED '" + assr + "', SET " + pssr + ")";
			//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", "Performing periodical automatic squawk code assignment", true, false, false, false, false);
#endif

			for (CRadarTarget RadarTarget = RadarTargetSelectFirst(); RadarTarget.IsValid();
				RadarTarget = RadarTargetSelectNext(RadarTarget))
			{
				//if (!RadarTarget.GetPosition().IsFPTrackPosition())
				//	continue;

				AssignAutoSquawk(RadarTarget.GetCorrelatedFlightPlan());
			}

		}
	}
}

void CCAMS::AssignAutoSquawk(CFlightPlan& FlightPlan)
{
	string DisplayMsg;
	const char* assr = FlightPlan.GetControllerAssignedData().GetSquawk();
	const char* pssr = FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetSquawk();

	// check controller class validity and restrict to APP/CTR/FSS controller types
	if (!ControllerMyself().IsValid() || !ControllerMyself().IsController() || (ControllerMyself().GetFacility() > 1 && ControllerMyself().GetFacility() < 5))
		return;

	// Check if FlightPlan is already processed
	if (IsFlightPlanProcessed(FlightPlan))
		return;

	// check for exclusion arguments from automatic squawk assignment
	// 1 .disregard simulated flight plans (out of the controllers range)
	// 2. disregard flight with flight rule VFR
	// 3. this flight has already assigned a valid unique code
	if (FlightPlan.GetSimulated() || strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V") == 0)
	{
		ProcessedFlightPlans.push_back(FlightPlan.GetCallsign());
		return;
	}
	else if (FlightPlan.GetSectorEntryMinutes() < 0)
	{
		// the flight will never enter the sector of the current controller
		ProcessedFlightPlans.push_back(FlightPlan.GetCallsign());
		return;
	}
	else if (hasValidSquawk(FlightPlan))
	{
		ProcessedFlightPlans.push_back(FlightPlan.GetCallsign());
		return;
	}
	else
	{
#ifdef _DEBUG
		DisplayMsg = string{ FlightPlan.GetCallsign() } + " has NOT a valid squawk code (ASSIGNED '" + assr + "', SET " + pssr + ")";
		DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
	}

	// disregard if the flight is assumed in the vicinity of the departure or arrival airport
	if (isADEPvicinity(FlightPlan) || FlightPlan.GetDistanceToDestination() < APTcodeMaxDist)
		return;

#ifdef _DEBUG
	DisplayMsg = string{ FlightPlan.GetCallsign() } + ": Tracking Controller Len '" + to_string(strlen(FlightPlan.GetTrackingControllerCallsign())) + "', CoordNextC '" + string{ FlightPlan.GetCoordinatedNextController() } + "', Minutes to entry " + to_string(FlightPlan.GetSectorEntryMinutes()) + ", TrackingMe: " + to_string(FlightPlan.GetTrackingControllerIsMe());
	//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
	if (!FlightPlan.GetTrackingControllerIsMe())
	{
		// the current controller is not tracking the flight plan

		CFlightPlanPositionPredictions Pos = FlightPlan.GetPositionPredictions();
		int min;

		for (min = 0; min < Pos.GetPointsNumber(); min++)
		{
			if (min <= 15 && _stricmp(FlightPlan.GetPositionPredictions().GetControllerId(min), "--") != 0)
			{
				break;
			}
		}


		//int nextController = 0;
		//const char* NextControllerId = "--";
		//bool WillEnterSector = false;
		//for (int min = 0; min < Pos.GetPointsNumber(); min++)
		//{
		//	if (min <= 15 && _stricmp(NextControllerId, "--") == 0 && _stricmp(FlightPlan.GetPositionPredictions().GetControllerId(min), "--") != 0)
		//	{
		//		NextControllerId = FlightPlan.GetPositionPredictions().GetControllerId(min);
		//		//nextController = min;
		//	}

		//	if (_stricmp(ControllerMyself().GetPositionId(), FlightPlan.GetPositionPredictions().GetControllerId(min)) == 0)
		//	{
		//		// if the current controller is predicted at any point of the route, the loop is stopped
		//		// the value min will then be the time for this flight to enter the sector of the current controller
		//		break;
		//	}
		//}
		//while (min < min(Pos.GetPointsNumber(), 15))
		//{
		//		break;

		//	min++;
		//}

		if (strlen(FlightPlan.GetTrackingControllerCallsign()) > 0)
		{
			// another controller is currently tracking the flight
			return;
		}
		//else if ((strlen(FlightPlan.GetCoordinatedNextController()) > 0 && FlightPlan.GetCoordinatedNextController() != ControllerMyself().GetCallsign()))
		else if (_stricmp(ControllerMyself().GetPositionId(), FlightPlan.GetPositionPredictions().GetControllerId(min)) != 0)
		{
			// the current controller is not the next controller of this flight
			return;
		}
		//else if (FlightPlan.GetSectorEntryMinutes() > 15 || FlightPlan.GetSectorEntryMinutes() < 0)
		else if (FlightPlan.GetSectorEntryMinutes() > 15)
		{
			// the flight is still too far away from the current controllers sector
			return;
		}
		else
		{
//#ifdef _DEBUG
			// The current controller is not tracking the flight, but automatic squawk assignment is applicable
			DisplayMsg = string{ FlightPlan.GetCallsign() } + " IS eligible for automatic squawk assignment. ASSIGNED '" + assr + "', SET " + pssr + ", Sector entry in " + to_string(FlightPlan.GetSectorEntryMinutes()) + " MIN";
			DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
//#endif
		}
	}

	// if the function has not been ended, the flight is subject to automatic squawk assignment

	//auto assr = FlightPlan.GetControllerAssignedData().GetSquawk();
	//auto pssr = RadarTarget.GetPosition().GetSquawk();

	if (isEligibleSquawkModeS(FlightPlan))
	{
		FlightPlan.GetControllerAssignedData().SetSquawk(squawkModeS);
//#ifdef _DEBUG
		DisplayMsg = string{ FlightPlan.GetCallsign() } + ", code 1000 AUTO assigned";
		DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
//#endif
	}
	else
	{
		PendingSquawks.insert(std::make_pair(FlightPlan.GetCallsign(), std::async(LoadWebSquawk,
			FlightPlan, ControllerMyself(), collectUsedCodes(FlightPlan), isADEPvicinity(FlightPlan), CCAMS::GetConnectionType())));
#ifdef _DEBUG
		DisplayMsg = string{ FlightPlan.GetCallsign() } + ", unique code AUTO assigned";
		DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
	}

	ProcessedFlightPlans.push_back(FlightPlan.GetCallsign());
}

void CCAMS::AssignPendingSquawks()
{
	for (auto it = PendingSquawks.begin(), next_it = it; it != PendingSquawks.end(); it = next_it)
	{
		bool must_delete = false;
		if (it->second.valid() && it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			std::string squawk = it->second.get();
			//if (squawk is an error number)
//#ifdef _DEBUG
			string DisplayMsg = string{ it->first } + ", code " + squawk + " assigned";
			DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
//#endif
			if (!FlightPlanSelect(it->first).GetControllerAssignedData().SetSquawk(squawk.c_str()))
			{
				if (squawk == "E404")
				{
					DisplayUserMessage(MY_PLUGIN_NAME, "Error 404", "The internet connection cannot be initiated", true, true, false, false, false);
				}
				else if (squawk == "E406")
				{
					DisplayUserMessage(MY_PLUGIN_NAME, "Error 406", "No answer received from the CCAMS server", true, true, false, false, false);
				}
				else
				{
					string DisplayMsg{ "Your request for a squawk from the centralised code server failed. Check your plugin version, try again or revert to the ES built-in functionalities for assigning a squawk (F9)." };
					DisplayUserMessage(MY_PLUGIN_NAME, "Error", DisplayMsg.c_str(), true, true, false, false, false);
					DisplayUserMessage(MY_PLUGIN_NAME, "Error", ("For troubleshooting, report code '" + squawk + "'").c_str(), true, true, false, false, false);
				}
			}
			must_delete = true;
		}

		++next_it;
		if (must_delete)
		{
			PendingSquawks.erase(it);
		}
	}
}

void CCAMS::DoInitialLoad(future<string> & fmessage)
{
	try
	{
		string message = fmessage.get();
		smatch match;
#ifdef _DEBUG
		string DisplayMsg = "Update string downloaded: " + message;
		DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
		if (regex_match(message, match, regex("^(\\d+)[:]([^:]+)[:]([A-Z,]+)$", regex::icase)))
		{
			int new_v = stoi(match[1].str(), nullptr, 0);
			if (new_v > MY_PLUGIN_VERSIONCODE)
				throw error{ "Your " + string { MY_PLUGIN_NAME } + " plugin (version " + MY_PLUGIN_VERSION + ") is outdated. Please change to the latest version.\n\nVisit https://github.com/kusterjs/CCAMS/releases" };
			else
				pluginVersionRestricted = false;

			//ModeSAirports = move(split(match[2].str(), ','));
			ModeSAirports = match[2].str();
			EquipmentCodesFAA = match[3].str();
			EquipmentCodesFAA.erase(remove(EquipmentCodesFAA.begin(), EquipmentCodesFAA.end(), ','), EquipmentCodesFAA.end());
		}
		else
			throw error{ string { MY_PLUGIN_NAME }  + " plugin couldn't parse the server data" };
	}
	catch (modesexception & e)
	{
		e.whatMessageBox();
	}
	catch (exception & e)
	{
		MessageBox(NULL, e.what(), MY_PLUGIN_NAME, MB_OK | MB_ICONERROR);
	}
	fmessage = future<string>();
}

inline bool CCAMS::IsFlightPlanProcessed(CFlightPlan & FlightPlan)
{
	string callsign { FlightPlan.GetCallsign() };
	for (auto &pfp : ProcessedFlightPlans)
		if (pfp.compare(callsign) == 0)
			return true;

	//ProcessedFlightPlans.push_back(FlightPlan.GetCallsign());
	return false;
}

bool CCAMS::isAcModeS(const CFlightPlan& FlightPlan) const
{
	////check for ICAO suffix
	//if (acceptEquipmentICAO)
	//{
	//	std::string actype = FlightPlan.GetFlightPlanData().GetAircraftInfo();
	//	std::regex icao_format("(.{2,4})\\/([LMHJ])-(.*)\\/(.*)", std::regex::icase);
	//	std::smatch acdata;
	//	if (std::regex_match(actype, acdata, icao_format) && acdata.size() == 5)
	//	{
	//		for (auto& code : EquipmentCodesICAO)
	//			if (strstr(acdata[4].str().c_str(), code.c_str()) != nullptr)
	//			if (strncmp(acdata[4].str().c_str(), code.c_str(), 1))
	//				return true;
	//	}
	//}

	////check for FAA suffix
	//if (acceptEquipmentFAA)
	//{
	//	std::string equipement_suffix{ FlightPlan.GetFlightPlanData().GetCapibilities() };
	//	if (equipement_suffix == "?")
	//		return false;

	//	for (auto& code : EquipmentCodesFAA)
	//		if (equipement_suffix == code)
	//			return true;
	//}

	//return false;
	return hasEquipment(FlightPlan, acceptEquipmentFAA, acceptEquipmentICAO, EquipmentCodesICAO);
}

bool CCAMS::isADEPvicinity(const CFlightPlan& FlightPlan) const
{
	if (FlightPlan.GetCorrelatedRadarTarget().GetGS() < APTcodeMaxGS &&
		FlightPlan.GetDistanceFromOrigin() < APTcodeMaxDist)
		return true;
	return false;
}

bool CCAMS::isApModeS(const string& icao) const
{
	if (regex_match(icao, ModeSAirports))
		return true;
	//for (auto& zone : ModeSAirports)
	//	if (zone.compare(0, zone.length(), icao, 0, zone.length()) == 0)
	//		return true;
	return false;
}

bool CCAMS::isEHS(const CFlightPlan& FlightPlan) const
{
	////check for ICAO suffix
	//std::string actype = FlightPlan.GetFlightPlanData().GetAircraftInfo();
	//std::regex icao_format("(.{2,4})\\/([LMHJ])-(.*)\\/(.*)", std::regex::icase);
	//std::smatch acdata;
	//if (std::regex_match(actype, acdata, icao_format) && acdata.size() == 5)
	//{
	//	for (const auto& code : EquipmentCodesICAOEHS)
	//		if (strncmp(acdata[4].str().c_str(), code.c_str(), 1))
	//			return true;
	//}

	////check for FAA suffix
	//if (acceptEquipmentFAA)
	//{
	//	std::string equipement_suffix{ FlightPlan.GetFlightPlanData().GetCapibilities() };
	//	if (equipement_suffix == "?")
	//		return false;

	//	for (auto& code : EquipmentCodesFAA)
	//		if (equipement_suffix == code)
	//			return true;
	//}

	//return false;
	return hasEquipment(FlightPlan, acceptEquipmentFAA, true, EquipmentCodesICAOEHS);
}

bool CCAMS::hasEquipment(const CFlightPlan& FlightPlan, bool acceptEquipmentFAA, bool acceptEquipmentICAO, string CodesICAO) const
{
	//check for ICAO suffix
	if (acceptEquipmentICAO)
	{
		//const char* actype = FlightPlan.GetFlightPlanData().GetAircraftInfo();
		//std::regex icao_format("(.{2,4})\\/([LMHJ])-(.*)\\/(.*)", std::regex::icase);
		cmatch acdata;
		if (regex_match(FlightPlan.GetFlightPlanData().GetAircraftInfo(), acdata, regex("(\\w{2,4})\\/([LMHJ])-(\\w+)\\/(\\w*?[" + CodesICAO + "]\\w*)", std::regex::icase)))
			return true;
	}

	//check for FAA suffix
	if (acceptEquipmentFAA)
	{
		//string equipement_suffix{ FlightPlan.GetFlightPlanData().GetCapibilities() };
		if (EquipmentCodesFAA.find(FlightPlan.GetFlightPlanData().GetCapibilities()) != string::npos)
			return true;

		//if (equipement_suffix == "?")
		//	return false;

		//for (auto& code : EquipmentCodesFAA)
		//	if (equipement_suffix == code)
		//		return true;
	}

	return false;
}

bool CCAMS::isEligibleSquawkModeS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const
{
	//return isAcModeS(FlightPlan) && isApModeS(FlightPlan.GetFlightPlanData().GetDestination()) &&
	//	(isApModeS(FlightPlan.GetFlightPlanData().GetOrigin()) || !isADEPvicinity(FlightPlan));
	return isAcModeS(FlightPlan) && isApModeS(FlightPlan.GetFlightPlanData().GetDestination()) &&
		(isApModeS(FlightPlan.GetFlightPlanData().GetOrigin()) || (!isADEPvicinity(FlightPlan) && isApModeS(ControllerMyself().GetCallsign())));
}

bool CCAMS::hasValidSquawk(const EuroScopePlugIn::CFlightPlan& FlightPlan)
{
	const char* assr = FlightPlan.GetControllerAssignedData().GetSquawk();
	const char* pssr = FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetSquawk();
	string DisplayMsg;

	if ((strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V") == 0 && (strcmp(assr, squawkVFR) == 0 || strcmp(pssr, squawkVFR) == 0))
		|| (isEligibleSquawkModeS(FlightPlan) && (strcmp(assr, squawkModeS) == 0 || strcmp(pssr, squawkModeS) == 0)))
	{
		return true;
	}
	else if (strlen(assr) == 4)
	{
		// assigned squawk is not valid
		if (!regex_search(assr, std::regex("[0-7]{4}")) || atoi(assr) % 100 == 0)
		{
#ifdef _DEBUG
			DisplayMsg = "ASSIGNED code " + string{ assr } + " is not valid for " + FlightPlan.GetCallsign();
			//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
			return false;
		}
	}
	else if (!regex_search(pssr, std::regex("[0-7]{4}")) || atoi(pssr) % 100 == 0)
	{
		// no squawk is assigned, but currently used code is not valid
		{
#ifdef _DEBUG
			DisplayMsg = "SET code " + string{ pssr } + " is not valid for " + FlightPlan.GetCallsign();
			//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
			return false;
		}
	}

	// searching for duplicate assignments
	for (CRadarTarget RadarTarget = RadarTargetSelectFirst(); RadarTarget.IsValid();
		RadarTarget = RadarTargetSelectNext(RadarTarget))
	{
		if (!RadarTarget.IsValid())
			continue;

		if (strcmp(RadarTarget.GetCallsign(),FlightPlan.GetCallsign()) == 0)
			continue;

		if (strlen(assr) == 4)
		{
			if (strcmp(assr, RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk()) == 0 || strcmp(assr, RadarTarget.GetPosition().GetSquawk()) == 0)
			{
				// duplicate identified for the assigned code
#ifdef _DEBUG
				DisplayMsg = "ASSIGNED code " + string{ assr } + " of " + FlightPlan.GetCallsign() + " is already used by " + RadarTarget.GetCallsign();
				//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
				return false;
			}

		}
		else
		{
			if (strcmp(pssr, RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk()) == 0 || strcmp(pssr, RadarTarget.GetPosition().GetSquawk()) == 0)
			{
				// duplicate identified for the actual set code
#ifdef _DEBUG
				DisplayMsg = "SET code " + string{ assr } + " of " + FlightPlan.GetCallsign() + " is already used by " + RadarTarget.GetCallsign();
				//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
				return false;

			}
			else
			{
				// as an option, if no code has been assigned and the currently used one has not been identified as a dpublicate, it could be set as the assigned code
				//FlightPlan.GetControllerAssignedData().SetSquawk(pssr);

			}
		}

//		// this code is also assigned to another aircraft
//		if (strcmp(assr, RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk()) == 0 || strcmp(pssr, RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk()) == 0)
//		{
//#ifdef _DEBUG
//			DisplayMsg = string{ FlightPlan.GetCallsign() } + ", code '" + RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk() + "' is already ASSIGNED to " + RadarTarget.GetCallsign();
//			//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
//#endif
//			return false;
//		}
//
//		// this code is already used by another aircraft
//		if (strcmp(assr, RadarTarget.GetPosition().GetSquawk()) == 0 || strcmp(pssr, RadarTarget.GetPosition().GetSquawk()) == 0)
//		{
//#ifdef _DEBUG
//			DisplayMsg = "Code " + string{ RadarTarget.GetPosition().GetSquawk() } + " is already SET by " + RadarTarget.GetCallsign();
//			//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
//#endif
//			return false;
//		}
	}
	// no duplicate with assigend or used codes has been found
#ifdef _DEBUG
	DisplayMsg = "No duplicates found for " + string{ FlightPlan.GetCallsign() } + " (ASSIGNED '" + assr + "', SET code " + pssr + ")";
	DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
	return true;
}

std::vector<const char*> CCAMS::collectUsedCodes(const CFlightPlan& FlightPlan)
{
	vector<const char*> usedCodes;
	for (CRadarTarget RadarTarget = RadarTargetSelectFirst(); RadarTarget.IsValid();
		RadarTarget = RadarTargetSelectNext(RadarTarget))
	{
		if (!RadarTarget.IsValid())
		{
#ifdef _DEBUG
			string DisplayMsg{ "Invalid radar target found" };
			DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
			continue;
		}

		if (RadarTarget.GetCallsign() == FlightPlanSelectASEL().GetCallsign())
		{
#ifdef _DEBUG
			string DisplayMsg{ "The code of " + (string)RadarTarget.GetCallsign() + " is not considered" };
			//DisplayUserMessage(MY_PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
			continue;
		}

		// search for all controller assigned codes
		auto assr = RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk();
		if (strlen(assr) == 4 &&
			atoi(assr) % 100 != 0 &&
			strcmp(assr, squawkModeS) != 0 &&
			strcmp(assr, squawkVFR) != 0)
		{
			usedCodes.push_back(assr);
		}

		// search for all actual codes used by pilots
		auto pssr = RadarTarget.GetPosition().GetSquawk();
		if (strlen(pssr) == 4 &&
			atoi(pssr) % 100 != 0 &&
			strcmp(pssr, squawkModeS) != 0 &&
			strcmp(pssr, squawkVFR) != 0 &&
			strcmp(pssr, assr) != 0)
		{
			usedCodes.push_back(pssr);
		}

		sort(usedCodes.begin(), usedCodes.end());
		usedCodes.erase(unique(usedCodes.begin(), usedCodes.end()), usedCodes.end());
	}
	return usedCodes;
}

