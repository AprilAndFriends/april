/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_KEYS
#define APRIL_KEYS

namespace april
{
	// this enum defines Windows-specific virtual keys
	// however, these are mostly nicely defined, so let's use them
	// on other platforms too

#ifndef HAVE_MARMELADE
	enum KeySym {
		AK_UNKNOWN = -1,
		AK_NONE = 0,
	
		// first several VKs are unneeded
		// they are mouse related, or ctrl+break
		AK_LBUTTON = 1,
		AK_RBUTTON = 2,
		AK_CANCEL = 3,
		AK_MBUTTON = 6,
		AK_WHEELUP = 4,
		AK_WHEELDN = 5,
		AK_DOUBLETAP = 7, // touchscreen only

		// mostly common keys
		AK_BACK = 8,
		AK_TAB = 9,
		AK_CLEAR = 12,
		AK_RETURN = 13,
		AK_SHIFT = 16,
		AK_CONTROL = 17,
		AK_MENU = 18, // alt key 
		AK_PAUSE = 19, // multimedia pause key 
		AK_CAPITAL = 20, // caps lock

		// various keys needed for asian keyboards
		AK_KANA = 0x15, 
		AK_HANGEUL = 0x15,
		AK_HANGUL = 0x15,
		AK_JUNJA = 0x17,
		AK_FINAL = 0x18,
		AK_HANJA = 0x19,
		AK_KANJI = 0x19,
		AK_ESCAPE = 0x1B,
		AK_CONVERT = 0x1C,
		AK_NONCONVERT = 0x1D,
		AK_ACCEPT = 0x1E,
		AK_MODECHANGE = 0x1F,

		// space
		AK_SPACE = 0x20,

		// right side of the "control block"
		// commonly above cursor keys
		AK_PRIOR = 0x21, // page up
		AK_NEXT = 0x22, // page down
		AK_END = 0x23, 
		AK_HOME = 0x24,

		// cursor keys
		AK_LEFT = 0x25,
		AK_UP = 0x26,
		AK_RIGHT = 0x27,
		AK_DOWN = 0x28,

		// some less common keys
		AK_SELECT = 0x29, // select key
		AK_PRINT = 0x2A, // print key
		AK_EXECUTE = 0x2B, // execute key
		AK_SNAPSHOT = 0x2C, // print screen key

		// left side of "control block" commonly
		// above cursor keys, plus the help key
		// help key is NOT f1
		AK_INSERT = 0x2D,
		AK_DELETE = 0x2E,
		AK_HELP = 0x2F,
	
		// 0x30-0x39 -- '0'-'9'
		AK_0 = '0',
		AK_1 = '1',
		AK_2 = '2',
		AK_3 = '3',
		AK_4 = '4',
		AK_5 = '5',
		AK_6 = '6',
		AK_7 = '7',
		AK_8 = '8',
		AK_9 = '9',

		// 0x3A-0x40 -- undefined

		// 0x41-0x5A -- 'A'-'Z'	
		AK_A = 'A',
		AK_B = 'B',
		AK_C = 'C',
		AK_D = 'D',
		AK_E = 'E',
		AK_F = 'F',
		AK_G = 'G',
		AK_H = 'H',
		AK_I = 'I',
		AK_J = 'J',
		AK_K = 'K',
		AK_L = 'L',
		AK_M = 'M',
		AK_N = 'N',
		AK_O = 'O',
		AK_P = 'P',
		AK_Q = 'Q',
		AK_R = 'R',
		AK_S = 'S',
		AK_T = 'T',
		AK_U = 'U',
		AK_V = 'V',
		AK_W = 'W',
		AK_X = 'X',
		AK_Y = 'Y',
		AK_Z = 'Z',

		// special but mostly common VKs
		AK_LWIN = 0x5B,
		AK_RWIN = 0x5C,
		AK_APPS = 0x5D,
		AK_SLEEP = 0x5F,

