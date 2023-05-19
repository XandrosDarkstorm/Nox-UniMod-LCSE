#pragma once
#include <windows.h>
/// <summary>
/// Returns NumLock activation status
/// </summary>
bool osNumLockStatus()
{
	return LOBYTE(GetKeyState(VK_NUMLOCK)) & 0x1 > 0;
}

bool osIsKeyPressed(int winapiVirtualKey)
{
	return HIWORD(GetKeyState(winapiVirtualKey)) > 0;
}

void osErrorMessageBox(std::string title, std::string message)
{
	MessageBoxA(NULL, message.c_str(), title.c_str(), MB_ICONHAND);
}

/// <summary>
/// Show warning message box.
/// </summary>
/// <param name="title">Message box title</param>
/// <param name="message">Message box text</param>
/// <param name="yesno">If set to true, messagebox will have Yes/No buttons, instead of OK.</param>
/// <returns>WinAPI messagebox response (IDOK/IDYES/IDNO)</returns>
int osWarningMessageBox(std::string title, std::string message, bool yesno = false)
{
	UINT style = MB_ICONEXCLAMATION;
	if (yesno)
		style |= MB_YESNO;
	return MessageBoxA(NULL, message.c_str(), title.c_str(), style);
}
