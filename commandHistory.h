#pragma once
#include <list>
#include <string>

namespace CommandHistory
{
	/// <summary>
	/// Command history array.
	/// </summary>
	std::list<std::wstring> commandHistoryBuffer;
	unsigned int commandHistorySize;
	const unsigned char DEFAULT_MAXENTRIES = 10;
	/// <summary>
	/// Maximum amount of entries allowed. If this amount is exceeded in the config, MAXENTRIES value will be used instead.
	/// The rationale behind the number: take 2 times the default. User will have a hard time navigating through history without
	/// having some additional tools.
	/// </summary>
	const unsigned char MAXENTRIES = 20; 
	std::list<std::wstring>::const_iterator getFirstRecord();
	std::list<std::wstring>::const_iterator getEndOfList();
	bool isEnabled();
	void clearHistory();
	void appendToHistoryBuffer(std::wstring command);
	void initCommandHistoryModule();
}
