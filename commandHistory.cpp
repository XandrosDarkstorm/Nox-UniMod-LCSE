#include "stdafx.h"
#include "commandHistory.h"
#include "modConfiguration.h"
#include "apitools_windows.h"

void CommandHistory::appendToHistoryBuffer(std::wstring command)
{
	//If duplicate found - delete it
	commandHistoryBuffer.remove(command);
	//If command history is full -- remove record from the front
	if (commandHistoryBuffer.size() == commandHistorySize)
		commandHistoryBuffer.pop_front();
	//Append command to the command history.
	commandHistoryBuffer.push_back(command);
}

std::list<std::wstring>::const_iterator CommandHistory::getFirstRecord()
{
	return commandHistoryBuffer.cbegin();
}

std::list<std::wstring>::const_iterator CommandHistory::getEndOfList()
{
	return commandHistoryBuffer.cend();
}

void CommandHistory::clearHistory()
{
	commandHistoryBuffer.clear();
}

bool CommandHistory::isEnabled()
{
	return commandHistorySize > 0;
}

void CommandHistory::initCommandHistoryModule()
{
	//TODO: X.D> There should be the config information.
	if (ModConfig::noConfigMode)
		commandHistorySize = DEFAULT_MAXENTRIES;
	else
	{
		if (!ModConfig::getUInt32Value("console.command_history_size", commandHistorySize))
		{
			commandHistorySize = MAXENTRIES;
		}
		if (commandHistorySize > MAXENTRIES)
		{
			osWarningMessageBox("Configuration error", "Command history size is too big (" + std::to_string(commandHistorySize) + " > " + std::to_string(MAXENTRIES) + ").\n\nThe mod will limit the command history to " + std::to_string(MAXENTRIES));
			commandHistorySize = MAXENTRIES;
		}
	}
}