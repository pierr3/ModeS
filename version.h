#pragma once
#include <vector>
#include <string>

struct PluginData
{
	const char * PLUGIN_NAME	{ "CCAMS" };
	const char * PLUGIN_VERSION { "1.7.8" };
	const char * PLUGIN_AUTHOR	{ "Pierre Ferran, Oliver Grützmann, Jonas Kuster" };
	const char * PLUGIN_LICENSE { "GPL v3" };
	const char * UPDATE_URL		{ "https://raw.githubusercontent.com/kusterjs/CCAMS/master/CCAMS/ver.txt" };
	const int VERSION_CODE		{ 977 };
};

struct ItemCodes
{
	enum ItemTypes : int
	{
		TAG_ITEM_ISMODES = 501,
		TAG_ITEM_EHS_HDG,
		TAG_ITEM_EHS_ROLL,
		TAG_ITEM_EHS_GS,
		TAG_ITEM_ERROR_MODES_USE,
		TAG_ITEM_SQUAWK
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

static const char* mode_s_code { "1000" };
static std::vector<const char*> noCorrelationSquawks{ "0000", "1200", "2000", "2200" };

//#define MY_EQUIPEMENT_CODES = const std::vector<std::string> { "H", "L", "E", "G", "W", "Q", "S" };
//#define MODE_S_CODE = "1000";