#include "stdafx.h"
#include <string>
#include "input_info.h"
#include "apitools_windows.h"
#include "commandHistory.h"
#include <cwctype>

struct packetChat
{
	byte pType;
	byte unk01,unk02;
	byte SomeFlag03;
	byte Some08;


};

struct packetCast
{
	byte pType;// =0x53
	//затем идет буффер спеллов
	DWORD Buf[5]; //+1
	byte X;//+15


};
extern void *(__cdecl *noxCallWndProc)(void* Window,int Msg,int A,int B);
extern int (__cdecl *noxDrawGetStringSize) (void *FontPtr, const wchar_t*String,int *Width,int *H,int);
extern int (__cdecl *noxSetRectColorMB) (int);
int (__cdecl *consoleParse)(wchar_t*Str,int Mode);
extern bool justDoNoxCmd;
int *dword_69FE50=(int*) 0x69FE50;

namespace
{
	/// <summary>
	/// *Event* Trigger onConPrint function when server receives a message to its console.
	/// </summary>
	/// <param name="color">Color id of the message</param>
	/// <param name="str">The message itself</param>
	void __cdecl onPrintConsole(int color,wchar_t* str)
	{
		int Top=lua_gettop(L);
		getServerVar("onConPrint");
		if (lua_isfunction(L,-1))
		{
			char conStr[0x208]={0};
			wcstombs(conStr,str,0x208);
			lua_pushstring(L,conStr);
			lua_pushinteger(L,color);
			lua_pcall(L,2,0,0);
		}
		lua_settop(L,Top);
	}

	/// <summary>
	/// An injection function, which attach onPrintConsole function to the message receive code.
	/// </summary>
	int __declspec(naked) onPrintConsoleTrap()
	{
		__asm
		{
			mov eax,[dword_69FE50+0]
			mov eax,[eax+0]
			push eax
			mov eax,[esp+4+8]
			push eax
			mov eax,[esp+8+4]
			push eax
			call onPrintConsole
			add esp,8
			pop eax
			push 450B95h
			ret
		};
	}

	//Console input related variables

	
	const size_t MAX_CONSOLE_COMMAND_LENGTH = 127;
	int conCursorPosition = 0; //Visual placement of the cursor
	int lenStrOld = 0;
	unsigned char conKeyboardModifiers = 0; //3 bits: SHIFT, CTRL, ALT. Meta key is not implemented intentionally.
	/// <summary>
	/// When user issues the "sysop" command, this variable is set to true.
	/// </summary>
	bool conSysopPasswordModeActive = false;
#ifdef CONSOLE_COMMAND_HISTORY
	/// <summary>
	/// When history commands being recalled, current console input is stored here, so user can revert back to it, if they change their mind.
	/// </summary>
	std::wstring conCurrentInput = L"";
	/// <summary>
	/// Is user is looking through the commands history?
	/// </summary>
	bool isShowingCommandHistory = false;
	/// <summary>
	/// Current pointer to the command in the commands history
	/// </summary>
	std::list<std::wstring>::const_iterator conCurrentHistoryRecord = CommandHistory::getEndOfList();
#endif
	//End of console input related variables

	/// <summary>
	/// Find the beginning of the word
	/// </summary>
	/// <param name="SourceStr">Original string</param>
	/// <param name="CursorPosition">Console cursor position</param>
	/// <param name="LookToLeft">If true -- look to the left from cursor position; otherwise -- to the right</param>
	/// <returns>Returns position for the console cursor.</returns>
	size_t findWordCloseToCursor(std::wstring& SourceStr, size_t CursorPosition, bool LookToLeft)
	{
		int pos = CursorPosition;
		if (LookToLeft)
		{
			//Just to be extra safe -- if we are at the start, then no need in processing.
			if (CursorPosition < 2)
				return 0;

			// If character to the left is a space -- skip spaces, until find first non-space
			if (SourceStr[--pos] == L' ')
			{
				for (; pos > -1; --pos)
				{
					if (SourceStr[pos] != L' ')
						break;
				}
			}

			// Look to the first space character. The next character will be the result.
			for (; pos > -1; --pos)
			{
				if (SourceStr[pos] == L' ')
					break;
			}
			return pos + 1;
		}
		else
		{
			//Just to be extra safe -- if we are at the end, then no need in processing.
			if (CursorPosition == SourceStr.length())
				return CursorPosition;

			// If character to the right is not a space -- skip characters, until find first space character.
			if (SourceStr[++pos] != L' ')
			{
				for (; pos < SourceStr.length(); ++pos)
				{
					if (SourceStr[pos] == L' ')
						break;
				}
			}

			// Look to the first non-space character -- it will be the result
			for (; pos < SourceStr.length(); ++pos)
			{
				if (SourceStr[pos] != L' ')
					break;
			}
			return pos;
		}
	}

