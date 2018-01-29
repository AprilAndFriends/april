/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a key code table.

#ifndef APRIL_KEYS_H
#define APRIL_KEYS_H

#include <hltypes/henum.h>

#include "aprilExport.h"

namespace april
{
	/// @class Key
	/// @brief Defines virtual key codes.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Key,
	(
		HL_ENUM_DECLARE(Key, None);
		HL_ENUM_DECLARE(Key, MouseL);
		HL_ENUM_DECLARE(Key, MouseR);
		HL_ENUM_DECLARE(Key, Cancel);
		HL_ENUM_DECLARE(Key, ScrollUp);
		HL_ENUM_DECLARE(Key, ScrollDown);
		HL_ENUM_DECLARE(Key, MouseM);
		HL_ENUM_DECLARE(Key, DoubleTap);
		HL_ENUM_DECLARE(Key, Backspace);
		HL_ENUM_DECLARE(Key, Tab);
		HL_ENUM_DECLARE(Key, Clear);
		HL_ENUM_DECLARE(Key, Return);
		HL_ENUM_DECLARE(Key, Shift);
		HL_ENUM_DECLARE(Key, Control);
		HL_ENUM_DECLARE(Key, Menu);
		HL_ENUM_DECLARE(Key, Pause);
		HL_ENUM_DECLARE(Key, CapsLock);
		HL_ENUM_DECLARE(Key, KanaHangul);
		HL_ENUM_DECLARE(Key, Junja);
		HL_ENUM_DECLARE(Key, Final);
		HL_ENUM_DECLARE(Key, HanjaKanji);
		HL_ENUM_DECLARE(Key, Escape);
		HL_ENUM_DECLARE(Key, Convert);
		HL_ENUM_DECLARE(Key, NonConvert);
		HL_ENUM_DECLARE(Key, Accept);
		HL_ENUM_DECLARE(Key, ModeChange);
		HL_ENUM_DECLARE(Key, CommandL);
		HL_ENUM_DECLARE(Key, CommandR);
		HL_ENUM_DECLARE(Key, Space);
		HL_ENUM_DECLARE(Key, Prior);
		HL_ENUM_DECLARE(Key, Next);
		HL_ENUM_DECLARE(Key, End);
		HL_ENUM_DECLARE(Key, Home);
		HL_ENUM_DECLARE(Key, ArrowLeft);
		HL_ENUM_DECLARE(Key, ArrowUp);
		HL_ENUM_DECLARE(Key, ArrowRight);
		HL_ENUM_DECLARE(Key, ArrowDown);
		HL_ENUM_DECLARE(Key, Select);
		HL_ENUM_DECLARE(Key, Print);
		HL_ENUM_DECLARE(Key, Execute);
		HL_ENUM_DECLARE(Key, PrintScreen);
		HL_ENUM_DECLARE(Key, Insert);
		HL_ENUM_DECLARE(Key, Delete);
		HL_ENUM_DECLARE(Key, Help);
		HL_ENUM_DECLARE(Key, Num0);
		HL_ENUM_DECLARE(Key, Num1);
		HL_ENUM_DECLARE(Key, Num2);
		HL_ENUM_DECLARE(Key, Num3);
		HL_ENUM_DECLARE(Key, Num4);
		HL_ENUM_DECLARE(Key, Num5);
		HL_ENUM_DECLARE(Key, Num6);
		HL_ENUM_DECLARE(Key, Num7);
		HL_ENUM_DECLARE(Key, Num8);
		HL_ENUM_DECLARE(Key, Num9);
		HL_ENUM_DECLARE(Key, A);
		HL_ENUM_DECLARE(Key, B);
		HL_ENUM_DECLARE(Key, C);
		HL_ENUM_DECLARE(Key, D);
		HL_ENUM_DECLARE(Key, E);
		HL_ENUM_DECLARE(Key, F);
		HL_ENUM_DECLARE(Key, G);
		HL_ENUM_DECLARE(Key, H);
		HL_ENUM_DECLARE(Key, I);
		HL_ENUM_DECLARE(Key, J);
		HL_ENUM_DECLARE(Key, K);
		HL_ENUM_DECLARE(Key, L);
		HL_ENUM_DECLARE(Key, M);
		HL_ENUM_DECLARE(Key, N);
		HL_ENUM_DECLARE(Key, O);
		HL_ENUM_DECLARE(Key, P);
		HL_ENUM_DECLARE(Key, Q);
		HL_ENUM_DECLARE(Key, R);
		HL_ENUM_DECLARE(Key, S);
		HL_ENUM_DECLARE(Key, T);
		HL_ENUM_DECLARE(Key, U);
		HL_ENUM_DECLARE(Key, V);
		HL_ENUM_DECLARE(Key, W);
		HL_ENUM_DECLARE(Key, X);
		HL_ENUM_DECLARE(Key, Y);
		HL_ENUM_DECLARE(Key, Z);
		HL_ENUM_DECLARE(Key, WindowsL);
		HL_ENUM_DECLARE(Key, WindowsR);
		HL_ENUM_DECLARE(Key, Apps);
		HL_ENUM_DECLARE(Key, Sleep);
		HL_ENUM_DECLARE(Key, NumPad0);
		HL_ENUM_DECLARE(Key, NumPad1);
		HL_ENUM_DECLARE(Key, NumPad2);
		HL_ENUM_DECLARE(Key, NumPad3);
		HL_ENUM_DECLARE(Key, NumPad4);
		HL_ENUM_DECLARE(Key, NumPad5);
		HL_ENUM_DECLARE(Key, NumPad6);
		HL_ENUM_DECLARE(Key, NumPad7);
		HL_ENUM_DECLARE(Key, NumPad8);
		HL_ENUM_DECLARE(Key, NumPad9);
		HL_ENUM_DECLARE(Key, Multiply);
		HL_ENUM_DECLARE(Key, Add);
		HL_ENUM_DECLARE(Key, Separator);
		HL_ENUM_DECLARE(Key, Subtract);
		HL_ENUM_DECLARE(Key, Decimal);
		HL_ENUM_DECLARE(Key, Divide);
		HL_ENUM_DECLARE(Key, F1);
		HL_ENUM_DECLARE(Key, F2);
		HL_ENUM_DECLARE(Key, F3);
		HL_ENUM_DECLARE(Key, F4);
		HL_ENUM_DECLARE(Key, F5);
		HL_ENUM_DECLARE(Key, F6);
		HL_ENUM_DECLARE(Key, F7);
		HL_ENUM_DECLARE(Key, F8);
		HL_ENUM_DECLARE(Key, F9);
		HL_ENUM_DECLARE(Key, F10);
		HL_ENUM_DECLARE(Key, F11);
		HL_ENUM_DECLARE(Key, F12);
		HL_ENUM_DECLARE(Key, F13);
		HL_ENUM_DECLARE(Key, F14);
		HL_ENUM_DECLARE(Key, F15);
		HL_ENUM_DECLARE(Key, F16);
		HL_ENUM_DECLARE(Key, F17);
		HL_ENUM_DECLARE(Key, F18);
		HL_ENUM_DECLARE(Key, F19);
		HL_ENUM_DECLARE(Key, F20);
		HL_ENUM_DECLARE(Key, F21);
		HL_ENUM_DECLARE(Key, F22);
		HL_ENUM_DECLARE(Key, F23);
		HL_ENUM_DECLARE(Key, F24);
		HL_ENUM_DECLARE(Key, NumLock);
		HL_ENUM_DECLARE(Key, ScrollLock);
		HL_ENUM_DECLARE(Key, ShiftL);
		HL_ENUM_DECLARE(Key, ShiftR);
		HL_ENUM_DECLARE(Key, ControlL);
		HL_ENUM_DECLARE(Key, ControlR);
		HL_ENUM_DECLARE(Key, MenuL);
		HL_ENUM_DECLARE(Key, MenuR);
		HL_ENUM_DECLARE(Key, BrowserBack);
		HL_ENUM_DECLARE(Key, BrowserForward);
		HL_ENUM_DECLARE(Key, BrowserRefresh);
		HL_ENUM_DECLARE(Key, BrowserStop);
		HL_ENUM_DECLARE(Key, BrowserSearch);
		HL_ENUM_DECLARE(Key, BrowserFavorites);
		HL_ENUM_DECLARE(Key, BrowserHome);
		HL_ENUM_DECLARE(Key, VolumeMute);
		HL_ENUM_DECLARE(Key, VolumeDown);
		HL_ENUM_DECLARE(Key, VolumeUp);
		HL_ENUM_DECLARE(Key, MediaNextTrack);
		HL_ENUM_DECLARE(Key, MediaPreviousTrack);
		HL_ENUM_DECLARE(Key, MediaStop);
		HL_ENUM_DECLARE(Key, MediaPlayPause);
		HL_ENUM_DECLARE(Key, LaunchMail);
		HL_ENUM_DECLARE(Key, LaunchMediaSelect);
		HL_ENUM_DECLARE(Key, LaunchApp1);
		HL_ENUM_DECLARE(Key, LaunchApp2);
		HL_ENUM_DECLARE(Key, Oem2);
		HL_ENUM_DECLARE(Key, Oem3);
		HL_ENUM_DECLARE(Key, Oem4);
		HL_ENUM_DECLARE(Key, Oem5);
		HL_ENUM_DECLARE(Key, Oem6);
		HL_ENUM_DECLARE(Key, Oem7);
		HL_ENUM_DECLARE(Key, Oem8);
		HL_ENUM_DECLARE(Key, Oem102);
		HL_ENUM_DECLARE(Key, Packet);
		HL_ENUM_DECLARE(Key, Attn);
		HL_ENUM_DECLARE(Key, Crsel);
		HL_ENUM_DECLARE(Key, Exsel);
		HL_ENUM_DECLARE(Key, Ereof);
		HL_ENUM_DECLARE(Key, Play);
		HL_ENUM_DECLARE(Key, Zoom);
		HL_ENUM_DECLARE(Key, NoName);
		HL_ENUM_DECLARE(Key, Pa1);
		HL_ENUM_DECLARE(Key, OemClear);
	));

	/// @class Button
	/// @brief Controller buttons.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Button,
	(
		HL_ENUM_DECLARE(Button, None);
		HL_ENUM_DECLARE(Button, Start);
		HL_ENUM_DECLARE(Button, Select);
		HL_ENUM_DECLARE(Button, Mode);
		HL_ENUM_DECLARE(Button, A);
		HL_ENUM_DECLARE(Button, B);
		HL_ENUM_DECLARE(Button, C);
		HL_ENUM_DECLARE(Button, X);
		HL_ENUM_DECLARE(Button, Y);
		HL_ENUM_DECLARE(Button, Z);
		HL_ENUM_DECLARE(Button, L1);
		HL_ENUM_DECLARE(Button, R1);
		HL_ENUM_DECLARE(Button, L2);
		HL_ENUM_DECLARE(Button, R2);
		HL_ENUM_DECLARE(Button, LS);
		HL_ENUM_DECLARE(Button, RS);
		HL_ENUM_DECLARE(Button, DPadDown);
		HL_ENUM_DECLARE(Button, DPadLeft);
		HL_ENUM_DECLARE(Button, DPadRight);
		HL_ENUM_DECLARE(Button, DPadUp);
		HL_ENUM_DECLARE(Button, AxisLX);
		HL_ENUM_DECLARE(Button, AxisLY);
		HL_ENUM_DECLARE(Button, AxisRX);
		HL_ENUM_DECLARE(Button, AxisRY);
		HL_ENUM_DECLARE(Button, TriggerL);
		HL_ENUM_DECLARE(Button, TriggerR);
	));

}
#endif
