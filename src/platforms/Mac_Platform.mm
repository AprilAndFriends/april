/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Mac_Window.h"

namespace april
{
	extern SystemInfo info;
	static hversion osVersion;
	
	hversion getMacOSVersion()
	{
#ifdef _DEBUG
		//return hversion(10, 6); // uncomment this to test behaviour on older macs
#endif
		if (osVersion.major == 0)
		{
			SInt32 major, minor, bugfix;
			if (Gestalt(gestaltSystemVersionMajor, &major)   == noErr &&
				Gestalt(gestaltSystemVersionMinor, &minor)   == noErr &&
				Gestalt(gestaltSystemVersionBugFix, &bugfix) == noErr)
			{
				osVersion.set(major, minor, bugfix);
			}
			else
			{
				osVersion.set(10, 3); // just in case. < 10.4 is not supported.
			}
		}
		return osVersion;
	}
	
	SystemInfo getSystemInfo()
	{
		static NSScreen* prevScreen = NULL;
		NSScreen* mainScreen = [NSScreen mainScreen];
		if (prevScreen != mainScreen)
		{
			prevScreen = mainScreen;
			// CPU cores
			info.cpuCores = (int)sysconf(_SC_NPROCESSORS_ONLN);
			// RAM
			info.name = "mac";
			info.deviceName = "unnamedMacDevice";
			info.osVersion = getMacOSVersion();
			
			float scalingFactor = 1.0f;
			if ([mainScreen respondsToSelector:@selector(backingScaleFactor)])
			{
				scalingFactor = [NSScreen mainScreen].backingScaleFactor;
			}

			int mib [] = { CTL_HW, HW_MEMSIZE };
			int64_t value = 0;
			size_t length = sizeof(value);

			if (sysctl(mib, 2, &value, &length, NULL, 0) == -1)
				info.ram = 2048;
			else
				info.ram = (int)(value / (1024 * 1024));

			// display resolution
			NSRect rect = [mainScreen frame];
			info.displayResolution.set((float)rect.size.width * scalingFactor, (float)rect.size.height * scalingFactor);
			// display DPI
			CGSize screenSize = CGDisplayScreenSize(CGMainDisplayID());
			info.displayDpi = 25.4f * info.displayResolution.y / screenSize.height;

			// locale
			// This code gets the prefered locale based on user's list of prefered languages against the supported languages
			// in the app bundle (the .lproj folders in the bundle)
			CFArrayRef locs = CFBundleCopyBundleLocalizations(CFBundleGetMainBundle());
			CFArrayRef preferred = CFBundleCopyPreferredLocalizationsFromArray(locs);
			CFStringRef loc = (CFStringRef) CFArrayGetValueAtIndex(preferred, 0);
			CFStringRef lang = CFLocaleCreateCanonicalLocaleIdentifierFromString(NULL, loc);
			CFRelease(preferred);
			char cstr[64 + 1];
			CFStringGetCString(lang, cstr, 64, kCFStringEncodingASCII);
			hstr fullLocale = hstr(cstr);
			info.locale = "en"; // default is "en"
			if (fullLocale.contains("-"))
			{
				fullLocale.split("-", info.locale, info.localeVariant);
			}
			else if (fullLocale.contains("_"))
			{
				fullLocale.split("_", info.locale, info.localeVariant);
			}
			else
			{
				info.locale = fullLocale;
			}
			info.locale = info.locale.lowered();
			info.localeVariant = info.localeVariant.uppered();
		}
		return info;
	}
	
	hstr getPackageName()
	{
		static hstr bundleID;
		if (bundleID == "")
		{
			NSString *bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
			bundleID = [bundleIdentifier UTF8String];
		}
		return bundleID;
	}

	hstr getUserDataPath()
	{
		hlog::warn(logTag, "Cannot use getUserDataPath() on this platform.");
		return ".";
	}
	
	int64_t getRamConsumption()
	{
		// TODOa
		return 0LL;
	}	
	
	bool openUrl(chstr url)
	{
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.cStr()]]];
		return true;
	}
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttons, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton), bool modal)
	{
		// fugly implementation of showing messagebox on mac os
		// ideas:
		// * display as a sheet attached on top of the current window
		// * prioritize buttons and automatically assign slots
		// * use constants for button captions
		// * use an array with constants for button captions etc
		
		NSString *buttons[] = {@"OK", nil, nil}; // set all buttons to nil, at first, except default one, just in case
		MessageBoxButton buttonTypes[3] = {1, 0, 0};
		int i0 = 0, i1 = 1, i2 = 2;
		
		if (buttons == MessageBoxButton::OkCancel)
		{
			buttons[i1] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Ok, "OK").cStr()];
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i1] = 1;
			buttonTypes[i0] = 2;
		}
		else if (buttons == MessageBoxButton::YesNoCancel)
		{
			buttons[i1] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			buttons[i2] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i1] = 4;
			buttonTypes[i2] = 8;
			buttonTypes[i0] = 2;
		}
		else if (buttons == MessageBoxButton::YesNo)
		{
			buttons[i1] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			buttonTypes[i1] = 4;
			buttonTypes[i0] = 8;
		}
		else if (buttons == MessageBoxButton::Cancel)
		{
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i0] = 2;
		}
		else if (buttons == MessageBoxButton::Ok)
		{
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Ok, "OK").cStr()];
			buttonTypes[i0] = 1;
		}
		else if (buttons == MessageBoxButton::Yes)
		{
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			buttonTypes[i0] = 4;
		}
		else if (buttons == MessageBoxButton::No)
		{
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			buttonTypes[i0] = 8;
		}
		
        harray<hstr> argButtons;
        harray<MessageBoxButton> argButtonTypes;
        argButtons += [buttons[i0] UTF8String];
        argButtons += buttons[i1] == 0 ? "" : [buttons[i1] UTF8String];
        argButtons += buttons[i2] == 0 ? "" : [buttons[i2] UTF8String];
        argButtonTypes += buttonTypes[i0];
        argButtonTypes += buttonTypes[i1];
        argButtonTypes += buttonTypes[i2];
		if (aprilWindow == NULL)
		{
			printf("ERROR: %s\n", text.cStr());
			exit(1);
		}
        aprilWindow->queueMessageBox(title, argButtons, argButtonTypes, text, callback);
	}
	
}
#endif
