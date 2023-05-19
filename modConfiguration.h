#pragma once
#ifdef WITH_MOD_CONFIG
#include <map>

namespace ModConfig
{
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
	enum varRequestResponse
	{
		ERR_NOT_FOUND = 0,
		ERR_INCOMPATIBLE_TYPE = 1,
		ERR_NO_ERROR = 2
	};
	bool noConfigMode = false; //If this set to true, modules MUST use default values!
	int lastError = 2;
	const char* MOD_CONFIGURATION_FILENAME = "unimod-lcse.cfg";
	std::map<std::string, std::string> configTable;
	bool reportConfigError(int line, std::string error_desc);
	void handleUserResponse(bool result);
	void initModConfig();
	bool getUInt32Value(std::string variable_full_name, unsigned int& result);
}
#endif // WITH_MOD_CONFIG
