/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_IPHONE
#include <sys/sysctl.h>
#import <UIKit/UIKit.h>
#include <gtypes/Vector2.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#import <CoreGraphics/CoreGraphics.h>
#import <mach/mach.h>

#import <OpenGLES/ES1/gl.h>

#include "Application.h"
#include "april.h"
#include "Image.h"
#include "iOS_Window.h"
#include "Platform.h"
#include "RenderSystem.h"

void getStaticiOSInfo(chstr name, april::SystemInfo& info);

namespace april
{
	extern SystemInfo info;
	static UIEdgeInsets _insets;
	
	void _setupSystemInfo_platform(SystemInfo& info)
	{
		if (info.locale == "")
		{
			info.locale = "en"; // default is "en"
			NSBundle* bundle = [NSBundle mainBundle];
			NSArray* langs = [bundle preferredLocalizations];
			langs = [langs count] ? langs : [NSLocale preferredLanguages];
			NSString* locale = [langs objectAtIndex:0];
			hstr fullLocale = [[NSLocale canonicalLanguageIdentifierFromString:locale] UTF8String];
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
			size_t size = 255;
			char cname[256] = {'\0'};
			sysctlbyname("hw.machine", cname, &size, NULL, 0);
			hstr name = cname;
			info.name = name; // defaults for unknown devices
			info.osType = SystemInfo::OsType::iOS;
			info.deviceName = [[UIDevice currentDevice].name UTF8String];
			hstr model = [[UIDevice currentDevice].model UTF8String];
			info.displayDpi = 0;
			UIScreen* mainScreen = [UIScreen mainScreen];
			info.displayScaleFactor = [mainScreen scale];
			int w = mainScreen.nativeBounds.size.width;
			int h = mainScreen.nativeBounds.size.height;
			hlog::writef(logTag, "iOS screen dimensions: %.0fx%.0f points, %dx%d pixels, scale: %.2f, nativeScale: %.2f", mainScreen.bounds.size.width, mainScreen.bounds.size.height, w, h, info.displayScaleFactor, [mainScreen nativeScale]);
			// forcing a w:h ratio where w > h
			info.displayResolution.set(hmax(w, h), hmin(w, h));
			info.osVersion.set(hstr::fromUnicode([[UIDevice currentDevice].systemVersion UTF8String]));
			NSProcessInfo* processInfo = [NSProcessInfo processInfo];
			info.cpuCores = (int)[processInfo processorCount];
			info.ram = (int)([processInfo physicalMemory] / 1024 / 1024);
			getStaticiOSInfo(name, info);
			// making sure it's only called from the main thread
			if (@available(iOS 11.0, *))
			{
				dispatch_async(dispatch_get_main_queue(),
				^{
					_insets = [UIApplication sharedApplication].delegate.window.safeAreaInsets;
				});
			}
		}
	}

	hstr _getPackageName_platform()
	{
		static hstr bundleId;
		if (bundleId == "")
		{
			bundleId = [[[NSBundle mainBundle] bundleIdentifier] UTF8String];
		}
		return bundleId;
	}

	hstr _getUserDataPath_platform()
	{
		hstr cwd = hdir::cwd();
		hstr path;
		NSSearchPathDirectory destDir;
		destDir = NSDocumentDirectory;
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
		hdir::chdir(cwd); // safe is safe
		return path;
	}
	
	int64_t _getRamConsumption_platform()
	{
		struct task_basic_info info;
		mach_msg_type_number_t size = sizeof(info);
		kern_return_t result = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size);
		if (result == KERN_SUCCESS)
		{
			return (int64_t)info.resident_size;
		}
		hlog::error(logTag, "getRamConsumption() failed, task_info() failed!");
		return 0LL;
	}	
	
	void _getNotchOffsets_platform(gvec2i& topLeft, gvec2i& bottomRight, bool landscape)
	{
		topLeft.set(0, 0);
		bottomRight.set(0, 0);
		if (@available(iOS 11.0, *))
		{
			topLeft.set((int)(_insets.left * info.displayScaleFactor), (int)(_insets.top * info.displayScaleFactor));
			bottomRight.set((int)(_insets.right * info.displayScaleFactor), (int)(_insets.bottom * info.displayScaleFactor));
		}
	}

	bool _openUrl_platform(chstr url)
	{
		NSURL* nsUrl = [NSURL URLWithString:[NSString stringWithUTF8String:url.cStr()]];
		dispatch_async(dispatch_get_main_queue(),
		^{
			if ([[UIApplication sharedApplication] canOpenURL:nsUrl])
			{
				[[UIApplication sharedApplication] openURL:nsUrl];
			}
		});
		return true; // assumes always success, because it can't get the result otherwise
	}
	
	void _showMessageBox_platform(const MessageBoxData& data)
	{
		NSString* nsButtons[] = {@"OK", nil, nil}; // set all buttons to nil at first, except default one, just in case
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
			nsButtons[i0] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			nsButtons[i1] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			nsButtons[i2] = [NSString stringWithUTF8String:data.customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i0] = MessageBoxButton::Yes;
			buttonTypes[i1] = MessageBoxButton::No;
			buttonTypes[i2] = MessageBoxButton::Cancel;
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
		NSString* nsTitle = [NSString stringWithUTF8String:data.title.cStr()];
		NSString* nsText = [NSString stringWithUTF8String:data.text.cStr()];
		UIAlertController* alert = [UIAlertController alertControllerWithTitle:nsTitle message:nsText preferredStyle:UIAlertControllerStyleAlert];
		UIAlertAction* action = NULL;
		for_iter (i, 0, 3)
		{
			if (nsButtons[i] != nil)
			{
				MessageBoxButton buttonType = buttonTypes[i];
				action = [UIAlertAction actionWithTitle:nsButtons[i] style:UIAlertActionStyleDefault handler:^(UIAlertAction* action)
				{
					april::Application::messageBoxCallback(buttonType);
				}];
				[alert addAction:action];
			}
		}
		UIViewController* viewController = [[[UIApplication sharedApplication] delegate] window].rootViewController;
		[viewController presentViewController:alert animated:YES completion:nil];
	}
}
#endif
