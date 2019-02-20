/// @file
/// @version 5.0
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
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Application.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Mac_CocoaWindow.h"
#include "Mac_Window.h"

#define MAC_WINDOW ((april::Mac_Window*)april::window)

namespace april
{
	extern SystemInfo info;
	
	static hversion _getMaxOsVersion()
	{
#ifdef _DEBUG
		//return hversion(10, 6); // uncomment this to test behaviour on older macs
#endif
		hversion result;
		SInt32 major, minor, bugfix;
		if (Gestalt(gestaltSystemVersionMajor, &major) == noErr &&
			Gestalt(gestaltSystemVersionMinor, &minor) == noErr &&
			Gestalt(gestaltSystemVersionBugFix, &bugfix) == noErr)
		{
			result.set(major, minor, bugfix);
		}
		else
		{
			result.set(10, 4); // just in case, < 10.4 is not supported.
		}
		return result;
	}
	
	void _setupSystemInfo_platform(SystemInfo& info)
	{
		static NSScreen* prevScreen = NULL;
		NSScreen* mainScreen = april::macCocoaWindow.screen;
		if (mainScreen == nil)
		{
			mainScreen = [NSScreen mainScreen];
		}
		if (prevScreen != mainScreen)
		{
			prevScreen = mainScreen;
			// CPU cores
			info.cpuCores = (int)sysconf(_SC_NPROCESSORS_ONLN);
			// RAM
			info.name = "mac";
			info.osType = SystemInfo::OsType::Mac;
			info.deviceName = "unnamedMacDevice";
			info.osVersion = _getMaxOsVersion();
			float scalingFactor = 1.0f;
			if ([mainScreen respondsToSelector:@selector(backingScaleFactor)])
			{
				scalingFactor = mainScreen.backingScaleFactor;
			}
			int mib [] = { CTL_HW, HW_MEMSIZE };
			int64_t value = 0;
			size_t length = sizeof(value);
			if (sysctl(mib, 2, &value, &length, NULL, 0) == -1)
			{
				info.ram = 2048;
			}
			else
			{
				info.ram = (int)(value / (1024 * 1024));
			}
			// display resolution
			NSRect rect = [mainScreen frame];
			info.displayResolution.set((float)rect.size.width * scalingFactor, (float)rect.size.height * scalingFactor);
			// display DPI
			CGSize screenSize = CGDisplayScreenSize(CGMainDisplayID());
			info.displayScaleFactor = mainScreen.backingScaleFactor;
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
			CFRelease(lang);
			CFRelease(locs);
		}
	}
	
	hstr _getPackageName_platform()
	{
		static hstr bundleID;
		if (bundleID == "")
		{
			NSString* bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
			bundleID = [bundleIdentifier UTF8String];
		}
		return bundleID;
	}