	/// <summary>
	/// Strips spaces both from left and right side in a given wstring.
	/// </summary>
	/// <param name="SourceStr">A string to perform operation on</param>
	void stripWhitespaces(std::wstring& SourceStr)
	{
		SourceStr.erase(0, SourceStr.find_first_not_of(L" "));
		SourceStr.erase(SourceStr.find_last_not_of(L" ") + 1);
	}

	void toLowercase(std::wstring& SourceStr)
	{
		//SourceStr = std::towlower(SourceStr.c_str());
	}

	/// <summary>
	/// Signal the game to process the event for the console command as is.
	/// </summary>
	int (__cdecl *consoleEditProc)(void* Window,int Msg,int A,int B);
	/// <summary>
	/// Signal the game to process the console command.
	/// </summary>
	void (__cdecl *consoleProcFn)();
	/// <summary>
	/// This function controlls the cursor and the text input of the console.
	/// </summary>
	/// <param name="Window">A window structure.</param>
	/// <param name="Msg">Message console textbox receives. 0x11 - MouseEnter 0x12 - MouseLeave, 0x15 - keyboard input</param>
	/// <param name="keyCode">Keycode in a strange form</param>
	/// <param name="keyState">Key status. 2 - pressed, 1 - released</param>
	int __cdecl consoleEditProcNew(void* Window,int Msg,int keyCode,int keyState)
	{
		//TODO: X.D> the comment below is garbage. Consult KirConjurer if this is recorded somewhere.
		//conPrintI((std::string("MSG: ") + std::to_string(Msg)).c_str());
		//conPrintI((std::string("A: ") + std::to_string(keyCode)).c_str());
		//conPrintI((std::string("B: ") + std::to_string(keyState)).c_str());
		/*
		* 
		* keyCode is not only a key code. It contains x (first 16 bit) and y (second 16 bit) coordinates
		* of the mouse when it comes to mouse events.
		* 
		* keyState is only a key state.
		*/

		//We do not support mouse operations yet.
		if (Msg != noxWindowEvents::KEYBOARD_INPUT)
			return consoleEditProc(Window, Msg, keyCode, keyState);

		//Keyboard input processing. 0x20 -- input string address
		BYTE* nox_console_input = (BYTE*) Window;
		nox_console_input = *((BYTE**)(nox_console_input + 0x20));
		
		/*
		* Numpad fixes.
		* Nox does not care about NumLock status and process buttons as numbers.
		* The code below fixes that by converting keyCode to the proper button.
		*/
		if (!osNumLockStatus())
		{
			//NumLock is not active -- convert keys into correct representation.
			keyCode = keyboardConvertNumlockNumbers(keyCode);
		}

		if (keyState == noxKeyboardEvents::KEY_RELEASE)
		{
			switch (keyCode)
			{
				case noxKeyboardKeys::KBD_SHIFT_LEFT:
				case noxKeyboardKeys::KBD_SHIFT_RIGHT:
					conKeyboardModifiers &= !keyboardModifiers::KBDMOD_SHIFT;
				break;

				case noxKeyboardKeys::KBD_CTRL_LEFT:
				case noxKeyboardKeys::KBD_CTRL_RIGHT:
					conKeyboardModifiers &= !keyboardModifiers::KBDMOD_CTRL;
				break;

				case noxKeyboardKeys::KBD_ALT_LEFT:
				case noxKeyboardKeys::KBD_ALT_RIGHT:
					conKeyboardModifiers &= !keyboardModifiers::KBDMOD_ALT;
				break;
			}
			return consoleEditProc(Window, Msg, keyCode, keyState);
		}
		else if (keyState == noxKeyboardEvents::KEY_PRESS)
		{
			std::wstring console_cmd((const wchar_t*) nox_console_input);
			bool passInputToNoxEngine = false; //Should Nox process the input?

			/*
			X.D> I do not know the codes for all keyboard buttons.
			If somebody wants to help -- please, assign debug_kbd to something in the game and send KEYCODE RAW values
			and their associated buttons to "Issues" of the Nox LCSE repository.
			*/
			int top = lua_gettop(L);
			getServerVar("debug_kbd");
			if (!lua_isnil(L, -1))
			{
				conPrintI((std::string("KEYCODE RAW: ") + std::to_string(keyCode)).c_str());
			}
			lua_settop(L, top);

			/*
			* Double check the key modifiers.
			* This is required to fix situation when key modifiers get stuck in
			* the pressed position during window switch (Nox quirk).
			* GetKeyState is not reliable by itself (sometimes it lags a bit, comparing to capture from Nox directly)
			*/
			if (conKeyboardModifiers & keyboardModifiers::KBDMOD_SHIFT && !osIsKeyPressed(VK_SHIFT))
				conKeyboardModifiers &= !keyboardModifiers::KBDMOD_SHIFT;
			if (conKeyboardModifiers & keyboardModifiers::KBDMOD_CTRL && !osIsKeyPressed(VK_CONTROL))
				conKeyboardModifiers &= !keyboardModifiers::KBDMOD_CTRL;
			if (conKeyboardModifiers & keyboardModifiers::KBDMOD_ALT && !osIsKeyPressed(VK_MENU))
				conKeyboardModifiers &= !keyboardModifiers::KBDMOD_ALT;
			
			//Process the key.
			switch (keyCode)
			{
				//Key modifiers. We don't replace the behavior - we only need to track modifiers.
				case noxKeyboardKeys::KBD_SHIFT_LEFT:
				case noxKeyboardKeys::KBD_SHIFT_RIGHT:
					conKeyboardModifiers |= keyboardModifiers::KBDMOD_SHIFT;
					passInputToNoxEngine = true;
				break;

				case noxKeyboardKeys::KBD_CTRL_LEFT:
				case noxKeyboardKeys::KBD_CTRL_RIGHT:
					conKeyboardModifiers |= keyboardModifiers::KBDMOD_CTRL;
					passInputToNoxEngine = true;
				break;

				case noxKeyboardKeys::KBD_ALT_LEFT:
				case noxKeyboardKeys::KBD_ALT_RIGHT:
					conKeyboardModifiers |= keyboardModifiers::KBDMOD_ALT;
					passInputToNoxEngine = true;
				break;

				//Cursor movement
				case noxKeyboardKeys::KBD_LEFT:
					if (conCursorPosition > 0)
					{
						if (conKeyboardModifiers & keyboardModifiers::KBDMOD_CTRL)
							conCursorPosition = findWordCloseToCursor(console_cmd, conCursorPosition, true);
						else
							conCursorPosition--;
					}
				break;
					
				case noxKeyboardKeys::KBD_RIGHT:
					if (conCursorPosition < console_cmd.size())
					{
						if (conKeyboardModifiers & keyboardModifiers::KBDMOD_CTRL)
							conCursorPosition = findWordCloseToCursor(console_cmd, conCursorPosition, false);
						else
							conCursorPosition++;
					}
				break;

				case noxKeyboardKeys::KBD_HOME:
					conCursorPosition = 0;
				break;

				case noxKeyboardKeys::KBD_END:
					conCursorPosition = console_cmd.size();
				break;

				//Text editing
				case noxKeyboardKeys::KBD_DELETE:
					//Only work when we are not at the end.
					if (conCursorPosition != console_cmd.size())
					{
						int cut_size = 1;
						if (conKeyboardModifiers & keyboardModifiers::KBDMOD_CTRL)
							cut_size = findWordCloseToCursor(console_cmd, conCursorPosition, false) - conCursorPosition;
						console_cmd.erase(conCursorPosition, cut_size);
						noxCallWndProc(Window, 0x401E, (int)console_cmd.c_str(), -1);
						lenStrOld -= cut_size;
#ifdef CONSOLE_COMMAND_HISTORY
						//If user change the input -- we no longer work with history
						conCurrentHistoryRecord = CommandHistory::getEndOfList();
#endif
					}						
				break;

				case noxKeyboardKeys::KBD_BACKSPACE:
					//Only work when we are not at the start.
					if (console_cmd.size() > 0 && conCursorPosition > 0)
					{
						int cut_size = 1;
						if (conKeyboardModifiers & keyboardModifiers::KBDMOD_CTRL)
							cut_size = conCursorPosition - findWordCloseToCursor(console_cmd, conCursorPosition, true);
						console_cmd.erase(conCursorPosition - cut_size, cut_size);
						noxCallWndProc(Window, 0x401E, (int)console_cmd.c_str(), -1);
						conCursorPosition -= cut_size;
						lenStrOld -= cut_size;
#ifdef CONSOLE_COMMAND_HISTORY
						//If user change the input -- we no longer work with history
						conCurrentHistoryRecord = CommandHistory::getEndOfList();
#endif
					}						
				break;

				case noxKeyboardKeys::KBD_V:
					if (conKeyboardModifiers & keyboardModifiers::KBDMOD_CTRL)
					{
						/*
						* Clipboard paste. Only supports ANSI encoded plain text.
						*/

						if (console_cmd.length() == MAX_CONSOLE_COMMAND_LENGTH)
							break; //Do not worry about pasting - there is no space for it anyway.

						if (!IsClipboardFormatAvailable(CF_TEXT))
							break;
								
						BOOL is_clipbrd_opened = OpenClipboard(NULL);
						if (is_clipbrd_opened)
						{
							HANDLE clipboard_obj = GetClipboardData(CF_TEXT);
							if (clipboard_obj)
							{
								LPTSTR clipboard_text = (LPTSTR)GlobalLock(clipboard_obj);
								int clipboard_text_length = 0;
								if (clipboard_text)
								{
									/*
									* Sanitize the clipboard:
									* Only 1 line of text is allowed.
									* Control characters are not allowed.
									*/
									for (; clipboard_text_length < strlen(clipboard_text) &&
										clipboard_text_length + console_cmd.size() < MAX_CONSOLE_COMMAND_LENGTH; ++clipboard_text_length)
									{
										//If character code is below the first printable character code (space)
										if (clipboard_text[clipboard_text_length] < 32)
											break;
									}

									if (clipboard_text_length > 0)
									{
										wchar_t *clipboardTextWide = new wchar_t[clipboard_text_length + 1];
										mbstowcs(clipboardTextWide, clipboard_text, clipboard_text_length);
										clipboardTextWide[clipboard_text_length] = 0;
										console_cmd.insert(conCursorPosition, clipboardTextWide);
										noxCallWndProc(Window, 0x401E, (int)console_cmd.c_str(), -1);
										conCursorPosition += clipboard_text_length;
										lenStrOld += clipboard_text_length;
									}										
								}
								GlobalUnlock(clipboard_obj);
							}
							CloseClipboard();
						}
						else
						{
							conPrintI("ERROR: Failed to open clipboard!");
						}
					}
					else
						passInputToNoxEngine = true;
				break;

				case noxKeyboardKeys::KBD_ESC:
					if (console_cmd.length() > 0)
					{
						//Clear console input textbox.
						console_cmd = L"";
						noxCallWndProc(Window, 0x401E, (int)console_cmd.c_str(), -1);
						conCursorPosition = 0;
						lenStrOld = 0;
#ifdef CONSOLE_COMMAND_HISTORY
						//If user change the input -- we no longer work with history
						conCurrentHistoryRecord = CommandHistory::getEndOfList();
#endif
					}
					else //If console input textbox is clean -- close the console (which is default Nox behavior)
						passInputToNoxEngine = true;
				break;

				case noxKeyboardKeys::KBD_UP:
#ifdef CONSOLE_COMMAND_HISTORY
					if (conCurrentHistoryRecord == CommandHistory::getFirstRecord())
					{
						/*
						* Notify that we have no more history records.
						* The idea is to not wrap around history. The reason for that is because we store
						* user's input before starting history recall.
						*/
						conPrintI("No more commands stored.");
						break;
					}
					
					//Store current console input so we can go back to it later.
					if (conCurrentHistoryRecord == CommandHistory::getEndOfList())
						conCurrentInput = console_cmd;
					
					//Change the command in the command input textbox.
					console_cmd = *(--conCurrentHistoryRecord);
					noxCallWndProc(Window, 0x401E, (int)console_cmd.c_str(), -1);
					lenStrOld = conCursorPosition = console_cmd.length();
#endif
				break;

				case noxKeyboardKeys::KBD_DOWN:
#ifdef CONSOLE_COMMAND_HISTORY
					//We can only advance if there is a next record
					if (conCurrentHistoryRecord != CommandHistory::getEndOfList())
					{
						//Restore user's input, if user reached the EOL mark.
						if (++conCurrentHistoryRecord == CommandHistory::getEndOfList())
						{
							console_cmd = conCurrentInput;
						}
						else
						{
							console_cmd = *conCurrentHistoryRecord;
						}
						//Change the command in the command input textbox.
						noxCallWndProc(Window, 0x401E, (int)console_cmd.c_str(), -1);
						lenStrOld = conCursorPosition = console_cmd.length();
					}
#endif
				break;
				//Execute command
				case noxKeyboardKeys::KBD_ENTER:
				case noxKeyboardKeys::KBD_KP_ENTER:
					//Strip spaces from the sent command. Nox does that in its code, so we have to conform.
					stripWhitespaces(console_cmd);
#ifdef CONSOLE_COMMAND_HISTORY
					if (console_cmd.length() > 0 && !conSysopPasswordModeActive)
					{
						CommandHistory::appendToHistoryBuffer(console_cmd);
					}
					conCurrentHistoryRecord = CommandHistory::getEndOfList();
#endif
					//Process sysop input flag
					if (!conSysopPasswordModeActive)
					{
						if (console_cmd == L"sysop")
							conSysopPasswordModeActive = true;
					}
					else
						conSysopPasswordModeActive = false;

					//Reset the cursor position and send command to the game
					conCursorPosition = 0;
					lenStrOld = 0;
					consoleProcFn();
				break;

				//Let Nox deal with other keys.
				default:
					passInputToNoxEngine = true;
			}

			if (passInputToNoxEngine)
			{
				/*
				* If we input something -- correctly refresh the Nox console.
				* Nox can only input at the end of the command. The code below moves the character to the cursor position.
				*/
				int oldStrLen = console_cmd.length();
				consoleEditProc(Window, Msg, keyCode, keyState);
				console_cmd = (const wchar_t*)nox_console_input; //Take the resulting buffer after Nox processing.
				if (console_cmd.length() > oldStrLen)
				{
					console_cmd.insert(conCursorPosition++, 1, console_cmd.at(console_cmd.length() - 1));
					console_cmd.erase(oldStrLen + 1);
					noxCallWndProc(Window, 0x401E, (int)console_cmd.c_str(), -1);

#ifdef CONSOLE_COMMAND_HISTORY
					//If user change the input -- we no longer work with history
					conCurrentHistoryRecord = CommandHistory::getEndOfList();
#endif
				}
			}
			return 1;
		}
		else
		{
			conPrintI((std::string("WARNING: Unknown keyboard event ") + std::to_string(keyState)).c_str());
			return consoleEditProc(Window, Msg, keyCode, keyState);
		}		
	}

