#include "stdafx.h"
#include "version.h"

const char * PLUGIN_NAME	{ "Mode S PlugIn" };
const char * PLUGIN_VERSION { "1.3.5e32" };
const char * PLUGIN_AUTHOR	{ "Pierre Ferran / Oliver Grützmann" };
const char * PLUGIN_LICENSE { "GPL v3" };

const char * UPDATE_URL		{ "http://www.cherryx.de/modes/modes.txt" };
const int VERSION_CODE		{ 905 };

const char * mode_s_code { "1000" };

const std::vector<std::string> EQUIPEMENT_CODES { "H", "L", "E", "G", "W", "Q", "S" };
const std::vector<std::string> ICAO_MODES { "EB", "EL", "ET", "ED", "LF", "EH", "LK", "LO", "LIM", "LIR" };