	hstr _getUserDataPath_platform()
	{
		hstr cwd = hdir::cwd();
		hstr path;
		NSSearchPathDirectory destDir = NSApplicationSupportDirectory;
		NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init]; 
		CFArrayRef destDirArr = (CFArrayRef)NSSearchPathForDirectoriesInDomains(destDir, NSUserDomainMask, YES);
		CFStringRef destDirPath = (CFStringRef)CFArrayGetValueAtIndex(destDirArr, 0);
		char* cpath_alloc = NULL;
		int buffersize = (int)CFStringGetMaximumSizeOfFileSystemRepresentation(destDirPath) + 1;
		cpath_alloc = (char*)malloc(buffersize);
		CFStringGetFileSystemRepresentation(destDirPath, cpath_alloc, buffersize);
		path = cpath_alloc;
		if (cpath_alloc != NULL)
		{
			free(cpath_alloc);
		}
		[arp release];
		// sandboxed app should use bundle ID
		NSDictionary* environ = [[NSProcessInfo processInfo] environment];
		BOOL appInSandbox = ([environ objectForKey:@"APP_SANDBOX_CONTAINER_ID"] != nil);
		hstr bundleID = [[[NSBundle mainBundle] bundleIdentifier] UTF8String];
		hstr gameName = "TEMP";
		if (appInSandbox)
		{
			path += "/" + bundleID;
		}
		hdir::chdir(cwd); // safe is safe
		return path;
	}
	
	int64_t _getRamConsumption_platform()
	{
		// TODOa
		hlog::warn(logTag, "Cannot use getRamConsumption() on this platform.");
		return 0LL;
	}	
	
	void _getNotchOffsets_platform(gvec2i& topLeft, gvec2i& bottomRight, bool landscape)
	{
		topLeft.set(0, 0);
		bottomRight.set(0, 0);
	}

	bool _openUrl_platform(chstr url)
	{
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.cStr()]]];
		return true;
	}
	
	void _showMessageBox_platform(const MessageBoxData& data)
	{
		// fugly implementation of showing messagebox on mac os
		// ideas:
		// * display as a sheet attached on top of the current window
		// * prioritize buttons and automatically assign slots
		// * use constants for button captions
		// * use an array with constants for button captions etc
		NSString *nsButtons[] = {@"OK", nil, nil}; // set all buttons to nil, at first, except default one, just in case
		MessageBoxButton buttonTypes[3] = {MessageBoxButton::Ok, MessageBoxButton::Ok, MessageBoxButton::Ok};
		int i0 = 0;
		int i1 = 1;
		int i2 = 2;
		if (data.buttons == MessageBoxButton::OkCancel)
		{
			nsButtons[i1] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Ok, "OK").cStr()];
			nsButtons[i0] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i1] = MessageBoxButton::Ok;
			buttonTypes[i0] = MessageBoxButton::Cancel;
		}
		else if (data.buttons == MessageBoxButton::YesNoCancel)
		{
			nsButtons[i1] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			nsButtons[i2] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			nsButtons[i0] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i1] = MessageBoxButton::Yes;
			buttonTypes[i2] = MessageBoxButton::No;
			buttonTypes[i0] = MessageBoxButton::Cancel;
		}
		else if (data.buttons == MessageBoxButton::YesNo)
		{
			nsButtons[i1] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			nsButtons[i0] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			buttonTypes[i1] = MessageBoxButton::Yes;
			buttonTypes[i0] = MessageBoxButton::No;
		}
		else if (data.buttons == MessageBoxButton::Cancel)
		{
			nsButtons[i0] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i0] = MessageBoxButton::Cancel;
		}
		else if (data.buttons == MessageBoxButton::Ok)
		{
			nsButtons[i0] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Ok, "OK").cStr()];
			buttonTypes[i0] = MessageBoxButton::Ok;
		}
		else if (data.buttons == MessageBoxButton::Yes)
		{
			nsButtons[i0] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			buttonTypes[i0] = MessageBoxButton::Yes;
		}
		else if (data.buttons == MessageBoxButton::No)
		{
			nsButtons[i0] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			buttonTypes[i0] = MessageBoxButton::No;
		}
		harray<hstr> argButtons;
		harray<MessageBoxButton> argButtonTypes;
		argButtons += [nsButtons[i0] UTF8String];
		argButtons += (nsButtons[i1] == 0 ? "" : [nsButtons[i1] UTF8String]);
		argButtons += (nsButtons[i2] == 0 ? "" : [nsButtons[i2] UTF8String]);
		argButtonTypes += buttonTypes[i0];
		argButtonTypes += buttonTypes[i1];
		argButtonTypes += buttonTypes[i2];
#define ns(s) [NSString stringWithUTF8String:s.cStr()]
		[AprilCocoaWindow showAlertView:ns(data.title) button1:ns(argButtons[0]) button2:ns(argButtons[1]) button3:ns(argButtons[2]) btn1_t:argButtonTypes[0] btn2_t:argButtonTypes[1] btn3_t:argButtonTypes[2] text:ns(data.text) callback:april::Application::messageBoxCallback];
	}
	
}
#endif
