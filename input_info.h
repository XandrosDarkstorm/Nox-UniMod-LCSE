#pragma once

/// <summary>
/// Nox keyboard layout. It does not look like anything familiar.
/// Unfinished. There is a gap.
/// </summary>
enum noxKeyboardKeys : unsigned char
{
	KBD_ESC = 1,
	KBD_1 = 2,
	KBD_2 = 3,
	KBD_3 = 4,
	KBD_4 = 5,
	KBD_5 = 6,
	KBD_6 = 7,
	KBD_7 = 8,
	KBD_8 = 9,
	KBD_9 = 10,
	KBD_0 = 11,
	KBD_MINUS = 12,
	KBD_EQUALS = 13,
	KBD_BACKSPACE = 14,
	KBD_TAB = 15,
	KBD_Q = 16,
	KBD_W = 17,
	KBD_E = 18,
	KBD_R = 19,
	KBD_T = 20,
	KBD_Y = 21,
	KBD_U = 22,
	KBD_I = 23,
	KBD_O = 24,
	KBD_P = 25,
	KBD_BRACKET_OPEN = 26, // [
	KBD_BRACKET_CLOSE = 27, // ]
	KBD_ENTER = 28,
	KBD_CTRL_LEFT = 29,
	KBD_A = 30,
	KBD_S = 31,
	KBD_D = 32,
	KBD_F = 33,
	KBD_G = 34,
	KBD_H = 35,
	KBD_J = 36,
	KBD_K = 37,
	KBD_L = 38,
	KBD_SEMICOLON = 39, // ;
	KBD_APOSTROPHE = 40, // '
	KBD_BACKQUOTE = 41, // `
	KBD_SHIFT_LEFT = 42,
	KBD_BACKSLASH = 43, // \ 
	KBD_Z = 44,
	KBD_X = 45,
	KBD_C = 46,
	KBD_V = 47,
	KBD_B = 48,
	KBD_N = 49,
	KBD_M = 50,
	KBD_COMMA = 51,
	KBD_DOT = 52,
	KBD_SLASH = 53,
	KBD_SHIFT_RIGHT = 54,
	KBD_KP_MULTIPLY = 55,
	KBD_ALT_LEFT = 56,
	KBD_SPACE = 57,
	KBD_CAPS_LOCK = 58,
	KBD_F1 = 59,
	KBD_F2 = 60,
	KBD_F3 = 61,
	KBD_F4 = 62,
	KBD_F5 = 63,
	KBD_F6 = 64,
	KBD_F7 = 65,
	KBD_F8 = 66,
	KBD_F9 = 67,
	KBD_F10 = 68,
	KBD_NUM_LOCK = 69,
	KBD_SCROLL_LOCK = 70,
	KBD_KP_7 = 71,
	KBD_KP_8 = 72,
	KBD_KP_9 = 73,
	KBD_KP_MINUS = 74,
	KBD_KP_4 = 75,
	KBD_KP_5 = 76,
	KBD_KP_6 = 77,
	KBD_KP_PLUS = 78,
	KBD_KP_1 = 79,
	KBD_KP_2 = 80,
	KBD_KP_3 = 81,
	KBD_KP_0 = 82,
	KBD_KP_DEL = 83,
	KBD_F11 = 87,
	KBD_F12 = 88,

	KBD_FN = 128, //Various additional function buttons are 128
	KBD_KP_ENTER = 156,
	KBD_CTRL_RIGHT = 157,
	KBD_KP_DIVIDE = 181,
	KBD_PRINT_SCREEN = 183,
	KBD_ALT_RIGHT = 184,
	KBD_HOME = 199,
	KBD_UP = 200,
	KBD_PAGEUP = 201,
	KBD_LEFT = 203,
	KBD_RIGHT = 205,
	KBD_END = 207,
	KBD_DOWN = 208,
	KBD_PAGEDOWN = 209,
	KBD_INSERT = 210,
	KBD_DELETE = 211,
	KBD_META = 219, // Windows key
	KBD_MENU = 221 //Usually the key between right alt and right ctrl.
};

/// <summary>
/// Keyboard key state.
/// </summary>
enum noxKeyboardEvents : unsigned char
{
	KEY_RELEASE = 1,
	KEY_PRESS = 2
};

/// <summary>
/// Window events nox uses
/// </summary>
enum noxWindowEvents : unsigned char
{
	MOUSE_LEFT_BUTTON_DOWN = 5,
	MOUSE_LEFT_BUTTON_UP_AFTERSEC = 6,
	MOUSE_LEFT_BUTTON_UP = 7,
	MOUSE_LEFT_BUTTON_HOLDDOWN = 8,

	MOUSE_RIGHT_BUTTON_DOWN = 9,
	MOUSE_RIGHT_BUTTON_UP_AFTERSEC = 10,
	MOUSE_RIGHT_BUTTON_UP = 11,
	MOUSE_RIGHT_BUTTON_HOLDDOWN = 12,

	MOUSE_MIDDLE_BUTTON_DOWN = 13,
	MOUSE_MIDDLE_BUTTON_UP_AFTERSEC = 14,
	MOUSE_MIDDLE_BUTTON_UP = 15,
	MOUSE_MIDDLE_BUTTON_HOLDDOWN = 16,
	
	MOUSE_ENTER = 17,
	MOUSE_LEAVE = 18,

	MOUSE_WHEEL_SCROLL_UP = 19,
	MOUSE_WHEEL_SCROLL_DOWN = 20,

	KEYBOARD_INPUT = 21
};

//Unimod LCSE specific

enum keyboardModifiers : unsigned char
{
	KBDMOD_SHIFT = 1,
	KBDMOD_CTRL = 2,
	KBDMOD_ALT = 4
};