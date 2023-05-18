#include "stdafx.h"
#include "modConfiguration.h"
#include <fstream>
#include <sstream>

extern void announceCapability(const char* capability_name);

void initModConfig()
{
	std::ifstream configFile(MOD_CONFIGURATION_FILENAME, std::ifstream::in);
	std::string buffer;
	announceCapability("modconfig");
	if (!configFile.is_open())
	{
		noConfigMode = true;
		return;
	}
	
	while (std::getline(configFile, buffer))
	{
		//Trim leading spaces
		buffer.erase(0, buffer.find_first_not_of(" "));
		//If line is empty or its first character is an INI comment (;) -- skip the line
		if (buffer.length() == 0 || buffer[0] == ';')
			continue;
		//Debug: print results
		//MessageBoxA(NULL, (std::string("|") + buffer + std::string("|")).c_str(), "testing modconfig", NULL);
	}
	configFile.close();
}