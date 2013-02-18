/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a key table.

#ifndef APRIL_KEYS
#define APRIL_KEYS

namespace april
{
	// this enum defines Windows-specific virtual keys
	// however, these are mostly nicely defined, so let's use them
	// on other platforms too

	enum Key
	{
#ifndef _ANDROID
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

		// most common keys
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

		// Mac CMD keys
		AK_LCOMMAND = 1117,
		AK_RCOMMAND = 1118,
		
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
	
		// '0'-'9'
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

		// 'A'-'Z'	
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
		
		// F-keys
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
#else
		// codes obtained from http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_0
		AK_UNKNOWN = -1,
		AK_NONE = 0,
	
		// LBUTTON and RBUTTON are necessary
		AK_LBUTTON = 1,
		AK_RBUTTON = 2,
		// these mostly don't exist on Android
		AK_CANCEL = 0,
		AK_MBUTTON = 0,
		AK_WHEELUP = 168, // ZOOM_IN
		AK_WHEELDN = 169, // ZOOM_OUT
		AK_DOUBLETAP = 0,

		// most common keys, some not supported
		AK_BACK = 67,
		AK_TAB = 61,
		AK_CLEAR = 28,
		AK_RETURN = 66,
		AK_SHIFT = 0,
		AK_CONTROL = 0,
		AK_MENU = 82,
		AK_PAUSE = 0,
		AK_CAPITAL = 115,

		// various keys for asian keyboards not supported
		AK_KANA = 0,
		AK_HANGEUL = 0,
		AK_HANGUL = 0,
		AK_JUNJA = 0,
		AK_FINAL = 0,
		AK_HANJA = 0,
		AK_KANJI = 0,
		AK_ESCAPE = 4, // using Android's back button for this
		AK_CONVERT = 0,
		AK_NONCONVERT = 0,
		AK_ACCEPT = 0,
		AK_MODECHANGE = 0,

		// space
		AK_SPACE = 62,

		// don't exist on Android
		AK_PRIOR = 0,
		AK_NEXT = 0,
		AK_END = 0,
		AK_HOME = 0,

		// don't exist on Android
		AK_LEFT = 0,
		AK_UP = 0,
		AK_RIGHT = 0,
		AK_DOWN = 0,

		// don't exist on Android
		AK_SELECT = 0,
		AK_PRINT = 0,
		AK_EXECUTE = 0,
		AK_SNAPSHOT = 0,

		// some more keys
		AK_INSERT = 124,
		AK_DELETE = 112,
		AK_HELP = 0,
	
		// '0'-'9'
		AK_0 = 7,
		AK_1 = 8,
		AK_2 = 9,
		AK_3 = 10,
		AK_4 = 11,
		AK_5 = 12,
		AK_6 = 13,
		AK_7 = 14,
		AK_8 = 15,
		AK_9 = 16,

		// 'A'-'Z'	
		AK_A = 29,
		AK_B = 30,
		AK_C = 31,
		AK_D = 32,
		AK_E = 33,
		AK_F = 34,
		AK_G = 35,
		AK_H = 36,
		AK_I = 37,
		AK_J = 38,
		AK_K = 39,
		AK_L = 40,
		AK_M = 41,
		AK_N = 42,
		AK_O = 43,
		AK_P = 44,
		AK_Q = 45,
		AK_R = 46,
		AK_S = 47,
		AK_T = 48,
		AK_U = 49,
		AK_V = 50,
		AK_W = 51,
		AK_X = 52,
		AK_Y = 53,
		AK_Z = 54,

		// don't exist on Android
		AK_LWIN = 0,
		AK_RWIN = 0,
		AK_APPS = 0,
		AK_SLEEP = 0,

		// numpad
		AK_NUMPAD0 = 144,
		AK_NUMPAD1 = 145,
		AK_NUMPAD2 = 146,
		AK_NUMPAD3 = 147,
		AK_NUMPAD4 = 148,
		AK_NUMPAD5 = 149,
		AK_NUMPAD6 = 150,
		AK_NUMPAD7 = 151,
		AK_NUMPAD8 = 152,
		AK_NUMPAD9 = 153,
		AK_MULTIPLY = 155,
		AK_ADD = 157,
		AK_SEPARATOR = 159,
		AK_SUBTRACT = 156,
		AK_DECIMAL = 158,
		AK_DIVIDE = 154,
		
		// F-keys
		AK_F1 = 131,
		AK_F2 = 132,
		AK_F3 = 133,
		AK_F4 = 134,
		AK_F5 = 135,
		AK_F6 = 136,
		AK_F7 = 137,
		AK_F8 = 138,
		AK_F9 = 139,
		AK_F10 = 140,
		AK_F11 = 141,
		AK_F12 = 142,
		// don't exist on Android
		AK_F13 = 0,
		AK_F14 = 0,
		AK_F15 = 0,
		AK_F16 = 0,
		AK_F17 = 0,
		AK_F18 = 0,
		AK_F19 = 0,
		AK_F20 = 0,
		AK_F21 = 0,
		AK_F22 = 0,
		AK_F23 = 0,
		AK_F24 = 0,

		// don't exist on Android
		AK_NUMLOCK = 143,
		AK_SCROLL = 116,

		// specific left-and-right-shift keys
		AK_LSHIFT = 59,
		AK_RSHIFT = 60,
		AK_LCONTROL = 113,
		AK_RCONTROL = 114,
		AK_LMENU = 57,
		AK_RMENU = 58,
		
		// browser control keys
		AK_BROWSER_BACK = 4,
		AK_BROWSER_FORWARD = 125,
		AK_BROWSER_REFRESH = 0,
		AK_BROWSER_STOP = 0,
		AK_BROWSER_SEARCH = 84,
		AK_BROWSER_FAVORITES = 0,
		AK_BROWSER_HOME = 0,

		// volume keys
		AK_VOLUME_MUTE = 164,
		AK_VOLUME_DOWN = 25,
		AK_VOLUME_UP = 24,

		// don't exist on Android
		AK_MEDIA_NEXT_TRACK = 0,
		AK_MEDIA_PREV_TRACK = 0,
		AK_MEDIA_STOP = 0,
		AK_MEDIA_PLAY_PAUSE = 0,

		// don't exist on Android
		AK_LAUNCH_MAIL = 0,
		AK_LAUNCH_MEDIA_SELECT = 0,
		AK_LAUNCH_APP1 = 0,
		AK_LAUNCH_APP2 = 0,

		// don't exist on Android
		AK_OEM_2 = 0,
		AK_OEM_3 = 0,
		AK_OEM_4 = 0,
		AK_OEM_5 = 0,
		AK_OEM_6 = 0,
		AK_OEM_7 = 0,
		AK_OEM_8 = 0,
		AK_OEM_102 = 0,

		// don't exist on Android
		AK_PACKET = 0,
		AK_ATTN = 0,
		AK_CRSEL = 0,
		AK_EXSEL = 0,
		AK_EREOF = 0,

		// don't exist on Android
		AK_PLAY = 0,
		AK_ZOOM = 0,

		// don't exist on Android
		AK_NONAME = 0,
		AK_PA1 = 0,
		AK_OEM_CLEAR = 0
#endif
	};

}

#endif
