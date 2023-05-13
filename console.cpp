#include "stdafx.h"
#include <string>
#include "string"
#include "input_info.h"

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

	const size_t MAX_CONSOLE_COMMAND_LENGTH = 127;
	int conCursorPosition = 0;
	int lenStrOld = 0;
	unsigned char conKeyboardModifiers = 0; //3 bits: SHIFT, CTRL, ALT

	int (__cdecl *consoleEditProc)(void* Window,int Msg,int A,int B);
	void (__cdecl *consoleProcFn)();
	/// <summary>
	/// This function controlls the cursor and the text input of the console.
	/// </summary>
	/// <param name="Window"></param>
	/// <param name="Msg">Message console textbox receives. 0x11 - MouseEnter 0x12 - MouseLeave, 0x15 - keyboard input</param>
	/// <param name="keyCode">Keycode in a strange form</param>
	/// <param name="keyState">Key status. 2 - pressed, 1 - released</param>
	int __cdecl consoleEditProcNew(void* Window,int Msg,int keyCode,int keyState)
	{
		BYTE *P=(BYTE*)Window;
		//TODO: the comment below is garbage. Consult KirConjurer if this is recorded somewhere.
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
		if (Msg == noxWindowEvents::KEYBOARD_INPUT)
		{
			P=*((BYTE**)(P+0x20));//Capture string data
			lua_getglobal(L, "debug_kbd");
			if (lua_type(L, -1) != LUA_TNIL)
			{
				conPrintI((std::string("KEYSTATE: ") + std::to_string(keyState)).c_str());
				conPrintI((std::string("KEYCODE RAW: ") + std::to_string(keyCode)).c_str());
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
				
			}
			else if (keyState == noxKeyboardEvents::KEY_PRESS)
			{
				std::wstring console_cmd((const wchar_t*) P);
				int cmd_length=console_cmd.size();
				bool skipNoxProcessing = true;

				/*
				* Double check the key modifiers.
				* This is required to fix situation when key modifiers get stuck in
				* the pressed position during window switch (Nox quirk).
				* GetKeyState is not reliable by itself (sometimes it lags a bit, comparing to capture from Nox directly)
				*/
				if (conKeyboardModifiers & keyboardModifiers::KBDMOD_SHIFT && !HIWORD(GetKeyState(VK_SHIFT)))
					conKeyboardModifiers &= !keyboardModifiers::KBDMOD_SHIFT;
				if (conKeyboardModifiers & keyboardModifiers::KBDMOD_CTRL && !HIWORD(GetKeyState(VK_CONTROL)))
					conKeyboardModifiers &= !keyboardModifiers::KBDMOD_CTRL;
				if (conKeyboardModifiers & keyboardModifiers::KBDMOD_ALT && !HIWORD(GetKeyState(VK_MENU)))
					conKeyboardModifiers &= !keyboardModifiers::KBDMOD_ALT;

				//Process the key
				switch (keyCode)
				{
					//Key modifiers. We don't replace the behavior - we only need to track modifiers.
					case noxKeyboardKeys::KBD_SHIFT_LEFT:
					case noxKeyboardKeys::KBD_SHIFT_RIGHT:
						conKeyboardModifiers |= keyboardModifiers::KBDMOD_SHIFT;
						skipNoxProcessing = false;
					break;

					case noxKeyboardKeys::KBD_CTRL_LEFT:
					case noxKeyboardKeys::KBD_CTRL_RIGHT:
						conKeyboardModifiers |= keyboardModifiers::KBDMOD_CTRL;
						skipNoxProcessing = false;
					break;

					case noxKeyboardKeys::KBD_ALT_LEFT:
					case noxKeyboardKeys::KBD_ALT_RIGHT:
						conKeyboardModifiers |= keyboardModifiers::KBDMOD_ALT;
						skipNoxProcessing = false;
					break;

					//Cursor movement
					case noxKeyboardKeys::KBD_LEFT:
						if (conCursorPosition > 0)
							conCursorPosition--;
					break;
					case noxKeyboardKeys::KBD_RIGHT:
						if (conCursorPosition < cmd_length)
							conCursorPosition++;
					break;

					case noxKeyboardKeys::KBD_HOME:
						conCursorPosition = 0;
					break;

					case noxKeyboardKeys::KBD_END:
						conCursorPosition = cmd_length;
					break;

					//Text editing
					case noxKeyboardKeys::KBD_DELETE:
						//Only work when we are not at the end.
						if (conCursorPosition != cmd_length)
						{
							console_cmd.erase(conCursorPosition, 1);
							noxCallWndProc(Window, 0x401E, (int)console_cmd.c_str(), -1);
							lenStrOld--;
						}						
					break;

					case noxKeyboardKeys::KBD_BACKSPACE:
						//Only work when we are not at the start.
						if (cmd_length > 0 && conCursorPosition > 0)
						{
							console_cmd.erase(conCursorPosition - 1, 1);
							noxCallWndProc(Window, 0x401E, (int)console_cmd.c_str(), -1);
							conCursorPosition--;
							lenStrOld--;
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
							{
								conPrintI("No supported text to paste!");
								break;
							}
								
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
										* Sanitize the clipboard.
										* CRLF characters are not allowed (only 1 line of text is supported).
										* Filter out tabs and other control characters.
										*/
										//conPrintI(std::to_string(strlen(clipboard_text)).c_str());
										//conPrintI("==CHARS==");
										for (; clipboard_text_length < strlen(clipboard_text) && clipboard_text_length + cmd_length < MAX_CONSOLE_COMMAND_LENGTH; ++clipboard_text_length)
										{
											//conPrintI(std::to_string(clipboard_text[clipboard_text_length]).c_str());
											if (clipboard_text[clipboard_text_length] == 0)
												break;
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
							skipNoxProcessing = false;
					break;

					//Command history
					case noxKeyboardKeys::KBD_UP:
						conPrintI("History scroll UP");
						/*int Top = lua_gettop(L); // запоминаем что в начале
						lua_getglobal(L, "conStr");
						if (lua_type(L, -1) != LUA_TTABLE) // если там нил например
						{
							lua_settop(L, Top);
							return 1;
						}
						lua_getfield(L, -1, "lastItem");
						int lastI = lua_tointeger(L, -1); // дл€ удобства
						lua_getfield(L, -2, "lastStr");
						int lastS = lua_tointeger(L, -1);
						lastS--;
						if (lastS < 1) lastS = luaL_getn(L, -3);
						if (lastS != lastI)
						{
							//lastS(старый) lastI [таблица]
							lua_pushinteger(L, lastS);
							lua_setfield(L, -4, "lastStr");
						}
						lua_rawgeti(L, -3, lastS);
						if (lua_type(L, -1) != LUA_TSTRING) // если там не строка а нил например
						{
							lua_settop(L, Top);
							return 1;
						}
						const char* V = lua_tostring(L, -1);
						lua_pop(L, 3);
						wchar_t* W = (wchar_t*)P;
						mbstowcs(W, V, 300);
						noxCallWndProc(Window, 0x401E, (int)(W), -1);
						W[strlen(V)] = 0;/// на вс€кий случай припишем 0
						conCursorPosition = strlen(V); // ѕомещаем в конец курсорчик
						lenStrOld = strlen(V);
						lua_settop(L, Top);*/
					break;

					case noxKeyboardKeys::KBD_DOWN:
						conPrintI("History scroll down");
						/*int Top = lua_gettop(L); // запоминаем что в начале
						lua_getglobal(L, "conStr");
						if (lua_type(L, -1) != LUA_TTABLE) // если там нил например
						{
							lua_settop(L, Top);
							return 1;
						}
						lua_getfield(L, -1, "lastItem");
						int lastI = lua_tointeger(L, -1); // дл€ удобства
						lua_getfield(L, -2, "lastStr");
						int lastS = lua_tointeger(L, -1);
						lastS++;
						if (lastS > 50 || lastS > luaL_getn(L, -3)) lastS = 1;
						if (lastS != lastI)
						{
							//lastS(старый) lastI [таблица]
							lua_pushinteger(L, lastS);
							lua_setfield(L, -4, "lastStr");
						}
						lua_rawgeti(L, -3, lastS);
						if (lua_type(L, -1) != LUA_TSTRING) // если там не строка а нил например
						{
							lua_settop(L, Top);
							return 1;
						}
						const char* V = lua_tostring(L, -1);
						lua_pop(L, 3);
						wchar_t* W = (wchar_t*)P;
						mbstowcs(W, V, 300);
						noxCallWndProc(Window, 0x401E, (int)(W), -1);
						W[strlen(V)] = 0;// на вс€кий случай припишем 0
						conCursorPosition = strlen(V); // курсор в конец
						lenStrOld = strlen(V);
						lua_settop(L, Top);*/
					break;

					//Execute command
					case noxKeyboardKeys::KBD_ENTER:
					case noxKeyboardKeys::KBD_KP_ENTER:
						conPrintI("History is not available yet.");
						conCursorPosition = 0; // курсор в начало
						lenStrOld = 0;
						consoleProcFn();// ¬ызываем обработку
					break;
					default:
						skipNoxProcessing = false;
				}
				//Remove after testing!
				/*std::string debug_modifiers;

				if (conKeyboardModifiers & keyboardModifiers::KBDMOD_SHIFT)
					debug_modifiers += "[SHIFT]";
				if (conKeyboardModifiers & keyboardModifiers::KBDMOD_CTRL)
					debug_modifiers += "[CTRL]";
				if (conKeyboardModifiers & keyboardModifiers::KBDMOD_ALT)
					debug_modifiers += "[ALT]";
				if (HIWORD(GetKeyState(VK_SHIFT)))
					debug_modifiers += "[SHIFT]";
				if (HIWORD(GetKeyState(VK_CONTROL)))
					debug_modifiers += "[CTRL]";
				if (HIWORD(GetKeyState(VK_MENU)))
					debug_modifiers += "[ALT]";
				conPrintI(debug_modifiers.c_str());*/
				//debug. how long is the command
				conPrintI(std::to_string(console_cmd.length()).c_str());
				
				if (skipNoxProcessing)
					return 1; //Prevent additional processing by Nox
				
				/*if (keyCode==0x1C) // нажали энтер?
				{
					int Top=lua_gettop(L); // запоминаеем что в начале
					char string[300]="";
					wchar_t *W=(wchar_t*)P;
					int Len=wcslen(W);
					wcstombs(string,W,Len);
					lua_getglobal(L,"conStr");
					if (lua_type(L,-1)!=LUA_TTABLE)
					{
						lua_newtable(L);
						lua_pushvalue(L,-1);
						lua_setglobal(L,"conStr");
					}
					if (Len==0) 
					{
						lua_settop(L,Top);
						consoleProcFn();// ¬ызываем обработку
						return 1;
					}
					lua_getfield(L,-1,"lastItem"); // достаем последний элемент 
					int lastI=lua_tointeger(L,-1)+1;
					if (lastI>50)
						lastI=1;
					lua_pop(L,1);
					lua_pushinteger(L,lastI);
					lua_setfield(L,-2,"lastItem"); 

					lua_pushstring(L,string);
					lua_rawseti(L,-2,lastI-1);

					lua_pushinteger(L,lastI);
					lua_setfield(L,-2,"lastStr");
					lua_settop(L,Top);
					conCursorPosition=0; // курсор в начало
					lenStrOld=0;
					consoleProcFn();// ¬ызываем обработку
					return 1;
				}*/
				consoleEditProc(Window,Msg,keyCode,keyState);
				console_cmd=(const wchar_t*)P;
				if (lenStrOld<console_cmd.size()) // код дл€ кнопачек
				{
					lenStrOld++;
					console_cmd.insert(conCursorPosition,1,console_cmd.at(lenStrOld-1));
					console_cmd.erase(lenStrOld);
					conCursorPosition++;
					noxCallWndProc(Window,0x401E,(int)console_cmd.c_str(),-1);
				}
				return 1;
			}
			else
			{
				conPrintI((std::string("ERROR: Unknown keyboard event ") + std::to_string(keyState)).c_str());
			}
		}
		return consoleEditProc(Window,Msg,keyCode,keyState);
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
		if ((lua_type(L,1)!=LUA_TSTRING) )
		{
			lua_pushstring(L,"[conExec,type_arg1] -> argument must be a string.");
			lua_error_(L);
		}
		wchar_t Buf[200];
		if(200==mbstowcs(Buf,lua_tostring(L,1),200))
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
	InjectJumpTo(0x004884C5 ,&consoleEditDraw); // ћен€ем размеры
	InjectJumpTo(0x450B90 ,&onPrintConsoleTrap);

	lua_newtable(L); // делаем таблицу дл€ строк
	lua_pushinteger(L,1);
	lua_setfield(L,-2,"lastItem");
	lua_pushinteger(L,1);
	lua_setfield(L,-2,"lastStr");
	lua_setglobal(L,"conStr");

	registerserver("conExec",&conExecL);
}