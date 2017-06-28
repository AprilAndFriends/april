/// @file
/// @version 4.4
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENKODE_WINDOW
#include <KD/kd.h>
#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include "april.h"
#include "OpenKODE_Keys.h"

namespace april
{
	static hmap<int, Key> gKeyMap;
	
	Key kd2april(int kdKeyCode)
	{
		if (gKeyMap.hasKey(kdKeyCode))
		{
			return gKeyMap[kdKeyCode];
		}
#ifdef _DEBUG
		hlog::writef(logTag, "Unknown key code: %u", kdKeyCode);
#endif
		return april::Key::None;
	}
	
	void initOpenKODEKeyMap()
	{
		hmap<int, Key>& m = gKeyMap;
		m[KD_INPUT_KEYS_LBUTTON] = Key::MouseL;
		m[KD_INPUT_KEYS_RBUTTON] = Key::MouseR;
		m[KD_INPUT_KEYS_MBUTTON] = Key::MouseM;
		m[KD_INPUT_KEYS_ESCAPE] = Key::Escape;
		m[KD_INPUT_KEYS_BACKSPACE] = Key::Backspace;
		m[KD_INPUT_KEYS_TAB] = Key::Tab;
		m[KD_INPUT_KEYS_ENTER] = Key::Return;
		m[KD_INPUT_KEYS_SPACE] = Key::Space;
		m[KD_INPUT_KEYS_SHIFT] = Key::Shift;
		m[KD_INPUT_KEYS_CTRL] = Key::Control;
		m[KD_INPUT_KEYS_ALT] = Key::Menu;
		m[KD_INPUT_KEYS_LWIN] = Key::CommandL;
		m[KD_INPUT_KEYS_RWIN] = Key::CommandR;
		m[KD_INPUT_KEYS_APPS] = Key::Apps;
		m[KD_INPUT_KEYS_PAUSE] = Key::Pause;
		m[KD_INPUT_KEYS_CAPSLOCK] = Key::CapsLock;
		m[KD_INPUT_KEYS_NUMLOCK] = Key::NumLock;
		m[KD_INPUT_KEYS_PGUP] = Key::Prior;
		m[KD_INPUT_KEYS_PGDN] = Key::Next;
		m[KD_INPUT_KEYS_HOME] = Key::Home;
		m[KD_INPUT_KEYS_END] = Key::End;
		m[KD_INPUT_KEYS_INSERT] = Key::Insert;
		m[KD_INPUT_KEYS_DELETE] = Key::Delete;
		m[KD_INPUT_KEYS_LEFT] = Key::ArrowLeft;
		m[KD_INPUT_KEYS_UP] = Key::ArrowUp;
		m[KD_INPUT_KEYS_RIGHT] = Key::ArrowRight;
		m[KD_INPUT_KEYS_DOWN] = Key::ArrowLeft;
		m[KD_INPUT_KEYS_0] = Key::Num0;
		m[KD_INPUT_KEYS_1] = Key::Num1;
		m[KD_INPUT_KEYS_2] = Key::Num2;
		m[KD_INPUT_KEYS_3] = Key::Num3;
		m[KD_INPUT_KEYS_4] = Key::Num4;
		m[KD_INPUT_KEYS_5] = Key::Num5;
		m[KD_INPUT_KEYS_6] = Key::Num6;
		m[KD_INPUT_KEYS_7] = Key::Num7;
		m[KD_INPUT_KEYS_8] = Key::Num8;
		m[KD_INPUT_KEYS_9] = Key::Num9;
		m[KD_INPUT_KEYS_A] = Key::A;
		m[KD_INPUT_KEYS_B] = Key::B;
		m[KD_INPUT_KEYS_C] = Key::C;
		m[KD_INPUT_KEYS_D] = Key::D;
		m[KD_INPUT_KEYS_E] = Key::E;
		m[KD_INPUT_KEYS_F] = Key::F;
		m[KD_INPUT_KEYS_G] = Key::G;
		m[KD_INPUT_KEYS_H] = Key::H;
		m[KD_INPUT_KEYS_I] = Key::I;
		m[KD_INPUT_KEYS_J] = Key::J;
		m[KD_INPUT_KEYS_K] = Key::K;
		m[KD_INPUT_KEYS_L] = Key::L;
		m[KD_INPUT_KEYS_M] = Key::M;
		m[KD_INPUT_KEYS_N] = Key::N;
		m[KD_INPUT_KEYS_O] = Key::O;
		m[KD_INPUT_KEYS_P] = Key::P;
		m[KD_INPUT_KEYS_Q] = Key::Q;
		m[KD_INPUT_KEYS_R] = Key::R;
		m[KD_INPUT_KEYS_S] = Key::S;
		m[KD_INPUT_KEYS_T] = Key::T;
		m[KD_INPUT_KEYS_U] = Key::U;
		m[KD_INPUT_KEYS_V] = Key::V;
		m[KD_INPUT_KEYS_W] = Key::W;
		m[KD_INPUT_KEYS_X] = Key::X;
		m[KD_INPUT_KEYS_Y] = Key::Y;
		m[KD_INPUT_KEYS_Z] = Key::Z;
		m[KD_INPUT_KEYS_NUMPAD0] = Key::NumPad0;
		m[KD_INPUT_KEYS_NUMPAD1] = Key::NumPad1;
		m[KD_INPUT_KEYS_NUMPAD2] = Key::NumPad2;
		m[KD_INPUT_KEYS_NUMPAD3] = Key::NumPad3;
		m[KD_INPUT_KEYS_NUMPAD4] = Key::NumPad4;
		m[KD_INPUT_KEYS_NUMPAD5] = Key::NumPad5;
		m[KD_INPUT_KEYS_NUMPAD6] = Key::NumPad6;
		m[KD_INPUT_KEYS_NUMPAD7] = Key::NumPad7;
		m[KD_INPUT_KEYS_NUMPAD8] = Key::NumPad8;
		m[KD_INPUT_KEYS_NUMPAD9] = Key::NumPad9;
		m[KD_INPUT_KEYS_MULTIPLY] = Key::Multiply;
		m[KD_INPUT_KEYS_DIVIDE] = Key::Divide;
		m[KD_INPUT_KEYS_ADD] = Key::Add;
		m[KD_INPUT_KEYS_SUBTRACT] = Key::Subtract;
		m[KD_INPUT_KEYS_DECIMAL] = Key::Decimal;
		m[KD_INPUT_KEYS_F1] = Key::F1;
		m[KD_INPUT_KEYS_F2] = Key::F2;
		m[KD_INPUT_KEYS_F3] = Key::F3;
		m[KD_INPUT_KEYS_F4] = Key::F4;
		m[KD_INPUT_KEYS_F5] = Key::F5;
		m[KD_INPUT_KEYS_F6] = Key::F6;
		m[KD_INPUT_KEYS_F7] = Key::F7;
		m[KD_INPUT_KEYS_F8] = Key::F8;
		m[KD_INPUT_KEYS_F9] = Key::F9;
		m[KD_INPUT_KEYS_F10] = Key::F10;
		m[KD_INPUT_KEYS_F11] = Key::F11;
		m[KD_INPUT_KEYS_F12] = Key::F12;
		m[KD_INPUT_KEYS_UNKNOWN] = Key::None;
		
//		m[KD_INPUT_KEYS_SCROLLLOCK] = Key::;
//		m[KD_INPUT_KEYS_GRAVE] = Key::;
//		m[KD_INPUT_KEYS_MINUS] = Key::;
//		m[KD_INPUT_KEYS_EQUALS] = Key::;
//		m[KD_INPUT_KEYS_BACKSLASH] = Key::;
//		m[KD_INPUT_KEYS_LBRACKET] = Key::;
//		m[KD_INPUT_KEYS_RBRACKET] = Key::;
//		m[KD_INPUT_KEYS_SEMICOLON] = Key::;
//		m[KD_INPUT_KEYS_APOSTROPHE] = Key::;
//		m[KD_INPUT_KEYS_COMMA] = Key::;
//		m[KD_INPUT_KEYS_PERIOD] = Key::;
//		m[KD_INPUT_KEYS_SLASH] = Key::;
	}
}

#endif
