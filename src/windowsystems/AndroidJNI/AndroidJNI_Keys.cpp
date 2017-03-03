/// @file
/// @version 4.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _ANDROID

#include <hltypes/hlog.h>
#include <hltypes/hmap.h>

#include "april.h"
#include "AndroidJNI_Keys.h"

namespace april
{
	static hmap<int, Key> gKeyMap;
	
	Key android2april(int androidKeyCode)
	{
		if (gKeyMap.hasKey(androidKeyCode))
		{
			return gKeyMap[androidKeyCode];
		}
#ifdef _DEBUG
		hlog::writef(logTag, "Unknown key code: %u", androidKeyCode);
#endif
		return april::Key::None;
	}
	
	void initAndroidKeyMap()
	{
		hmap<int, Key>& m = gKeyMap;

		// codes obtained from http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_0
		m[0] = Key::None;
		
		// LBUTTON and RBUTTON are necessary
		m[1] = Key::MouseL;
		m[2] = Key::MouseR;
		
		// most common keys; some not supported
		m[67] = Key::Backspace;
		m[61] = Key::Tab;
		m[28] = Key::Clear;
		m[66] = Key::Return;
		m[82] = Key::Menu;
		m[115] = Key::Capital;
		
		// various keys for asian keyboards not supported
		m[4] = Key::Escape; // using Android's back button for this
		// space
		m[62] = Key::Space;
		
		// some more keys
		m[124] = Key::Insert;
		m[112] = Key::Delete;
		
		// '0'-'9'
		m[7] = Key::Num0;
		m[8] = Key::Num1;
		m[9] = Key::Num2;
		m[10] = Key::Num3;
		m[11] = Key::Num4;
		m[12] = Key::Num5;
		m[13] = Key::Num6;
		m[14] = Key::Num7;
		m[15] = Key::Num8;
		m[16] = Key::Num9;
		
		// 'A'-'Z'
		m[29] = Key::A;
		m[30] = Key::B;
		m[31] = Key::C;
		m[32] = Key::D;
		m[33] = Key::E;
		m[34] = Key::F;
		m[35] = Key::G;
		m[36] = Key::H;
		m[37] = Key::I;
		m[38] = Key::J;
		m[39] = Key::K;
		m[40] = Key::L;
		m[41] = Key::M;
		m[42] = Key::N;
		m[43] = Key::O;
		m[44] = Key::P;
		m[45] = Key::Q;
		m[46] = Key::R;
		m[47] = Key::S;
		m[48] = Key::T;
		m[49] = Key::U;
		m[50] = Key::V;
		m[51] = Key::W;
		m[52] = Key::X;
		m[53] = Key::Y;
		m[54] = Key::Z;
		
		// numpad
		m[144] = Key::NumPad0;
		m[145] = Key::NumPad1;
		m[146] = Key::NumPad2;
		m[147] = Key::NumPad3;
		m[148] = Key::NumPad4;
		m[149] = Key::NumPad5;
		m[150] = Key::NumPad6;
		m[151] = Key::NumPad7;
		m[152] = Key::NumPad8;
		m[153] = Key::NumPad9;
		m[155] = Key::Multiply;
		m[157] = Key::Add;
		m[159] = Key::Separator;
		m[156] = Key::Subtract;
		m[158] = Key::Decimal;
		m[154] = Key::Divide;
		
		// F-keys
		m[131] = Key::F1;
		m[132] = Key::F2;
		m[133] = Key::F3;
		m[134] = Key::F4;
		m[135] = Key::F5;
		m[136] = Key::F6;
		m[137] = Key::F7;
		m[138] = Key::F8;
		m[139] = Key::F9;
		m[140] = Key::F10;
		m[141] = Key::F11;
		m[142] = Key::F12;
		
		// don't exist on Android
		m[143] = Key::NumLock;
		m[116] = Key::Scroll;
		
		// specific left-and-right-shift keys
		m[59] = Key::ShiftL;
		m[60] = Key::ShiftR;
		m[113] = Key::ControlL;
		m[114] = Key::ControlR;
		m[57] = Key::MenuL;
		m[58] = Key::MenuR;
		
		// browser control keys
		//m[4] = Key::BrowserBack;
		//m[125] = Key::BrowserForward;
		//m[84] = Key::BrowserSearch;
		
		// volume keys
		m[164] = Key::VolumeMute;
		m[25] = Key::VolumeDown;
		m[24] = Key::VolumeUp;
	}
}
#endif
