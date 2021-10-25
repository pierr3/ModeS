#include "stdafx.h"
#include "CCAMS.h"

CCAMS::CCAMS(PluginData pd, const DefaultCodes&& dc) :
	CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
			pd.PLUGIN_NAME,
			pd.PLUGIN_VERSION,
			pd.PLUGIN_AUTHOR,
			pd.PLUGIN_LICENSE),
	pluginData(pd),
	EQUIPMENT_CODES(dc.EQUIPEMENT_CODES),
	EQUIPMENT_CODES_ICAO(dc.EQUIPEMENT_CODES_ICAO),
	EQUIPMENT_CODES_EHS(dc.EQUIPEMENT_CODES_EHS),
	ICAO_MODES(dc.ICAO_MODES)
{
	string DisplayMsg { "Version " + string { pd.PLUGIN_VERSION } + " loaded" };
	DisplayUserMessage(pd.PLUGIN_NAME, "Initialisation", DisplayMsg.c_str(), true, false, false, false, false);

	RegisterTagItemType("Transponder Type", ItemCodes::TAG_ITEM_ISMODES);
#ifdef _DEBUG
	RegisterTagItemType("EHS Heading", ItemCodes::TAG_ITEM_EHS_HDG);
	RegisterTagItemType("EHS Roll Angle", ItemCodes::TAG_ITEM_EHS_ROLL);
	RegisterTagItemType("EHS GS", ItemCodes::TAG_ITEM_EHS_GS);
#endif
	RegisterTagItemType("Mode S squawk error", ItemCodes::TAG_ITEM_ERROR_MODES_USE);
	RegisterTagItemType("Assigned squawk", ItemCodes::TAG_ITEM_SQUAWK);

	RegisterTagItemFunction("Auto assign squawk", ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_AUTO);
	RegisterTagItemFunction("Open SQUAWK assign popup", ItemCodes::TAG_FUNC_SQUAWK_POPUP);

#ifdef _DEBUG
	this->FpListEHS = RegisterFpList("Mode S EHS");
	if (this->FpListEHS.GetColumnNumber() == 0)
	{
		this->FpListEHS.AddColumnDefinition("C/S", 7, false, NULL, TAG_ITEM_TYPE_CALLSIGN, NULL, TAG_ITEM_FUNCTION_OPEN_FP_DIALOG, NULL, NULL);
		this->FpListEHS.AddColumnDefinition("HDG", 5, true, this->pluginData.PLUGIN_NAME, ItemCodes::TAG_ITEM_EHS_HDG, NULL, NULL, NULL, NULL);
		this->FpListEHS.AddColumnDefinition("Roll", 5, true, this->pluginData.PLUGIN_NAME, ItemCodes::TAG_ITEM_EHS_ROLL, NULL, NULL, NULL, NULL);
		this->FpListEHS.AddColumnDefinition("GS", 4, true, this->pluginData.PLUGIN_NAME, ItemCodes::TAG_ITEM_EHS_GS, NULL, NULL, NULL, NULL);
	}
#endif

	// Start new thread to get the version file from the server
	fUpdateString = async(LoadUpdateString, pd);

	// Set default setting values
	this->squawkVFR = "7000";
	this->acceptEquipmentICAO = true;
	this->acceptEquipmentFAA = true;
	this->autoAssignMSCC = true;
#ifdef _DEBUG
	this->autoAssign = true;
#else
	this->autoAssign = false;
#endif
	this->APTcodeMaxGS = 50;
	this->APTcodeMaxDist = 3;
	
	// Overwrite setting values by plugin settings, if available
	try
	{
		const char* cstrSetting = GetDataFromSettings("codeVFR");
		if (cstrSetting != NULL)
		{
			if (regex_search(cstrSetting, std::regex("[0-7]{4}")))
			{
				this->squawkVFR = cstrSetting;
			}
		}

		cstrSetting = GetDataFromSettings("acceptFPLformatICAO");
		if (cstrSetting != NULL)
		{
			if (strcmp(cstrSetting, "0") == 0)
			{
				this->acceptEquipmentICAO = false;
			}
		}

		cstrSetting = GetDataFromSettings("acceptFPLformatFAA");
		if (cstrSetting != NULL)
		{
			if (strcmp(cstrSetting, "0") == 0)
			{
				this->acceptEquipmentFAA = false;
			}
		}

		cstrSetting = GetDataFromSettings("AutoAssign");
		if (cstrSetting != NULL)
		{
			if (strcmp(cstrSetting, "0") == 0)
			{
				this->autoAssign = false;
			}
		}
	}
	catch (std::runtime_error const& e)
	{
		DisplayUserMessage(pd.PLUGIN_NAME, "Error", e.what(), true, true, false, false, false);
	}
	catch (...)
	{
		DisplayUserMessage(pd.PLUGIN_NAME, "Error", std::to_string(GetLastError()).c_str(), true, true, false, false, false);
	}

}

