/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
#include <sys/sysctl.h>
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
#include "RenderSystem.h"
#include "Window.h"
#include "SDL_Window.h"

namespace april
{
	bool platform_CursorIsVisible()
	{
		return CGCursorIsVisible();
	}
	
	void platform_cursorVisibilityUpdate()
	{
		// mac only: extra visibility handling
		NSWindow* window = [[NSApplication sharedApplication] keyWindow];
		bool shouldShow;
		
		if (!april::window->isCursorVisible())
		{
			//NSPoint 	mouseLoc = [window convertScreenToBase:[NSEvent mouseLocation]];
			//[window frame]
			NSPoint mouseLoc;
			id hideInsideView; // either NSView or NSWindow; both implement "frame" method
			if ([window contentView])
			{
				hideInsideView = [window contentView];
				mouseLoc = [window convertScreenToBase:[NSEvent mouseLocation]];
			}
			else
			{
				hideInsideView = window;
				mouseLoc = [NSEvent mouseLocation];
			}
			
			if (hideInsideView)
			{
				shouldShow = !NSPointInRect(mouseLoc, [hideInsideView frame]);
			}
			else // no view? let's presume we are in fullscreen where we should blindly honor the requests from the game
			{
				shouldShow = false;
			}
		}
		else
		{			
			shouldShow = true;
		}
		
		if (!shouldShow && CGCursorIsVisible())
		{
			CGDisplayHideCursor(kCGDirectMainDisplay);
		}
		else if (shouldShow && !CGCursorIsVisible())
		{
			CGDisplayShowCursor(kCGDirectMainDisplay);
		}
	}
	
	SystemInfo getSystemInfo()
	{
		// TODO
		static SystemInfo info;
		if (info.locale == "")
		{
			// CPU cores
			info.cpuCores = sysconf(_SC_NPROCESSORS_ONLN);
			// RAM
			info.name = "mac";

			int mib [] = { CTL_HW, HW_MEMSIZE };
			int64_t value = 0;
			size_t length = sizeof(value);

			if (sysctl(mib, 2, &value, &length, NULL, 0) == -1)
				info.ram = 2048;
			else
				info.ram = value / (1024 * 1024);

			// display resolution
			NSScreen* mainScreen = [NSScreen mainScreen];
			NSRect rect = [mainScreen frame];
			info.displayResolution.set((float)rect.size.width, (float)rect.size.height);
			// display DPI
			info.displayDpi = 72; // TODO
			info.maxTextureSize = 0;
			// locale
			info.locale = [[[NSLocale preferredLanguages] objectAtIndex:0] UTF8String];
		}
		// TODO
		if (info.maxTextureSize == 0 && april::window != NULL && ((SDL_Window*) april::window)->isCreated()) // SDL window cast is a hack, TODO fix this systematically
		{
			info.maxTextureSize = april::rendersys->_getMaxTextureSize();
		}
		return info;
	}
	
	DeviceType getDeviceType()
	{
		return DEVICE_MAC_PC;
	}
    
	hstr getPackageName()
	{
		hlog::warn("Cannot use getPackageName() on this platform.");
		return "";
	}

	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
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
//		NSString *textns = [NSString stringWithUTF8String:text.c_str()];
		
		int clicked = NSRunAlertPanel(titlens, @"%s", buttons[0], buttons[1], buttons[2], text.c_str());
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
	}
	
}
#endif
