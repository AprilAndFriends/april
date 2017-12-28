/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "Keys.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE_LOOSE(Key, 0,
	(
		HL_ENUM_DEFINE_VALUE(Key, None, 0);
		HL_ENUM_DEFINE_VALUE(Key, MouseL, 1);
		HL_ENUM_DEFINE_VALUE(Key, MouseR, 2);
		HL_ENUM_DEFINE_VALUE(Key, Cancel, 3);
		HL_ENUM_DEFINE_VALUE(Key, ScrollUp, 4);
		HL_ENUM_DEFINE_VALUE(Key, ScrollDown, 5);
		HL_ENUM_DEFINE_VALUE(Key, MouseM, 6);
		HL_ENUM_DEFINE_VALUE(Key, DoubleTap, 7);
		HL_ENUM_DEFINE_VALUE(Key, Backspace, 8);
		HL_ENUM_DEFINE_VALUE(Key, Tab, 9);
		HL_ENUM_DEFINE_VALUE(Key, Clear, 12);
		HL_ENUM_DEFINE_VALUE(Key, Return, 13);
		HL_ENUM_DEFINE_VALUE(Key, Shift, 16);
		HL_ENUM_DEFINE_VALUE(Key, Control, 17);
		HL_ENUM_DEFINE_VALUE(Key, Menu, 18);
		HL_ENUM_DEFINE_VALUE(Key, Pause, 19);
		HL_ENUM_DEFINE_VALUE(Key, CapsLock, 20);
		HL_ENUM_DEFINE_VALUE(Key, KanaHangul, 0x15);
		HL_ENUM_DEFINE_VALUE(Key, Junja, 0x17);
		HL_ENUM_DEFINE_VALUE(Key, Final, 0x18);
		HL_ENUM_DEFINE_VALUE(Key, HanjaKanji, 0x19);
		HL_ENUM_DEFINE_VALUE(Key, Escape, 0x1B);
		HL_ENUM_DEFINE_VALUE(Key, Convert, 0x1C);
		HL_ENUM_DEFINE_VALUE(Key, NonConvert, 0x1D);
		HL_ENUM_DEFINE_VALUE(Key, Accept, 0x1E);
		HL_ENUM_DEFINE_VALUE(Key, ModeChange, 0x1F);
		HL_ENUM_DEFINE_VALUE(Key, CommandL, 1117);
		HL_ENUM_DEFINE_VALUE(Key, CommandR, 1118);
		HL_ENUM_DEFINE_VALUE(Key, Space, 0x20);
		HL_ENUM_DEFINE_VALUE(Key, Prior, 0x21);
		HL_ENUM_DEFINE_VALUE(Key, Next, 0x22);
		HL_ENUM_DEFINE_VALUE(Key, End, 0x23);
		HL_ENUM_DEFINE_VALUE(Key, Home, 0x24);
		HL_ENUM_DEFINE_VALUE(Key, ArrowLeft, 0x25);
		HL_ENUM_DEFINE_VALUE(Key, ArrowUp, 0x26);
		HL_ENUM_DEFINE_VALUE(Key, ArrowRight, 0x27);
		HL_ENUM_DEFINE_VALUE(Key, ArrowDown, 0x28);
		HL_ENUM_DEFINE_VALUE(Key, Select, 0x29);
		HL_ENUM_DEFINE_VALUE(Key, Print, 0x2A);
		HL_ENUM_DEFINE_VALUE(Key, Execute, 0x2B);
		HL_ENUM_DEFINE_VALUE(Key, PrintScreen, 0x2C);
		HL_ENUM_DEFINE_VALUE(Key, Insert, 0x2D);
		HL_ENUM_DEFINE_VALUE(Key, Delete, 0x2E);
		HL_ENUM_DEFINE_VALUE(Key, Help, 0x2F);
		HL_ENUM_DEFINE_VALUE(Key, Num0, '0');
		HL_ENUM_DEFINE_VALUE(Key, Num1, '1');
		HL_ENUM_DEFINE_VALUE(Key, Num2, '2');
		HL_ENUM_DEFINE_VALUE(Key, Num3, '3');
		HL_ENUM_DEFINE_VALUE(Key, Num4, '4');
		HL_ENUM_DEFINE_VALUE(Key, Num5, '5');
		HL_ENUM_DEFINE_VALUE(Key, Num6, '6');
		HL_ENUM_DEFINE_VALUE(Key, Num7, '7');
		HL_ENUM_DEFINE_VALUE(Key, Num8, '8');
		HL_ENUM_DEFINE_VALUE(Key, Num9, '9');
		HL_ENUM_DEFINE_VALUE(Key, A, 'A');
		HL_ENUM_DEFINE_VALUE(Key, B, 'B');
		HL_ENUM_DEFINE_VALUE(Key, C, 'C');
		HL_ENUM_DEFINE_VALUE(Key, D, 'D');
		HL_ENUM_DEFINE_VALUE(Key, E, 'E');
		HL_ENUM_DEFINE_VALUE(Key, F, 'F');
		HL_ENUM_DEFINE_VALUE(Key, G, 'G');
		HL_ENUM_DEFINE_VALUE(Key, H, 'H');
		HL_ENUM_DEFINE_VALUE(Key, I, 'I');
		HL_ENUM_DEFINE_VALUE(Key, J, 'J');
		HL_ENUM_DEFINE_VALUE(Key, K, 'K');
		HL_ENUM_DEFINE_VALUE(Key, L, 'L');
		HL_ENUM_DEFINE_VALUE(Key, M, 'M');
		HL_ENUM_DEFINE_VALUE(Key, N, 'N');
		HL_ENUM_DEFINE_VALUE(Key, O, 'O');
		HL_ENUM_DEFINE_VALUE(Key, P, 'P');
		HL_ENUM_DEFINE_VALUE(Key, Q, 'Q');
		HL_ENUM_DEFINE_VALUE(Key, R, 'R');
		HL_ENUM_DEFINE_VALUE(Key, S, 'S');
		HL_ENUM_DEFINE_VALUE(Key, T, 'T');
		HL_ENUM_DEFINE_VALUE(Key, U, 'U');
		HL_ENUM_DEFINE_VALUE(Key, V, 'V');
		HL_ENUM_DEFINE_VALUE(Key, W, 'W');
		HL_ENUM_DEFINE_VALUE(Key, X, 'X');
		HL_ENUM_DEFINE_VALUE(Key, Y, 'Y');
		HL_ENUM_DEFINE_VALUE(Key, Z, 'Z');
		HL_ENUM_DEFINE_VALUE(Key, WindowsL, 0x5B);
		HL_ENUM_DEFINE_VALUE(Key, WindowsR, 0x5C);
		HL_ENUM_DEFINE_VALUE(Key, Apps, 0x5D);
		HL_ENUM_DEFINE_VALUE(Key, Sleep, 0x5F);
		HL_ENUM_DEFINE_VALUE(Key, NumPad0, 0x60);
		HL_ENUM_DEFINE_VALUE(Key, NumPad1, 0x61);
		HL_ENUM_DEFINE_VALUE(Key, NumPad2, 0x62);
		HL_ENUM_DEFINE_VALUE(Key, NumPad3, 0x63);
		HL_ENUM_DEFINE_VALUE(Key, NumPad4, 0x64);
		HL_ENUM_DEFINE_VALUE(Key, NumPad5, 0x65);
		HL_ENUM_DEFINE_VALUE(Key, NumPad6, 0x66);
		HL_ENUM_DEFINE_VALUE(Key, NumPad7, 0x67);
		HL_ENUM_DEFINE_VALUE(Key, NumPad8, 0x68);
		HL_ENUM_DEFINE_VALUE(Key, NumPad9, 0x69);
		HL_ENUM_DEFINE_VALUE(Key, Multiply, 0x6A);
		HL_ENUM_DEFINE_VALUE(Key, Add, 0x6B);
		HL_ENUM_DEFINE_VALUE(Key, Separator, 0x6C);
		HL_ENUM_DEFINE_VALUE(Key, Subtract, 0x6D);
		HL_ENUM_DEFINE_VALUE(Key, Decimal, 0x6E);
		HL_ENUM_DEFINE_VALUE(Key, Divide, 0x6F);
		HL_ENUM_DEFINE_VALUE(Key, F1, 0x70);
		HL_ENUM_DEFINE_VALUE(Key, F2, 0x71);
		HL_ENUM_DEFINE_VALUE(Key, F3, 0x72);
		HL_ENUM_DEFINE_VALUE(Key, F4, 0x73);
		HL_ENUM_DEFINE_VALUE(Key, F5, 0x74);
		HL_ENUM_DEFINE_VALUE(Key, F6, 0x75);
		HL_ENUM_DEFINE_VALUE(Key, F7, 0x76);
		HL_ENUM_DEFINE_VALUE(Key, F8, 0x77);
		HL_ENUM_DEFINE_VALUE(Key, F9, 0x78);
		HL_ENUM_DEFINE_VALUE(Key, F10, 0x79);
		HL_ENUM_DEFINE_VALUE(Key, F11, 0x7A);
		HL_ENUM_DEFINE_VALUE(Key, F12, 0x7B);
		HL_ENUM_DEFINE_VALUE(Key, F13, 0x7C);
		HL_ENUM_DEFINE_VALUE(Key, F14, 0x7D);
		HL_ENUM_DEFINE_VALUE(Key, F15, 0x7E);
		HL_ENUM_DEFINE_VALUE(Key, F16, 0x7F);
		HL_ENUM_DEFINE_VALUE(Key, F17, 0x80);
		HL_ENUM_DEFINE_VALUE(Key, F18, 0x81);
		HL_ENUM_DEFINE_VALUE(Key, F19, 0x82);
		HL_ENUM_DEFINE_VALUE(Key, F20, 0x83);
		HL_ENUM_DEFINE_VALUE(Key, F21, 0x84);
		HL_ENUM_DEFINE_VALUE(Key, F22, 0x85);
		HL_ENUM_DEFINE_VALUE(Key, F23, 0x86);
		HL_ENUM_DEFINE_VALUE(Key, F24, 0x87);
		HL_ENUM_DEFINE_VALUE(Key, NumLock, 0x90);
		HL_ENUM_DEFINE_VALUE(Key, ScrollLock, 0x91);
		HL_ENUM_DEFINE_VALUE(Key, ShiftL, 0xA0);
		HL_ENUM_DEFINE_VALUE(Key, ShiftR, 0xA1);
		HL_ENUM_DEFINE_VALUE(Key, ControlL, 0xA2);
		HL_ENUM_DEFINE_VALUE(Key, ControlR, 0xA3);
		HL_ENUM_DEFINE_VALUE(Key, MenuL, 0xA4);
		HL_ENUM_DEFINE_VALUE(Key, MenuR, 0xA5);
		HL_ENUM_DEFINE_VALUE(Key, BrowserBack, 0xA6);
		HL_ENUM_DEFINE_VALUE(Key, BrowserForward, 0xA7);
		HL_ENUM_DEFINE_VALUE(Key, BrowserRefresh, 0xA8);
		HL_ENUM_DEFINE_VALUE(Key, BrowserStop, 0xA9);
		HL_ENUM_DEFINE_VALUE(Key, BrowserSearch, 0xAA);
		HL_ENUM_DEFINE_VALUE(Key, BrowserFavorites, 0xAB);
		HL_ENUM_DEFINE_VALUE(Key, BrowserHome, 0xAC);
		HL_ENUM_DEFINE_VALUE(Key, VolumeMute, 0xAD);
		HL_ENUM_DEFINE_VALUE(Key, VolumeDown, 0xAE);
		HL_ENUM_DEFINE_VALUE(Key, VolumeUp, 0xAF);
		HL_ENUM_DEFINE_VALUE(Key, MediaNextTrack, 0xB0);
		HL_ENUM_DEFINE_VALUE(Key, MediaPreviousTrack, 0xB1);
		HL_ENUM_DEFINE_VALUE(Key, MediaStop, 0xB2);
		HL_ENUM_DEFINE_VALUE(Key, MediaPlayPause, 0xB3);
		HL_ENUM_DEFINE_VALUE(Key, LaunchMail, 0xB4);
		HL_ENUM_DEFINE_VALUE(Key, LaunchMediaSelect, 0xB5);
		HL_ENUM_DEFINE_VALUE(Key, LaunchApp1, 0xB6);
		HL_ENUM_DEFINE_VALUE(Key, LaunchApp2, 0xB7);
		HL_ENUM_DEFINE_VALUE(Key, Oem2, 0xBF);
		HL_ENUM_DEFINE_VALUE(Key, Oem3, 0xC0);
		HL_ENUM_DEFINE_VALUE(Key, Oem4, 0xDB);
		HL_ENUM_DEFINE_VALUE(Key, Oem5, 0xDC);
		HL_ENUM_DEFINE_VALUE(Key, Oem6, 0xDD);
		HL_ENUM_DEFINE_VALUE(Key, Oem7, 0xDE);
		HL_ENUM_DEFINE_VALUE(Key, Oem8, 0xDF);
		HL_ENUM_DEFINE_VALUE(Key, Oem102, 0xE2);
		HL_ENUM_DEFINE_VALUE(Key, Packet, 0xE7);
		HL_ENUM_DEFINE_VALUE(Key, Attn, 0xF6);
		HL_ENUM_DEFINE_VALUE(Key, Crsel, 0xF7);
		HL_ENUM_DEFINE_VALUE(Key, Exsel, 0xF8);
		HL_ENUM_DEFINE_VALUE(Key, Ereof, 0xF9);
		HL_ENUM_DEFINE_VALUE(Key, Play, 0xFA);
		HL_ENUM_DEFINE_VALUE(Key, Zoom, 0xFB);
		HL_ENUM_DEFINE_VALUE(Key, NoName, 0xFC);
		HL_ENUM_DEFINE_VALUE(Key, Pa1, 0xFD);
		HL_ENUM_DEFINE_VALUE(Key, OemClear, 0xFE);
	));
	
	HL_ENUM_CLASS_DEFINE_LOOSE(Button, 0,
	(
		HL_ENUM_DEFINE_VALUE(Button, None, 0);
		HL_ENUM_DEFINE_VALUE(Button, Start, 1);
		HL_ENUM_DEFINE_VALUE(Button, Select, 2);
		HL_ENUM_DEFINE_VALUE(Button, Mode, 3);
		HL_ENUM_DEFINE_VALUE(Button, A, 11);
		HL_ENUM_DEFINE_VALUE(Button, B, 12);
		HL_ENUM_DEFINE_VALUE(Button, C, 13);
		HL_ENUM_DEFINE_VALUE(Button, X, 14);
		HL_ENUM_DEFINE_VALUE(Button, Y, 15);
		HL_ENUM_DEFINE_VALUE(Button, Z, 16);
		HL_ENUM_DEFINE_VALUE(Button, L1, 21);
		HL_ENUM_DEFINE_VALUE(Button, R1, 22);
		HL_ENUM_DEFINE_VALUE(Button, L2, 23);
		HL_ENUM_DEFINE_VALUE(Button, R2, 24);
		HL_ENUM_DEFINE_VALUE(Button, LS, 31);
		HL_ENUM_DEFINE_VALUE(Button, RS, 32);
		HL_ENUM_DEFINE_VALUE(Button, DPadDown, 42);
		HL_ENUM_DEFINE_VALUE(Button, DPadLeft, 44);
		HL_ENUM_DEFINE_VALUE(Button, DPadRight, 46);
		HL_ENUM_DEFINE_VALUE(Button, DPadUp, 48);
		HL_ENUM_DEFINE_VALUE(Button, AxisLX, 100);
		HL_ENUM_DEFINE_VALUE(Button, AxisLY, 101);
		HL_ENUM_DEFINE_VALUE(Button, AxisRX, 102);
		HL_ENUM_DEFINE_VALUE(Button, AxisRY, 103);
		HL_ENUM_DEFINE_VALUE(Button, TriggerL, 111);
		HL_ENUM_DEFINE_VALUE(Button, TriggerR, 112);
	));

}
