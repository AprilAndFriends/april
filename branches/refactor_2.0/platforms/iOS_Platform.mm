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
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#include "iOSWindow.h"

#include <gtypes/Vector2.h>
#include <hltypes/hltypesUtil.h>

#include "iOSWindow.h"
#include "Platform.h"

namespace april
{
	gvec2 getDisplayResolution()
	{
		UIScreen* mainScreen = [UIScreen mainScreen];
		float scale = 1.0f;
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		if ([mainScreen respondsToSelector:@selector(scale)])
		{
			scale = [mainScreen scale];
		}
#endif
		int w = mainScreen.bounds.size.width * scale;
		int h = mainScreen.bounds.size.height * scale;
		// forcing a w:h ratio where w > h
		return gvec2((float)hmax(w, h), (float)hmin(w, h));
	}

	SystemInfo getSystemInfo()
	{
		static SystemInfo info;
		info.cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
		if (info.name == "")
		{
			info.locale = [[[NSLocale preferredLanguages] objectAtIndex:0] UTF8String];
			
			size_t size = 255;
			char cname[256] = {'\0'};
			sysctlbyname("hw.machine", cname, &size, NULL, 0);
			hstr name = cname;
			
			info.name = name; // defaults for unknown devices
			info.ram = 1024; // defaults
			info.max_texture_size = 0;
			
			if (name.starts_with("iPad"))
			{
				if (name.starts_with("iPad1"))
				{
					info.name = "iPad1";
					info.ram = 256;
				}
				else if (name.starts_with("iPad2"))
				{
					info.name = "iPad2";
					info.ram = 512;
				}
				else if (name.starts_with("iPad3"))
				{
					info.name = "iPad3";
					info.ram = 1024;
				}
			}
			else if (name.starts_with("iPhone"))
			{
				if (name == "iPhone1,1")
				{
					info.name = "iPhone2G";
					info.ram = 128;
				}
				else if (name == "iPhone1,2")
				{
					info.name = "iPhone3G";
					info.ram = 128;
				}
				else if (name == "iPhone2,1")
				{
					info.name = "iPhone3GS";
					info.ram = 256;
				}
				else if (name.starts_with("iPhone3"))
				{
					info.name = "iPhone4";
					info.ram = 512;
				}
				else if (name.starts_with("iPhone4"))
				{
					info.name = "iPhone4S";
					info.ram = 512;
				}
				else if (name.starts_with("iPhone5"))
				{
					info.name = "iPhone5";
					info.ram = 1024;
				}
			}
			else if (name.starts_with("iPod"))
			{
				if (name == "iPod1,1")
				{
					info.name = "iPod1";
					info.ram = 128;
				}
				else if (name == "iPod2,1")
				{
					info.name = "iPod2";
					info.ram = 128;
				}
				else if (name == "iPod3,1")
				{
					info.name = "iPod3";
					info.ram = 256;
				}
				else if (name == "iPod4,1")
				{
					info.name = "iPod4";
					info.ram = 256;
				}
			}
			//else: i386 (iphone simulator) and possible future device types
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
		if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone)
		{
			return DEVICE_IPHONE;
		}
		return DEVICE_IPAD;
	}
	
	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
        NSString *buttons[] = {@"OK", nil, nil}; // set all buttons to nil, at first, except default one, just in case
		MessageBoxButton buttonTypes[] = {AMSGBTN_OK, AMSGBTN_NULL, AMSGBTN_NULL};
        
		if (buttonMask & AMSGBTN_OK && buttonMask & AMSGBTN_CANCEL)
		{
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_OK, "OK").c_str()];
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            
            buttonTypes[1] = AMSGBTN_OK;
            buttonTypes[0] = AMSGBTN_CANCEL;
        }
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO && buttonMask & AMSGBTN_CANCEL)
		{
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
			buttons[2] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            
            buttonTypes[1] = AMSGBTN_YES;
            buttonTypes[2] = AMSGBTN_NO;
            buttonTypes[0] = AMSGBTN_CANCEL;
		}
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO)
		{
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
            
            buttonTypes[1] = AMSGBTN_YES;
            buttonTypes[0] = AMSGBTN_NO;
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

        AprilMessageBoxDelegate *mbd = [[[AprilMessageBoxDelegate alloc] initWithModality:(style & AMSGSTYLE_MODAL)] autorelease];
        mbd.callback = callback;
        mbd.buttonTypes = buttonTypes;
		[mbd retain];

		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:titlens
														message:textns
													   delegate:mbd 
											  cancelButtonTitle:buttons[0]
											  otherButtonTitles:buttons[1], buttons[2], nil];
		[alert show];
		if (style & AMSGSTYLE_MODAL) 
		{
			CFRunLoopRun();
		}
		[alert release];
		
		// We're modal?
		// If so, we know what to return!
		if (style & AMSGSTYLE_MODAL)
		{
			return mbd.selectedButton;
		}
		
		// NOTE: does not return proper values unless modal! 
		//       you need to implement a delegate.
		
		
		// some dummy returnvalues
		if (buttonMask & AMSGBTN_CANCEL)
		{
			return AMSGBTN_CANCEL;
		}
		if (buttonMask & AMSGBTN_OK)
		{
			return AMSGBTN_OK;
		}
		if (buttonMask & AMSGBTN_NO)
		{
			return AMSGBTN_NO;
		}
		if (buttonMask & AMSGBTN_YES)
		{
			return AMSGBTN_YES;
		}
		return AMSGBTN_OK;
	}

}
#endif