		// numpad
		AK_NUMPAD0 = 0x60,
		AK_NUMPAD1 = 0x61,
		AK_NUMPAD2 = 0x62,
		AK_NUMPAD3 = 0x63,
		AK_NUMPAD4 = 0x64,
		AK_NUMPAD5 = 0x65,
		AK_NUMPAD6 = 0x66,
		AK_NUMPAD7 = 0x67,
		AK_NUMPAD8 = 0x68,
		AK_NUMPAD9 = 0x69,
		AK_MULTIPLY = 0x6A,
		AK_ADD = 0x6B,
		AK_SEPARATOR = 0x6C,
		AK_SUBTRACT = 0x6D,
		AK_DECIMAL = 0x6E,
		AK_DIVIDE = 0x6F,
		
		// f-keys
		AK_F1 = 0x70,
		AK_F2 = 0x71,
		AK_F3 = 0x72,
		AK_F4 = 0x73,
		AK_F5 = 0x74,
		AK_F6 = 0x75,
		AK_F7 = 0x76,
		AK_F8 = 0x77,
		AK_F9 = 0x78,
		AK_F10 = 0x79,
		AK_F11 = 0x7A,
		AK_F12 = 0x7B,
		AK_F13 = 0x7C,
		AK_F14 = 0x7D,
		AK_F15 = 0x7E,
		AK_F16 = 0x7F,
		AK_F17 = 0x80,
		AK_F18 = 0x81,
		AK_F19 = 0x82,
		AK_F20 = 0x83,
		AK_F21 = 0x84,
		AK_F22 = 0x85,
		AK_F23 = 0x86,
		AK_F24 = 0x87,

		// some more lock keys
		AK_NUMLOCK = 0x90,
		AK_SCROLL = 0x91,

		// specific left-and-right-shift keys
		AK_LSHIFT = 0xA0,
		AK_RSHIFT = 0xA1,
		AK_LCONTROL = 0xA2,
		AK_RCONTROL = 0xA3,
		AK_LMENU = 0xA4,
		AK_RMENU = 0xA5,
		
		// browser control keys
		AK_BROWSER_BACK = 0xA6,
		AK_BROWSER_FORWARD = 0xA7,
		AK_BROWSER_REFRESH = 0xA8,
		AK_BROWSER_STOP = 0xA9,
		AK_BROWSER_SEARCH = 0xAA,
		AK_BROWSER_FAVORITES = 0xAB,
		AK_BROWSER_HOME = 0xAC,

		// volume keys
		AK_VOLUME_MUTE = 0xAD,
		AK_VOLUME_DOWN = 0xAE,
		AK_VOLUME_UP = 0xAF,

		// more multimedia keys
		AK_MEDIA_NEXT_TRACK = 0xB0,
		AK_MEDIA_PREV_TRACK = 0xB1,
		AK_MEDIA_STOP = 0xB2,
		AK_MEDIA_PLAY_PAUSE = 0xB3,

		// app launching keys
		AK_LAUNCH_MAIL = 0xB4,
		AK_LAUNCH_MEDIA_SELECT = 0xB5,
		AK_LAUNCH_APP1 = 0xB6,
		AK_LAUNCH_APP2 = 0xB7,

		// oem keys
		AK_OEM_2 = 0xBF,
		AK_OEM_3 = 0xC0,
		AK_OEM_4 = 0xDB,
		AK_OEM_5 = 0xDC,
		AK_OEM_6 = 0xDD,
		AK_OEM_7 = 0xDE,
		AK_OEM_8 = 0xDF,
		AK_OEM_102 = 0xE2,

		// uncommon keys
		AK_PACKET = 0xE7,
		AK_ATTN = 0xF6,
		AK_CRSEL = 0xF7,
		AK_EXSEL = 0xF8,
		AK_EREOF = 0xF9,

		// multimedia keys
		AK_PLAY = 0xFA,
		AK_ZOOM = 0xFB,

		// uncommon and oem keys
		AK_NONAME = 0xFC,
		AK_PA1 = 0xFD,
		AK_OEM_CLEAR = 0xFE
	};
#else
	enum KeySym {
		AK_UNKNOWN = -1,
		AK_NONE = 0,
	
		// first several VKs are unneeded
		// they are mouse related, or ctrl+break
		AK_CANCEL = 230,
		AK_CLEAR = 231,
		AK_SHIFT = 232,
		AK_CONTROL = 233,
		//AK_MENU = 234,


