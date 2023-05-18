#pragma once
#include <list>
#include <string>

namespace CommandHistory
{
	/// <summary>
	/// Command history array.
	/// </summary>
	std::list<std::wstring> commandHistoryBuffer;
	unsigned char commandHistorySize;
	const unsigned char MAXENTRIES = 10;
	std::list<std::wstring>::const_iterator getFirstRecord();
	std::list<std::wstring>::const_iterator getEndOfList();
	bool isEnabled();
	void clearHistory();
	void appendToHistoryBuffer(std::wstring command);
	void initCommandHistoryModule();
}
