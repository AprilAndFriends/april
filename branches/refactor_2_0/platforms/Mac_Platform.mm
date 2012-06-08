/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#import <AppKit/NSApplication.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSPanel.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSWindow.h>
#import <Foundation/NSString.h>

#include <gtypes/Vector2.h>

#include "Platform.h"

namespace april
{
	gvec2 getDisplayResolution()
	{
		NSScreen* mainScreen = [NSScreen mainScreen];
		NSRect rect = [mainScreen frame];
		return gvec2(rect.size.width, rect.size.height);
	}

	SystemInfo getSystemInfo()
	{
		// TODO
		static SystemInfo info;
		if (info.locale == "")
		{
			info.cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
			info.ram = 1024;
			info.max_texture_size = 0;
			info.locale = "en";
		}
		// TODO
		if (info.max_texture_size == 0 && april::rendersys != NULL)
		{
			info.max_texture_size = april::rendersys->_getMaxTextureSize();
		}
		return info;
	}
	
	DeviceType getDeviceType()
	{
		return DEVICE_MAC_PC;
	}
    
	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		// fugly implementation of showing messagebox on mac os
		// ideas:
		// * display as a sheet attached on top of the current window
		// * prioritize buttons and automatically assign slots
		// * use constants for button captions
		// * use an array with constants for button captions etc
		
		NSString *buttons[] = {@"OK", nil, nil}; // set all buttons to nil, at first, except default one, just in case
		MessageBoxButton buttonTypes[] = {AMSGBTN_OK, AMSGBTN_NULL, AMSGBTN_NULL};
        
		if (buttonMask & AMSGBTN_OK && buttonMask & AMSGBTN_CANCEL)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_OK, "OK").c_str()];
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            
            buttonTypes[0] = AMSGBTN_OK;
            buttonTypes[1] = AMSGBTN_CANCEL;
		}
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO && buttonMask & AMSGBTN_CANCEL)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
			buttons[2] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            
            buttonTypes[0] = AMSGBTN_YES;
            buttonTypes[1] = AMSGBTN_NO;
            buttonTypes[2] = AMSGBTN_CANCEL;

		}
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
            
            buttonTypes[0] = AMSGBTN_YES;
            buttonTypes[1] = AMSGBTN_NO;
		}
		else if (buttonMask & AMSGBTN_CANCEL)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            buttonTypes[0] = AMSGBTN_CANCEL;
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_OK, "OK").c_str()];
            buttonTypes[0] = AMSGBTN_OK;
		}
		else if (buttonMask & AMSGBTN_YES)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
            buttonTypes[0] = AMSGBTN_YES;
		}
		else if (buttonMask & AMSGBTN_NO)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
            buttonTypes[0] = AMSGBTN_NO;
		}
		
		NSString *titlens = [NSString stringWithUTF8String:title.c_str()];
		NSString *textns = [NSString stringWithUTF8String:text.c_str()];
		
		int clicked = NSRunAlertPanel(titlens, textns, buttons[0], buttons[1], buttons[2]);
		switch (clicked)
		{
		case NSAlertDefaultReturn:
			clicked = 0;
			break;
		case NSAlertAlternateReturn:
			clicked = 1;
			break;
		case NSAlertOtherReturn:
			clicked = 2;
			break;
		}
        
        if (callback != NULL)
        {
            (*callback)(buttonTypes[clicked]);
        }
        return buttonTypes[clicked];
	}
	
}
#endif