CCAMS::~CCAMS()
{}

bool CCAMS::OnCompileCommand(const char* command)
{
	string commandString(command);
	smatch matches;

	if (regex_match(commandString, matches, regex("\\.ccams\\s+(\\w+)", regex::icase)))
	{
		string pluginCommand = matches[1];
		transform(pluginCommand.begin(), pluginCommand.end(), pluginCommand.begin(), ::tolower);

#ifdef _DEBUG
		if (pluginCommand == "ehslist")
		{
			this->FpListEHS.ShowFpList(true);
			return true;
		}
#endif

		return false;
	}
#ifdef _DEBUG
	if (Help(command)) return true;
#endif

	return false;
}

bool CCAMS::Help(const char* Command)
{
	string sCommand(Command);
	if (_stricmp(Command, ".help") == 0)
	{
		DisplayUserMessage("HELP", "HELP", ".HELP CCAMS | Centralised code assignment and management system Help", true, true, true, true, false);
		return NULL;
	}
	else if (_stricmp(Command, ".help ccams") == 0)
	{
		// Display HELP
		DisplayUserMessage("HELP", this->pluginData.PLUGIN_NAME, ".CCAMS EHSLIST | Displays the flight plan list with EHS values of the currently selected aircraft.", true, true, true, true, false);
		return true;
	}
#ifdef _DEBUG
	else if (_stricmp(Command, ".ccams reset") == 0)
	{
		this->ProcessedFlightPlans.clear();
	}
	else if (_stricmp(sCommand.substr(0, 7).c_str(), ".ccams ") == 0 && sCommand.size() > 7)
	{
		string callsign = sCommand.substr(7, sCommand.size() - 7);

		for (CFlightPlan FlightPlan = FlightPlanSelectFirst();
			FlightPlan.IsValid();
			FlightPlan = FlightPlanSelectNext(FlightPlan))
		{
			if (_stricmp(callsign.c_str(), FlightPlan.GetCallsign()) == 0)
			{
				string DisplayMsg = string{ FlightPlan.GetCallsign() } + ": Tracking Controller Len '" + to_string(strlen(FlightPlan.GetTrackingControllerCallsign())) + "', CoordNextC '" + string{ FlightPlan.GetCoordinatedNextController() } + "', Minutes to entry " + to_string(FlightPlan.GetSectorEntryMinutes()) + ", TrackingMe: " + to_string(FlightPlan.GetTrackingControllerIsMe());
				DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
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

	AssignAutoSquawk(FlightPlan);

	if (ItemCode == ItemCodes::TAG_ITEM_ISMODES)
	{
		if (isAcModeS(FlightPlan))
			strcpy_s(sItemString, 16, "S");
		else
			strcpy_s(sItemString, 16, "A");
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
				sprintf_s(sItemString, 16, "%03i�", RadarTarget.GetPosition().GetReportedHeading() % 360);
	#ifdef _DEBUG
				string DisplayMsg{ to_string(RadarTarget.GetPosition().GetReportedHeading()) };
				DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
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
					sprintf_s(sItemString, 16, "%c%i�", rollb < 0 ? 'R' : 'L', abs(rollb));
				}
	#ifdef _DEBUG
				string DisplayMsg{ to_string(abs(rollb)) };
				DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
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
				DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
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
			if (strcmp(assr, ::mode_s_code) != 0 &&
				strcmp(pssr, ::mode_s_code) != 0)
				return;

			*pColorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
			strcpy_s(sItemString, 16, "MSSQ");
		}
		else if (ItemCode == ItemCodes::TAG_ITEM_SQUAWK)
		{
			auto assr = RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk();
			auto pssr = RadarTarget.GetPosition().GetSquawk();

			if (!isEligibleSquawkModeS(FlightPlan) && (strcmp(assr, ::mode_s_code) == 0 || (strcmp(pssr, ::mode_s_code) == 0 && strlen(assr) == 0)))
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
	if (!FlightPlan.GetTrackingControllerIsMe())
		ProcessedFlightPlans.erase(remove(ProcessedFlightPlans.begin(), ProcessedFlightPlans.end(), FlightPlan.GetCallsign()),
								   ProcessedFlightPlans.end());
}

void CCAMS::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	ProcessedFlightPlans.erase(remove(ProcessedFlightPlans.begin(), ProcessedFlightPlans.end(), FlightPlan.GetCallsign()),
							   ProcessedFlightPlans.end());

#ifdef _DEBUG
	this->FpListEHS.RemoveFpFromTheList(FlightPlan);
#endif
}

void CCAMS::OnRefreshFpListContent(CFlightPlanList AcList)
{

	if (ControllerMyself().IsValid() && RadarTargetSelectASEL().IsValid())
	{
#ifdef _DEBUG
		string DisplayMsg{ "The following call sign was identified to be added to the EHS Mode S list: " + string { FlightPlanSelectASEL().GetCallsign() } };
		//DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
		for (CFlightPlan FP = FlightPlanSelectFirst(); FP.IsValid(); FP = FlightPlanSelectNext(FP))
		{
			this->FpListEHS.RemoveFpFromTheList(FP);
		}
		this->FpListEHS.AddFpToTheList(FlightPlanSelectASEL());
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
	else if (FunctionId == ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_AUTO)
	{
		if (isEligibleSquawkModeS(FlightPlan))
		{
			FlightPlan.GetControllerAssignedData().SetSquawk(::mode_s_code);
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
					DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
				}
#endif
			}
		}
		catch (std::runtime_error const& e)
		{
			DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Error", e.what(), true, true, false, false, false);
		}
		catch (...)
		{
			DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Error", std::to_string(GetLastError()).c_str(), true, true, false, false, false);
		}

	}
	else if (FunctionId == ItemCodes::TAG_FUNC_ASSIGN_SQUAWK_VFR)
	{
		FlightPlan.GetControllerAssignedData().SetSquawk(this->squawkVFR);
	}
}

void CCAMS::OnTimer(int Counter)
{
	if (fUpdateString.valid() && fUpdateString.wait_for(0ms) == future_status::ready)
		DoInitialLoad(fUpdateString);


	if (ControllerMyself().IsValid() && ControllerMyself().IsController())
	{
		AssignPendingSquawks();

		if (!(Counter % 5) && this->autoAssign)
		{
			for (CRadarTarget RadarTarget = RadarTargetSelectFirst();
				RadarTarget.IsValid();
				RadarTarget = RadarTargetSelectNext(RadarTarget))
			{
				if (!RadarTarget.GetPosition().IsFPTrackPosition())
					continue;

				AssignAutoSquawk(RadarTarget.GetCorrelatedFlightPlan());
			}

		}
	}
}

void CCAMS::AssignAutoSquawk(CFlightPlan& FlightPlan)
{
	string DisplayMsg;

	// check controller class validity and restrict to APP/CTR/FSS controller types
	if (!ControllerMyself().IsValid() || !ControllerMyself().IsController() || (ControllerMyself().GetFacility() > 1 && ControllerMyself().GetFacility() < 5))
		return;

#ifndef _DEBUG
	if (RadarTarget.GetPosition().GetFlightLevel() < 24500)
	if (FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetFlightLevel() < 10000)
			return;
#endif // _DEBUG

	// Check if FlightPlan is already processed
	if (IsFlightPlanProcessed(FlightPlan))
		return;

	// disregard if the flight is assumed in the vicinity of the departure airport
	if (isADEPvicinity(FlightPlan))
		return;

	// disregard simulated flight plans (out of the controllers range)
	if (FlightPlan.GetSimulated())
		return;

	// disregard flight with flight rule VFR
	if (strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V") == 0)
		return;

#ifdef _DEBUG
	DisplayMsg = string{ FlightPlan.GetCallsign() } + ": Tracking Controller Len '" + to_string(strlen(FlightPlan.GetTrackingControllerCallsign())) + "', CoordNextC '" + string{ FlightPlan.GetCoordinatedNextController() } + "', Minutes to entry " + to_string(FlightPlan.GetSectorEntryMinutes()) + ", TrackingMe: " + to_string(FlightPlan.GetTrackingControllerIsMe());
	//DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
	if (!FlightPlan.GetTrackingControllerIsMe())
	{
		// the current controller is not tracking the flight plan

		if (strlen(FlightPlan.GetTrackingControllerCallsign()) > 0)
		{
			// another controller is currently tracking the flight
			return;
		}
		else if ((strlen(FlightPlan.GetCoordinatedNextController()) > 0 && FlightPlan.GetCoordinatedNextController() != ControllerMyself().GetCallsign()))
		{
			// the current controller is not the next controller of this flight
			return;
		}
		else if (FlightPlan.GetSectorEntryMinutes() > 15)
		{
			// the flight is still too far away from the current controllers sector
			return;
		}
		else
		{
#ifdef _DEBUG
			// The current controller is not tracking the flight, but automatic squawk assignment is applicable
			DisplayMsg = "The non-tracked flight " + string{ FlightPlan.GetCallsign() } + " IS eligible for automatic squawk assingment. Tracking Controller Len: '" + to_string(strlen(FlightPlan.GetTrackingControllerCallsign())) + "', CoordNextC: '" + string{ FlightPlan.GetCoordinatedNextController() } + "', Minutes to entry: " + to_string(FlightPlan.GetSectorEntryMinutes());
			DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
		}
	}

	// if the function has not been ended, the flight is subject to automatic squawk assignment

	if (hasValidSquawk(FlightPlan))
	{
		// this flight has already assigned a valid unique code, add this flight to the processed flight plans for performance
		ProcessedFlightPlans.push_back(FlightPlan.GetCallsign());
		return;
	}

	//auto assr = FlightPlan.GetControllerAssignedData().GetSquawk();
	//auto pssr = RadarTarget.GetPosition().GetSquawk();

	if (isEligibleSquawkModeS(FlightPlan))
	{
		//FlightPlan.GetControllerAssignedData().SetSquawk(::mode_s_code);
#ifdef _DEBUG
		DisplayMsg = "Squawk Code Auto Assignment (1000): " + string{ FlightPlan.GetCallsign() };
		DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
	}
	else
	{
		//PendingSquawks.insert(std::make_pair(FlightPlan.GetCallsign(), std::async(LoadWebSquawk,
		//	FlightPlan, ControllerMyself(), collectUsedCodes(FlightPlan), isADEPvicinity(FlightPlan), CCAMS::GetConnectionType())));
#ifdef _DEBUG
		DisplayMsg = "Squawk Code Auto Assignment (unique): " + string{ FlightPlan.GetCallsign() };
		DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
	}
#ifdef _DEBUG
	ProcessedFlightPlans.push_back(FlightPlan.GetCallsign());
#endif

}

void CCAMS::AssignPendingSquawks()
{
	for (auto it = PendingSquawks.begin(), next_it = it; it != PendingSquawks.end(); it = next_it)
	{
		bool must_delete = false;
		if (it->second.valid() && it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			std::string squawk = it->second.get();
			//if (squawk is an error number)
			string DisplayMsg = "The received squawk code is " + squawk;
			DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
			if (!FlightPlanSelect(it->first).GetControllerAssignedData().SetSquawk(squawk.c_str()))
			{
				string DisplayMsg{ "Your request for a squawk from the centralised code server failed. Check your plugin version, try again or revert to the ES built-in functionalities for assigning a squawk (F9).\n\nFor troubleshooting, report code " + squawk };
				DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Error", DisplayMsg.c_str(), true, true, false, false, false);
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
		regex update_string { "^([A-z,]+)[|]([A-z,]+)[|]([0-9]{1,3})$" };
		smatch match;
		if (regex_match(message, match, update_string))
		{
			EQUIPMENT_CODES = std::move(split(match[1].str(), ','));
			ICAO_MODES = std::move(split(match[2].str(), ','));

			int new_v = stoi(match[3].str(), nullptr, 0);
			if (new_v > pluginData.VERSION_CODE)
				throw error{ "Your " + string { this->pluginData.PLUGIN_NAME } + " plugin (version " + pluginData.PLUGIN_VERSION + ") is outdated. Please change to the latest version.\n\nVisit it\n\nhttps://github.com/kusterjs/CCAMS/releases" };
		}
		else
			throw error{ string { this->pluginData.PLUGIN_NAME }  + " plugin couldn't parse the server data" };
	}
	catch (modesexception & e)
	{
		e.whatMessageBox();
	}
	catch (exception & e)
	{
		MessageBox(NULL, e.what(), this->pluginData.PLUGIN_NAME, MB_OK | MB_ICONERROR);
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

bool CCAMS::isAcModeS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const
{
	//check for ICAO suffix
	if (this->acceptEquipmentICAO)
	{
		std::string actype = FlightPlan.GetFlightPlanData().GetAircraftInfo();
		std::regex icao_format("(.{2,4})\\/([LMHJ])-(.*)\\/(.*)", std::regex::icase);
		std::smatch acdata;
		if (std::regex_match(actype, acdata, icao_format) && acdata.size() == 5)
		{
			for (const auto& code : EQUIPMENT_CODES_ICAO)
				if (strncmp(acdata[4].str().c_str(), code.c_str(), 1))
					return true;
		}
	}

	//check for FAA suffix
	if (this->acceptEquipmentFAA)
	{
		std::string equipement_suffix{ FlightPlan.GetFlightPlanData().GetCapibilities() };
		if (equipement_suffix == "?")
			return false;

		for (auto& code : EQUIPMENT_CODES)
			if (equipement_suffix == code)
				return true;
	}

	return false;
}

bool CCAMS::isADEPvicinity(const EuroScopePlugIn::CFlightPlan& FlightPlan) const
{
	if (FlightPlan.GetCorrelatedRadarTarget().GetGS() < this->APTcodeMaxGS &&
		FlightPlan.GetDistanceFromOrigin() < this->APTcodeMaxDist)
		return true;
	return false;
}

bool CCAMS::isApModeS(const std::string& icao) const
{
	for (auto& zone : ICAO_MODES)
		if (zone.compare(0, zone.length(), icao, 0, zone.length()) == 0)
			return true;
	return false;
}

bool CCAMS::isEHS(const EuroScopePlugIn::CFlightPlan& FlightPlan) const
{
	//check for ICAO suffix
	std::string actype = FlightPlan.GetFlightPlanData().GetAircraftInfo();
	std::regex icao_format("(.{2,4})\\/([LMHJ])-(.*)\\/(.*)", std::regex::icase);
	std::smatch acdata;
	if (std::regex_match(actype, acdata, icao_format) && acdata.size() == 5)
	{
		for (const auto& code : EQUIPMENT_CODES_EHS)
			if (strncmp(acdata[4].str().c_str(), code.c_str(), 1))
				return true;
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
#ifdef _DEBUG
	string DisplayMsg = string{ FlightPlan.GetCallsign() } + " has ASSIGNED code '" + assr + "' and SET code '" + pssr + "'";
	DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif

	if ((strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V") == 0 && (strcmp(assr, this->squawkVFR) == 0 || strcmp(pssr, this->squawkVFR) == 0))
		|| (isEligibleSquawkModeS(FlightPlan) && (strcmp(assr, ::mode_s_code) == 0 || strcmp(pssr, ::mode_s_code) == 0)))
	{
		return true;
	}
	else if (strlen(assr) == 4)
	{
		// assigned squawk is not valid
		if (!regex_search(assr, std::regex("[0-7]{4}")) || atoi(assr) % 100 == 0)
			return false;
	}
	else if (!regex_search(pssr, std::regex("[0-7]{4}")) || atoi(pssr) % 100 == 0)
	{
		// no squawk is assigned, but currently used code is not valid
		return false;
	}

	// searching for duplicate assignments
	for (EuroScopePlugIn::CRadarTarget RadarTarget = RadarTargetSelectFirst(); RadarTarget.IsValid();
		RadarTarget = RadarTargetSelectNext(RadarTarget))
	{
		if (!RadarTarget.IsValid())
			continue;

		if (RadarTarget.GetCallsign() == FlightPlanSelectASEL().GetCallsign())
			continue;

		// this code is also assigned to another aircraft
		if (strcmp(assr, RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk()) == 0 || strcmp(pssr, RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk()) == 0)
			return false;

		// this code is already used by another aircraft
		if (strcmp(assr, RadarTarget.GetPosition().GetSquawk()) == 0 || strcmp(pssr, RadarTarget.GetPosition().GetSquawk()) == 0)
			return false;
	}
	// no duplicate with assigend or used codes has been found
	// even if the assigned squawk is still empty, the currently used code can be accepted and no new codes needs to be issued
	//if (strlen(assr) != 4)
	//	FlightPlan.GetControllerAssignedData().SetSquawk(pssr);
	return true;
}

std::vector<const char*> CCAMS::collectUsedCodes(const EuroScopePlugIn::CFlightPlan& FlightPlan)
{
	vector<const char*> usedCodes;
	for (EuroScopePlugIn::CRadarTarget RadarTarget = RadarTargetSelectFirst(); RadarTarget.IsValid();
		RadarTarget = RadarTargetSelectNext(RadarTarget))
	{
		if (!RadarTarget.IsValid())
		{
#ifdef _DEBUG
			string DisplayMsg{ "Invalid radar target found" };
			DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
			continue;
		}

		if (RadarTarget.GetCallsign() == FlightPlanSelectASEL().GetCallsign())
		{
#ifdef _DEBUG
			string DisplayMsg{ "The code of " + (string)RadarTarget.GetCallsign() + " is not considered" };
			//DisplayUserMessage(this->pluginData.PLUGIN_NAME, "Debug", DisplayMsg.c_str(), true, false, false, false, false);
#endif
			continue;
		}

		// search for all controller assigned codes
		auto assr = RadarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetSquawk();
		if (strlen(assr) == 4 &&
			atoi(assr) % 100 != 0 &&
			strcmp(assr, ::mode_s_code) != 0 &&
			strcmp(assr, this->squawkVFR) != 0)
		{
			usedCodes.push_back(assr);
		}

		// search for all actual codes used by pilots
		auto pssr = RadarTarget.GetPosition().GetSquawk();
		if (strlen(pssr) == 4 &&
			atoi(pssr) % 100 != 0 &&
			strcmp(pssr, ::mode_s_code) != 0 &&
			strcmp(pssr, this->squawkVFR) != 0 &&
			strcmp(pssr, assr) != 0)
		{
			usedCodes.push_back(pssr);
		}

		sort(usedCodes.begin(), usedCodes.end());
		usedCodes.erase(unique(usedCodes.begin(), usedCodes.end()), usedCodes.end());
	}
	return usedCodes;
}