	int __cdecl conGetWidthForCur(BYTE *Window,BYTE *Wdd)
	{
		Window=*((BYTE**)(Window+0x20));
		std::wstring s((const wchar_t*)Window);
		s.erase(conCursorPosition,s.size());
		Wdd=*((BYTE**) (Wdd+0x0c8));
		int Width;
		int *PWidth=&Width;
		noxDrawGetStringSize((void*)Wdd,(wchar_t*)s.c_str(),PWidth,0,0);
		return *PWidth+5;
	}


	void __declspec(naked) consoleEditDraw()
	{
		__asm
		{	

			call    noxSetRectColorMB
			push	eax
			mov		eax,[edi+18Ch]
			test	eax,eax
			jz		l2
			push	ebx
			mov		ebx,0x69FE44
			cmp		eax,[ebx+0]
			jnz		l1
			push	esi
			push	edi
			call	conGetWidthForCur
			add		esp,8
			pop		ebx
			mov		ebx,eax	
			jmp		l2
			l1:
			pop		ebx
			l2:
			pop		eax
			push	0x4884CA  
			ret
		};
	}

	int conExecL(lua_State *L)
	{
		//TODO: X.D> We need to fix this function
		if ((lua_type(L,1)!=LUA_TSTRING) )
		{
			lua_pushstring(L,"[conExec,type_arg1] -> argument must be a string.");
			lua_error_(L);
		}
		wchar_t Buf[MAX_CONSOLE_COMMAND_LENGTH];
		if(mbstowcs(Buf,lua_tostring(L,1), MAX_CONSOLE_COMMAND_LENGTH) == MAX_CONSOLE_COMMAND_LENGTH)
			return 0;
		justDoNoxCmd=true;
		lua_pushinteger(L,consoleParse(Buf,1));
		justDoNoxCmd=false;
		return 1;

	}	
}
extern void InjectAddr(DWORD Addr,void *Fn);
extern void InjectJumpTo(DWORD Addr,void *Fn);

void consoleInit()
{

	ASSIGN(consoleEditProc,0x00450F40);
	ASSIGN(consoleProcFn,0x00450FD0);
	ASSIGN(consoleParse,0x00443C80);

	InjectAddr(0x00450E4C+1,&consoleEditProcNew);
	InjectJumpTo(0x004884C5 ,&consoleEditDraw); // Меняем размеры
	InjectJumpTo(0x450B90 ,&onPrintConsoleTrap);

	registerserver("conExec",&conExecL);
	announceCapability("console_mods");
}