		AK_LBUTTON = 223,
		AK_RBUTTON = 224,
		AK_MBUTTON = 225,
		AK_WHEELUP = 226,
		AK_WHEELDN = 227,
		AK_DOUBLETAP = 228, // touchscreen only

		// mostly common keys
		AK_BACKSPACE = 3,
		AK_TAB = 2,
		AK_RETURN = 4,
		AK_ESCAPE = 1,
		AK_CAPITAL = 114, // caps lock

		// various keys needed for asian keyboards
		AK_KANA = 235, 
		AK_HANGEUL = 236,
		AK_HANGUL = 237,
		AK_JUNJA = 238,
		AK_FINAL = 239,
		AK_HANJA = 240,
		AK_KANJI = 241,
		AK_CONVERT = 242,
		AK_NONCONVERT = 243,
		AK_ACCEPT = 244,
		AK_MODECHANGE = 245,

		// space
		AK_SPACE = 8,

		// right side of the "control block"
		// commonly above cursor keys
		AK_PRIOR = 120, // page up
		AK_NEXT = 121, // page down
		AK_END = 122, 
		AK_HOME = 119,
		AK_INSERT = 118,
		AK_DELETE = 123,
		AK_PAUSE = 124,

		// cursor keys
		AK_LEFT = 9,
		AK_UP = 10,
		AK_RIGHT = 11,
		AK_DOWN = 12,

		// some less common keys
		//AK_SELECT = 0x29, // select key
		//AK_PRINT = 0x2A, // print key
		//AK_EXECUTE = 0x2B, // execute key
		//AK_SNAPSHOT = 0x2C, // print screen key

		// left side of "control block" commonly
		// above cursor keys, plus the help key
		// help key is NOT f1
		//AK_HELP = 0x2F,

		// special but mostly common VKs
		//AK_LWIN = 0x5B,
		//AK_RWIN = 0x5C,
		//AK_APPS = 0x5D,
		//AK_SLEEP = 0x5F,

		// browser control keys
		//AK_BROWSER_BACK = 0xA6,
		//AK_BROWSER_FORWARD = 0xA7,
		//AK_BROWSER_REFRESH = 0xA8,
		//AK_BROWSER_STOP = 0xA9,
		//AK_BROWSER_SEARCH = 0xAA,
		//AK_BROWSER_FAVORITES = 0xAB,
		//AK_BROWSER_HOME = 0xAC,

		// more multimedia keys
		//AK_MEDIA_NEXT_TRACK = 0xB0,
		//AK_MEDIA_PREV_TRACK = 0xB1,
		//AK_MEDIA_STOP = 0xB2,
		//AK_MEDIA_PLAY_PAUSE = 0xB3,

		// app launching keys
		//AK_LAUNCH_MAIL = 0xB4,
		//AK_LAUNCH_MEDIA_SELECT = 0xB5,
		//AK_LAUNCH_APP1 = 0xB6,
		//AK_LAUNCH_APP2 = 0xB7,

		// uncommon keys
		//AK_PACKET = 0xE7,
		//AK_ATTN = 0xF6,
		//AK_CRSEL = 0xF7,
		//AK_EXSEL = 0xF8,
		//AK_EREOF = 0xF9,

		// multimedia keys
		//AK_PLAY = 0xFA,
		//AK_ZOOM = 0xFB,
	
		// character keys

		AK_0 = 13,
		AK_1 = 14,
		AK_2 = 15,
		AK_3 = 16,
		AK_4 = 17,
		AK_5 = 18,
		AK_6 = 19,
		AK_7 = 20,
		AK_8 = 21,
		AK_9 = 22,

		AK_A = 23,
		AK_B = 24,
		AK_C = 25,
		AK_D = 26,
		AK_E = 27,
		AK_F = 28,
		AK_G = 29,
		AK_H = 30,
		AK_I = 31,
		AK_J = 32,
		AK_K = 33,
		AK_L = 34,
		AK_M = 35,
		AK_N = 36,
		AK_O = 37,
		AK_P = 38,
		AK_Q = 39,
		AK_R = 40,
		AK_S = 41,
		AK_T = 42,
		AK_U = 43,
		AK_V = 44,
		AK_W = 45,
		AK_X = 46,
		AK_Y = 47,
		AK_Z = 48,

