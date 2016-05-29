#pragma once
#include <vector>
#include <string>

struct PluginData
{
	const char * PLUGIN_NAME	{ "Mode S PlugIn" };
	const char * PLUGIN_VERSION { "1.3.5e32" };
	const char * PLUGIN_AUTHOR	{ "Pierre Ferran / Oliver Grützmann" };
	const char * PLUGIN_LICENSE { "GPL v3" };
	const char * UPDATE_URL		{ "http://www.cherryx.de/modes/modes.txt" };
	const int VERSION_CODE		{ 905 };
};

struct ItemCodes
{
	enum ItemTypes : int
	{
		TAG_ITEM_ISMODES = 501,
		TAG_ITEM_MODESHDG,
		TAG_ITEM_MODESROLLAGL,
		TAG_ITEM_MODESREPGS
	};

	enum ItemFunctions : int
	{
		TAG_FUNC_ASSIGNMODES = 869,
		TAG_FUNC_ASSIGNMODEAS
	};
};

struct DefaultCodes
{
	const std::vector<std::string> EQUIPEMENT_CODES { "H", "L", "E", "G", "W", "Q", "S" };
	const std::vector<std::string> ICAO_MODES { "EB", "EL", "ET", "ED", "LF", "EH", "LK", "LO", "LIM", "LIR" };
};

static constexpr char * mode_s_code { "1000" };
