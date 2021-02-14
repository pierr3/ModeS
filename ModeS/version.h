#pragma once
#include <vector>
#include <string>

struct PluginData
{
	const char * PLUGIN_NAME	{ "CCAMS" };
	const char * PLUGIN_VERSION { "1.5.3" };
	const char * PLUGIN_AUTHOR	{ "Pierre Ferran, Oliver Gr�tzmann, Jonas Kuster" };
	const char * PLUGIN_LICENSE { "GPL v3" };
	const char * UPDATE_URL		{ "https://raw.githubusercontent.com/ogruetzmann/ModeS/master/version.txt" };
	const int VERSION_CODE		{ 953 };
};

struct ItemCodes
{
	enum ItemTypes : int
	{
		TAG_ITEM_ISMODES = 501,
		TAG_ITEM_EHS_HDG,
		TAG_ITEM_EHS_ROLL,
		TAG_ITEM_EHS_GS
	};

	enum ItemFunctions : int
	{
		TAG_FUNC_SQUAWK_POPUP = 869,
		TAG_FUNC_ASSIGN_SQUAWK,
		TAG_FUNC_ASSIGN_SQUAWK_AUTO,
		TAG_FUNC_ASSIGN_SQUAWK_MANUAL,
		TAG_FUNC_ASSIGN_SQUAWK_VFR,
		TAG_FUNC_ASSIGN_MODES
	};
};

struct DefaultCodes
{
	const std::vector<std::string> EQUIPEMENT_CODES { "H", "L", "E", "G", "W", "Q", "S" };
	const std::vector<std::string> EQUIPEMENT_CODES_ICAO { "E", "H", "I", "L", "S" };
	const std::vector<std::string> EQUIPEMENT_CODES_EHS { "E", "H", "L", "S" };
	const std::vector<std::string> ICAO_MODES { "EB", "EL", "ET", "ED", "LF", "LH", "LR", "LZ", "EH", "LK", "LO", "LIM", "LIR", "EP", "LD", "LSZR", "LSZB", "LSZG", "LSGC", "LSZH", "LSGG" };
};

static const char * mode_s_code { "1000" };
