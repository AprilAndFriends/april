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
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#import <CoreGraphics/CoreGraphics.h>
#import <mach/mach.h>

#import <OpenGLES/ES1/gl.h>

#include "april.h"
#include "RenderSystem.h"
#include "Image.h"
#include "iOS_Window.h"
#include "Platform.h"
#include "april.h"

void getStaticiOSInfo(chstr name, april::SystemInfo& info);

@interface AprilMessageBoxDelegate : NSObject<UIAlertViewDelegate> {
	void(*callback)(april::MessageBoxButton);
	april::MessageBoxButton buttonTypes[3];
	
	CFRunLoopRef runLoop;
	BOOL isModal;
	april::MessageBoxButton selectedButton;
}
@property (nonatomic, assign) void(*callback)(april::MessageBoxButton);
@property (nonatomic, assign) april::MessageBoxButton *buttonTypes;
@property (nonatomic, readonly) april::MessageBoxButton selectedButton;
@end

@implementation AprilMessageBoxDelegate

@synthesize callback;
@synthesize selectedButton;
@dynamic buttonTypes;

-(id)initWithModality:(BOOL)_isModal
{
	self = [super init];
	if (self)
	{
		runLoop = CFRunLoopGetCurrent();
		isModal = _isModal;
	}
	return self;
}

-(april::MessageBoxButton*)buttonTypes
{
	return buttonTypes;
}

-(void)setButtonTypes:(april::MessageBoxButton*)_buttonTypes
{
	buttonTypes[0] = _buttonTypes[0];
	buttonTypes[1] = _buttonTypes[1];
	buttonTypes[2] = _buttonTypes[2];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if (callback)
	{
		callback(buttonTypes[buttonIndex]);
	}
	if (isModal)
	{
		CFRunLoopStop(runLoop);
	}
	
	selectedButton = buttonTypes[buttonIndex];
	
	[self release];
}

- (void)willPresentAlertView:(UIAlertView*)alertView
{
	NSString *reqSysVer = @"4.0";
	NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
	BOOL isFourOh = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
	
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone && buttonTypes[2].value != 0 && isFourOh)
	{
		// landscape sucks on 4.0+ phones when we have three buttons.
		// it doesnt show hint message.
		// unless we hack.
		
		float w = alertView.bounds.size.width;
		if (w < 5.0f)
		{
			hlog::write(april::logTag, "In messageBox()'s label hack, width override took place");
			w = 400.0f; // hardcoded width! seems to work ok
		}
		
		UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(0.0f, 30.0f, alertView.bounds.size.width, 40.0f)]; 
		label.backgroundColor = [UIColor clearColor]; 
		label.textColor = [UIColor whiteColor]; 
		label.font = [UIFont systemFontOfSize:14.0f]; 
		label.textAlignment = NSTextAlignmentCenter;
		label.text = alertView.message; 
		[alertView addSubview:label]; 
		[label release];
	}
	
}

@end

namespace april
{
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
			
			info.deviceName = [[[UIDevice currentDevice] name] UTF8String];
			info.displayDpi = 0;

			UIScreen* mainScreen = [UIScreen mainScreen];
			float scale = [mainScreen scale];
			float nativeScale;
			if ([mainScreen respondsToSelector:@selector(nativeScale:)])
			{
				nativeScale = [mainScreen nativeScale];
			}
			else // older than iOS8
			{
				nativeScale = scale;
			}
			
			
			int w = mainScreen.bounds.size.width * scale;
			int h = mainScreen.bounds.size.height * scale;
			hlog::writef(logTag, "iOS screen dimensions: %.0f x %.0f points, %d x %d pixes, scale: %.2f, nativeScale: %.2f", mainScreen.bounds.size.width, mainScreen.bounds.size.height, w, h, scale, nativeScale);
			// forcing a w:h ratio where w > h
			info.displayResolution.set((float)hmax(w, h), (float)hmin(w, h));
			info.osVersion.set(hstr::fromUnicode([[UIDevice currentDevice].systemVersion UTF8String]));