		// numpad
		AK_NUMPAD0 = 59,
		AK_NUMPAD1 = 60,
		AK_NUMPAD2 = 61,
		AK_NUMPAD3 = 62,
		AK_NUMPAD4 = 63,
		AK_NUMPAD5 = 64,
		AK_NUMPAD6 = 65,
		AK_NUMPAD7 = 66,
		AK_NUMPAD8 = 67,
		AK_NUMPAD9 = 68,
		AK_ADD = 69,
		AK_SUBTRACT = 70,
		AK_DIVIDE = 116,
		AK_DECIMAL = 115,
		AK_MULTIPLY = 77,
		AK_NUMENTER = 71,

		//AK_SEPARATOR = 0x6C,
		
		// f-keys
		AK_F1 = 49,
		AK_F2 = 50,
		AK_F3 = 51,
		AK_F4 = 52,
		AK_F5 = 53,
		AK_F6 = 54,
		AK_F7 = 55,
		AK_F8 = 56,
		AK_F9 = 57,
		AK_F10 = 58,
		AK_F11 = 97,
		AK_F12 = 98,
		//AK_F13 = 0x7C,
		//AK_F14 = 0x7D,
		//AK_F15 = 0x7E,
		//AK_F16 = 0x7F,
		//AK_F17 = 0x80,
		//AK_F18 = 0x81,
		//AK_F19 = 0x82,
		//AK_F20 = 0x83,
		//AK_F21 = 0x84,
		//AK_F22 = 0x85,
		//AK_F23 = 0x86,
		//AK_F24 = 0x87,

		// some more lock keys
		AK_NUMLOCK = 117,
		//AK_SCROLL = 0x91,

		// specific left-and-right-shift keys
		AK_LSHIFT = 5,
		AK_RSHIFT = 102,
		AK_LCONTROL = 6,
		AK_RCONTROL = 100,
		AK_LMENU = 99, // left alt
		AK_RMENU = 101, // right alt

		// android specific keys
		// android common keys
		AK_MENU = 127,
		AK_BACK = 126,
		AK_OK = 78,
		AK_SEARCH = 128,
		AK_LS = 74,
		AK_RS = 75,

		// android soft keys
		AK_RSK = 72,
		AK_LSK = 73,

		// android media
		
		//AK_VOLUME_MUTE = 0xAD,
		AK_CAMERA = 82,
		AK_MIC = 83,
		AK_VOLUME_DOWN = 81,
		AK_VOLUME_UP = 80,

		// android phone keys
		AK_ACCEPT_CALL = 86,
		AK_DECLINE_CALL = 87,

		// abstract keys
		AK_ABS_GAME_A = 200,
		AK_ABS_GAME_B = 201,
		AK_ABS_GAME_C = 202,
		AK_ABS_GAME_D = 203,

		AK_ABS_UP = 204,
		AK_ABS_DOWN = 205,
		AK_ABS_LEFT = 206,
		AK_ABS_RIGHT = 207,
		AK_ABS_OK = 208,
		AK_ABS_ACT = 209,
		AK_ABS_BACK = 210,

		// other key codes related to marmelade
		AK_RESERVED = 7,
		AK_HASH = 76,
		AK_CLR = 79,
		AK_FN = 87,
		AK_SYM = 85,
		AK_BACKTICK = 103,
		AK_COMMA = 104,
		AK_PERIOD = 105,
		AK_SLASH = 106,
		AK_BACKSLASH = 107,
		AK_SEMICOLON = 108,
		AK_APOSTROPHE = 109,
		AK_LEFT_BRACKET = 110,
		AK_RIGHT_BRACKET = 111,
		AK_EQUALS = 112,
		AK_MINUS = 113,
		AK_AT = 125,

		// oem keys
		AK_OEM_1 = 89,
		AK_OEM_2 = 90,
		AK_OEM_3 = 91,
		AK_OEM_4 = 92,
		AK_OEM_5 = 93,
		AK_OEM_6 = 94,
		AK_OEM_7 = 95,
		AK_OEM_8 = 96,
		//AK_NONAME = 0xFC,
		//AK_PA1 = 0xFD,
		//AK_OEM_CLEAR = 0xFE*/
		//AK_OEM_102 = 0xE2,
	};
#endif
}

#endif
