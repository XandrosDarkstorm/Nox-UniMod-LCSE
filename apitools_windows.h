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