			getStaticiOSInfo(name, info);
//			Probably not needed, in order to report correct cores on simulator using static info
//			-- commented out by kspes@20151221, remove completely if it didn't affect anything after a while
//			int systemCores = (int)sysconf(_SC_NPROCESSORS_ONLN);
//			if (systemCores > info.cpuCores)
//			{
//				info.cpuCores = systemCores;
//			}
		}
	}

	hstr _getPackageName_platform()
	{
		static hstr bundleId;
		if (bundleId == "")
		{
			NSString *bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
			bundleId = [bundleIdentifier UTF8String];
		}
		return bundleId;
	}

	hstr _getUserDataPath_platform()
	{
		hlog::warn(logTag, "Cannot use getUserDataPath() on this platform.");
		return ".";
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
	
	bool _openUrl_platform(chstr url)
	{
		if ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.cStr()]]])
		{
			[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.cStr()]]];
			return true;
		}
		return false;
	}
	
	void _showMessageBox_platform(chstr title, chstr text, MessageBoxButton buttons, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton), bool modal)
	{
		NSString *nsButtons[] = {@"OK", nil, nil}; // set all buttons to nil, at first, except default one, just in case
		MessageBoxButton buttonTypes[3] = {MessageBoxButton::Ok, MessageBoxButton::Ok, MessageBoxButton::Ok};
		
		int i0 = 0, i1 = 1, i2 = 2;
//		if (NSFoundationVersionNumber >= NSFoundationVersionNumber_iOS_7_0)
//		{
//			i0 = 1, i1 = 0; // we want to bold the "OK" button, but in ios7 and up, the cancel button is bolded by default
//		}

		if (buttons == MessageBoxButton::OkCancel)
		{
			nsButtons[i1] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Ok, "OK").cStr()];
			nsButtons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i1] = MessageBoxButton::Ok;
			buttonTypes[i0] = MessageBoxButton::Cancel;
		}
		else if (buttons == MessageBoxButton::YesNoCancel)
		{
			nsButtons[i1] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			nsButtons[i2] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			nsButtons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i1] = MessageBoxButton::Yes;
			buttonTypes[i2] = MessageBoxButton::No;
			buttonTypes[i0] = MessageBoxButton::Cancel;
		}
		else if (buttons == MessageBoxButton::YesNo)
		{
			nsButtons[i1] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			nsButtons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			buttonTypes[i1] = MessageBoxButton::Yes;
			buttonTypes[i0] = MessageBoxButton::No;
		}
		else if (buttons == MessageBoxButton::Cancel)
		{
			nsButtons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel").cStr()];
			buttonTypes[i0] = MessageBoxButton::Cancel;
		}
		else if (buttons == MessageBoxButton::Ok)
		{
			nsButtons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Ok, "OK").cStr()];
			buttonTypes[i0] = MessageBoxButton::Ok;
		}
		else if (buttons == MessageBoxButton::Yes)
		{
			nsButtons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes").cStr()];
			buttonTypes[i0] = MessageBoxButton::Yes;
		}
		else if (buttons == MessageBoxButton::No)
		{
			nsButtons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MessageBoxButton::No, "No").cStr()];
			buttonTypes[i0] = MessageBoxButton::No;
		}
		
		NSString *titlens = [NSString stringWithUTF8String:title.cStr()];
		NSString *textns = [NSString stringWithUTF8String:text.cStr()];

		AprilMessageBoxDelegate *mbd = [[[AprilMessageBoxDelegate alloc] initWithModality:modal] autorelease];
		mbd.callback = callback;
		[mbd setButtonTypes:buttonTypes];
		[mbd retain];

		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:titlens
														message:textns
													   delegate:mbd 
											  cancelButtonTitle:nsButtons[i0]
											  otherButtonTitles:nsButtons[i1], nsButtons[i2], nil];
		if (alert != nil) // just in case, hapens in some very weird situations..
		{
			[alert show];
			if (modal)
			{
				CFRunLoopRun();
			}
			[alert release];
		}
		else
		{
			hlog::error(logTag, "Failed to display AlertView!");
		}
	}
	
}
#endif
