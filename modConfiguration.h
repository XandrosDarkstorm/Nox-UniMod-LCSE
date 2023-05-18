#pragma once
#ifdef WITH_MOD_CONFIG
#include <map>

struct modConfigEntry
{
	int datatype;
	void* data_ptr;
};
enum dataTypeEntry
{
	CFG_TYPE_STRING = 0,
	CFG_TYPE_INTEGER = 1,
	CFG_TYPE_BOOLEAN = 2,
	CFG_TYPE_FLOAT = 3
};
bool noConfigMode = false; //If this set to true, modules MUST use default values!
const char* MOD_CONFIGURATION_FILENAME = "unimod-lcse.cfg";
std::map<char*, char*> params;
void initModConfig();
#endif // WITH_MOD_CONFIG
