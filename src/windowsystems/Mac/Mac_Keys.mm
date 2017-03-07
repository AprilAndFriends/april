/// @file
/// @version 4.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <Foundation/Foundation.h>

#include <hltypes/hlog.h>
#include <hltypes/hmap.h>

#include "april.h"
#include "Mac_Keys.h"

enum CarbonKeyCode
{
	kVK_ANSI_A                    = 0x00,
	kVK_ANSI_S                    = 0x01,
	kVK_ANSI_D                    = 0x02,
	kVK_ANSI_F                    = 0x03,
	kVK_ANSI_H                    = 0x04,
	kVK_ANSI_G                    = 0x05,
	kVK_ANSI_Z                    = 0x06,
	kVK_ANSI_X                    = 0x07,
	kVK_ANSI_C                    = 0x08,
	kVK_ANSI_V                    = 0x09,
	kVK_ANSI_B                    = 0x0B,
	kVK_ANSI_Q                    = 0x0C,
	kVK_ANSI_W                    = 0x0D,
	kVK_ANSI_E                    = 0x0E,
	kVK_ANSI_R                    = 0x0F,
	kVK_ANSI_Y                    = 0x10,
	kVK_ANSI_T                    = 0x11,
	kVK_ANSI_1                    = 0x12,
	kVK_ANSI_2                    = 0x13,
	kVK_ANSI_3                    = 0x14,
	kVK_ANSI_4                    = 0x15,
	kVK_ANSI_6                    = 0x16,
	kVK_ANSI_5                    = 0x17,
	kVK_ANSI_Equal                = 0x18,
	kVK_ANSI_9                    = 0x19,
	kVK_ANSI_7                    = 0x1A,
	kVK_ANSI_Minus                = 0x1B,
	kVK_ANSI_8                    = 0x1C,
	kVK_ANSI_0                    = 0x1D,
	kVK_ANSI_RightBracket         = 0x1E,
	kVK_ANSI_O                    = 0x1F,
	kVK_ANSI_U                    = 0x20,
	kVK_ANSI_LeftBracket          = 0x21,
	kVK_ANSI_I                    = 0x22,
	kVK_ANSI_P                    = 0x23,
	kVK_ANSI_L                    = 0x25,
	kVK_ANSI_J                    = 0x26,
	kVK_ANSI_Quote                = 0x27,
	kVK_ANSI_K                    = 0x28,
	kVK_ANSI_Semicolon            = 0x29,
	kVK_ANSI_Backslash            = 0x2A,
	kVK_ANSI_Comma                = 0x2B,
	kVK_ANSI_Slash                = 0x2C,
	kVK_ANSI_N                    = 0x2D,
	kVK_ANSI_M                    = 0x2E,
	kVK_ANSI_Period               = 0x2F,
	kVK_ANSI_Grave                = 0x32,
	kVK_ANSI_KeypadDecimal        = 0x41,
	kVK_ANSI_KeypadMultiply       = 0x43,
	kVK_ANSI_KeypadPlus           = 0x45,
	kVK_ANSI_KeypadClear          = 0x47,
	kVK_ANSI_KeypadDivide         = 0x4B,
	kVK_ANSI_KeypadEnter          = 0x4C,
	kVK_ANSI_KeypadMinus          = 0x4E,
	kVK_ANSI_KeypadEquals         = 0x51,
	kVK_ANSI_Keypad0              = 0x52,
	kVK_ANSI_Keypad1              = 0x53,
	kVK_ANSI_Keypad2              = 0x54,
	kVK_ANSI_Keypad3              = 0x55,
	kVK_ANSI_Keypad4              = 0x56,
	kVK_ANSI_Keypad5              = 0x57,
	kVK_ANSI_Keypad6              = 0x58,
	kVK_ANSI_Keypad7              = 0x59,
	kVK_ANSI_Keypad8              = 0x5B,
	kVK_ANSI_Keypad9              = 0x5C,

/* keycodes for keys that are independent of keyboard layout*/
	kVK_Return                    = 0x24,
	kVK_Tab                       = 0x30,
	kVK_Space                     = 0x31,
	kVK_Delete                    = 0x33,
	kVK_Escape                    = 0x35,
	kVK_Command                   = 0x37,
	kVK_Shift                     = 0x38,
	kVK_CapsLock                  = 0x39,
	kVK_Option                    = 0x3A,
	kVK_Control                   = 0x3B,
	kVK_RightShift                = 0x3C,
	kVK_RightOption               = 0x3D,
	kVK_RightControl              = 0x3E,
	kVK_Function                  = 0x3F,
	kVK_F17                       = 0x40,
	kVK_VolumeUp                  = 0x48,
	kVK_VolumeDown                = 0x49,
	kVK_Mute                      = 0x4A,
	kVK_F18                       = 0x4F,
	kVK_F19                       = 0x50,
	kVK_F20                       = 0x5A,
	kVK_F5                        = 0x60,
	kVK_F6                        = 0x61,
	kVK_F7                        = 0x62,
	kVK_F3                        = 0x63,
	kVK_F8                        = 0x64,
	kVK_F9                        = 0x65,
	kVK_F11                       = 0x67,
	kVK_F13                       = 0x69,
	kVK_F16                       = 0x6A,
	kVK_F14                       = 0x6B,
	kVK_F10                       = 0x6D,
	kVK_F12                       = 0x6F,
	kVK_F15                       = 0x71,
	kVK_Help                      = 0x72,
	kVK_Home                      = 0x73,
	kVK_PageUp                    = 0x74,
	kVK_ForwardDelete             = 0x75,
	kVK_F4                        = 0x76,
	kVK_End                       = 0x77,
	kVK_F2                        = 0x78,
	kVK_PageDown                  = 0x79,
	kVK_F1                        = 0x7A,
	kVK_LeftArrow                 = 0x7B,
	kVK_RightArrow                = 0x7C,
	kVK_DownArrow                 = 0x7D,
	kVK_UpArrow                   = 0x7E,

