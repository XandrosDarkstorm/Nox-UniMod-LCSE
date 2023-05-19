#include "stdafx.h"
#include "noxManifest.h"
#include "modConfiguration.h"
#include "apitools_windows.h"
#include <fstream>
#include <sstream>

/// <summary>
/// Shows an error message and asks user whether they prefer to work in "no config" mode or close the app.
/// </summary>
/// <param name="line">Line number in the configuration file.</param>
/// <param name="error_desc">Error description</param>
bool ModConfig::reportConfigError(int line, std::string error_desc)
{
	std::string msgbuffer = "Error reading configuration file '";
	msgbuffer.append(MOD_CONFIGURATION_FILENAME);
	msgbuffer.append("'.\nLine " + std::to_string(line) + ": " + error_desc);
	msgbuffer.append("\n\nWould you like to continue without loading configuration file? If yes, mod will start in \"non-configured\" mode and use default settings.");
	return osWarningMessageBox("Config read error", msgbuffer, true) == IDYES;
}

/// <summary>
/// Switch mod config to "no config" mode or exit the application
/// </summary>
/// <param name="result">Set to true, if you want to switch to "no config" mode.</param>
void ModConfig::handleUserResponse(bool result)
{
	if (result)
		noConfigMode = true;
	else
		exit(1);
}

void ModConfig::initModConfig()
{
	std::ifstream configFile(MOD_CONFIGURATION_FILENAME, std::ifstream::in);
	std::string buffer;
	std::string category;
	std::string variablename;
	std::string variablevalue;
	int linepos = 0;
	announceCapability("modconfig");
	if (!configFile.is_open())
	{
		noConfigMode = true;
		return;
	}
	
	while (std::getline(configFile, buffer))
	{
		++linepos;
		//Trim leading spaces
		buffer.erase(0, buffer.find_first_not_of(" "));
		//If line is empty or its first character is an INI comment (;) -- skip the line
		if (buffer.length() == 0 || buffer[0] == ';')
			continue;

		if (buffer[0] == '[')
		{
			//New category!
			if (buffer.find_first_of("]") == buffer.npos)
			{
				handleUserResponse(reportConfigError(linepos, "']' character is missing in the category definition."));
				break;
			}
			category = buffer.substr(1, buffer.find_first_of("]") - 1);
			if (category == "")
			{
				handleUserResponse(reportConfigError(linepos, "Category cannot be an empty string."));
				break;
			}
		}
		else
		{
			//A variable definition
			if (category == "")
			{
				handleUserResponse(reportConfigError(linepos, "A category definition is missing before that line."));
				break;
			}
			if (buffer.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") != 0)
			{
				handleUserResponse(reportConfigError(linepos, "Variable name must start with a letter character."));
				break;
			}

			size_t bufferpos = buffer.find_first_of('=');
			if (bufferpos == buffer.npos)
			{
				handleUserResponse(reportConfigError(linepos, "Line must be written in format: 'variablename = value'."));
				break;
			}
			variablename = buffer.substr(0, bufferpos - 1);
			//Strip trailing spaces
			variablename = variablename.substr(0, variablename.find_first_of(' ') - 1);
			if (bufferpos + 1 == buffer.length())
			{
				//Treat as an empty string
				configTable[category + "." + variablename] = "";
			}
			else
			{
				//Store the variable value
				variablevalue = buffer.substr(bufferpos + 1);
				//Strip trailing and leading spaces
				stripWhitespaces(variablevalue);
				configTable[category + "." + variablename] = variablevalue;
			}
		}
	}
	configFile.close();
}

/// <summary>
/// Retrieve an unsigned int value from configuration table.
/// </summary>
/// <param name="variable_full_name">Variable name in style "category.name"</param>
/// <param name="result">The output variable.</param>
/// <returns>True, if value was retrieved successfully. Otherwise a false is return and lastError variable is set</returns>
bool ModConfig::getUInt32Value(std::string variable_full_name, unsigned int &result)
{
	size_t chars_processed = 0;
	int preresult = 0;
	std::map<std::string, std::string>::iterator entry = configTable.find(variable_full_name);
	if (entry == configTable.end())
	{
		lastError = varRequestResponse::ERR_NOT_FOUND;
		return false;
	}
	preresult = std::stoul((*entry).second, &chars_processed);
	if (chars_processed != (*entry).second.length())
	{
		lastError = varRequestResponse::ERR_INCOMPATIBLE_TYPE;
		return false;
	}
	result = preresult;
	lastError = varRequestResponse::ERR_NO_ERROR;
	return true;
}