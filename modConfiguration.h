#pragma once
#ifdef WITH_MOD_CONFIG
#include <map>

bool noConfigMode = false; //If this set to true, modules MUST use default values!
const char* MOD_CONFIGURATION_FILENAME = "unimod-lcse.cfg";
std::map<char*, char*> params;
void initModConfig();
#endif // WITH_MOD_CONFIG
