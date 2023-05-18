#include "stdafx.h"
#include "commandHistory.h"
#include "modConfiguration.h"

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

void CommandHistory::initCommandHistoryModule()
{
	//TODO: X.D> There should be the config information.
	if (noConfigMode)
		commandHistorySize = CommandHistory::MAXENTRIES;
	else
		commandHistorySize = 10;
}