	kVK_ISO_Section               = 0x0A,

	kVK_JIS_Yen                   = 0x5D,
	kVK_JIS_Underscore            = 0x5E,
	kVK_JIS_KeypadComma           = 0x5F,
	kVK_JIS_Eisu                  = 0x66,
	kVK_JIS_Kana                  = 0x68
};

namespace april
{
	static hmap<unsigned int, Key> gMacKeyMap;
	
	Key getAprilMacKeyCode(unsigned int macKeyCode)
	{
		if (gMacKeyMap.hasKey(macKeyCode))
		{
			return gMacKeyMap[macKeyCode];
		}

#ifdef _DEBUG
		hlog::writef(logTag, "Unknown key code: %u", macKeyCode);
#endif
		return april::Key::None;
	}
	
	void initMacKeyMap()
	{
		hmap<unsigned int, Key>& m = gMacKeyMap;
		

		m[kVK_ANSI_A] = april::Key::A;
		m[kVK_ANSI_S] = april::Key::S;
		m[kVK_ANSI_D] = april::Key::D;
		m[kVK_ANSI_F] = april::Key::F;
		m[kVK_ANSI_H] = april::Key::H;
		m[kVK_ANSI_G] = april::Key::G;
		m[kVK_ANSI_Z] = april::Key::Z;
		m[kVK_ANSI_X] = april::Key::X;
		m[kVK_ANSI_C] = april::Key::C;
		m[kVK_ANSI_V] = april::Key::V;
		m[kVK_ANSI_B] = april::Key::B;
		m[kVK_ANSI_Q] = april::Key::Q;
		m[kVK_ANSI_W] = april::Key::W;
		m[kVK_ANSI_E] = april::Key::E;
		m[kVK_ANSI_R] = april::Key::R;
		m[kVK_ANSI_Y] = april::Key::Y;
		m[kVK_ANSI_T] = april::Key::T;
		m[kVK_ANSI_1] = april::Key::Num1;
		m[kVK_ANSI_2] = april::Key::Num2;
		m[kVK_ANSI_3] = april::Key::Num3;
		m[kVK_ANSI_4] = april::Key::Num4;
		m[kVK_ANSI_6] = april::Key::Num5;
		m[kVK_ANSI_5] = april::Key::Num6;
		m[kVK_ANSI_7] = april::Key::Num7;
		m[kVK_ANSI_8] = april::Key::Num8;
		m[kVK_ANSI_9] = april::Key::Num9;
		m[kVK_ANSI_0] = april::Key::Num0;
		m[kVK_ANSI_O] = april::Key::O;
		m[kVK_ANSI_U] = april::Key::U;
		m[kVK_ANSI_I] = april::Key::I;
		m[kVK_ANSI_P] = april::Key::P;
		m[kVK_ANSI_L] = april::Key::L;
		m[kVK_ANSI_J] = april::Key::J;
		m[kVK_ANSI_K] = april::Key::K;
		m[kVK_ANSI_N] = april::Key::N;
		m[kVK_ANSI_M] = april::Key::M;
		m[kVK_ANSI_Keypad0] = april::Key::NumPad0;
		m[kVK_ANSI_Keypad1] = april::Key::NumPad1;
		m[kVK_ANSI_Keypad2] = april::Key::NumPad2;
		m[kVK_ANSI_Keypad3] = april::Key::NumPad3;
		m[kVK_ANSI_Keypad4] = april::Key::NumPad4;
		m[kVK_ANSI_Keypad5] = april::Key::NumPad5;
		m[kVK_ANSI_Keypad6] = april::Key::NumPad6;
		m[kVK_ANSI_Keypad7] = april::Key::NumPad7;
		m[kVK_ANSI_Keypad8] = april::Key::NumPad8;
		m[kVK_ANSI_Keypad9] = april::Key::NumPad9;
		m[kVK_Return] = april::Key::Return;
		m[kVK_Tab] = april::Key::Tab;
		m[kVK_Space] = april::Key::Space;
		m[kVK_Delete] = april::Key::Backspace;
		m[kVK_Escape] = april::Key::Escape;
		m[kVK_Shift] = april::Key::Shift;
		m[kVK_CapsLock] = april::Key::CapsLock;
		m[kVK_Option] = april::Key::Menu;
		m[kVK_Control] = april::Key::Control;
		m[kVK_RightShift] = april::Key::ShiftR;
		m[kVK_RightOption] = april::Key::MenuR;
		m[kVK_RightControl] = april::Key::ControlR;
		m[kVK_VolumeUp] = april::Key::VolumeUp;
		m[kVK_VolumeDown] = april::Key::VolumeDown;
		m[kVK_Mute] = april::Key::VolumeMute;
		m[kVK_F1] = april::Key::F1;
		m[kVK_F2] = april::Key::F2;
		m[kVK_F3] = april::Key::F3;
		m[kVK_F4] = april::Key::F4;
		m[kVK_F5] = april::Key::F5;
		m[kVK_F6] = april::Key::F6;
		m[kVK_F7] = april::Key::F7;
		m[kVK_F8] = april::Key::F8;
		m[kVK_F9] = april::Key::F9;
		m[kVK_F10] = april::Key::F10;
		m[kVK_F11] = april::Key::F11;
		m[kVK_F12] = april::Key::F12;
		m[kVK_F13] = april::Key::F13;
		m[kVK_F14] = april::Key::F14;
		m[kVK_F15] = april::Key::F15;
		m[kVK_F16] = april::Key::F16;
		m[kVK_F17] = april::Key::F17;
		m[kVK_F18] = april::Key::F18;
		m[kVK_F19] = april::Key::F19;
		m[kVK_F20] = april::Key::F20;
		m[kVK_Help] = april::Key::Help;
		m[kVK_Home] = april::Key::Home;
		m[kVK_PageUp] = april::Key::Prior;
		m[kVK_ForwardDelete] = april::Key::Delete;
		
		m[kVK_End] = april::Key::E;
		m[kVK_PageDown] = april::Key::Next;
		m[kVK_LeftArrow] = april::Key::ArrowLeft;
		m[kVK_RightArrow] = april::Key::ArrowRight;
		m[kVK_DownArrow] = april::Key::ArrowDown;
		m[kVK_UpArrow] = april::Key::ArrowUp;
		m[kVK_ANSI_KeypadEnter] = april::Key::Return;
//		m[kVK_ISO_Section] = april::Key::;
//		m[kVK_JIS_Yen] = april::Key::;
//		m[kVK_JIS_Underscore] = april::Key::;
//		m[kVK_JIS_KeypadComma] = april::Key::;
//		m[kVK_JIS_Eisu] = april::Key::;
//		m[kVK_JIS_Kana] = april::Key::;
//		m[kVK_Function] = april::Key::;
//		m[kVK_ANSI_Equal] = april::Key::;
//		m[kVK_ANSI_Minus] = april::Key::;
//		m[kVK_ANSI_RightBracket] = april::Key::;
//		m[kVK_ANSI_LeftBracket] = april::Key::;
//		m[kVK_ANSI_Quote] = april::Key::;
//		m[kVK_ANSI_Semicolon] = april::Key::;
//		m[kVK_ANSI_Backslash] = april::Key::;
//		m[kVK_ANSI_Comma] = april::Key::;
//		m[kVK_ANSI_Slash] = april::Key::;
//		m[kVK_ANSI_Period] = april::Key::;
//		m[kVK_ANSI_Grave] = april::Key::;
//		m[kVK_ANSI_KeypadDecimal] = april::Key::;
//		m[kVK_ANSI_KeypadMultiply] = april::Key::;
//		m[kVK_ANSI_KeypadPlus] = april::Key::;
//		m[kVK_ANSI_KeypadClear] = april::Key::;
//		m[kVK_ANSI_KeypadDivide] = april::Key::;
//		m[kVK_ANSI_KeypadMinus] = april::Key::;
//		m[kVK_ANSI_KeypadEquals] = april::Key::;
		m[kVK_Command] = april::Key::CommandL;
		m[0x36] = april::Key::CommandR;
	}
}
