/// @file
/// @version 3.7
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
	if(self)
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
	memcpy(buttonTypes, _buttonTypes, sizeof(april::MessageBoxButton)*3);
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
	
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone && buttonTypes[2] && isFourOh) 
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
	extern SystemInfo info;
	
	SystemInfo getSystemInfo()
	{
		info.cpuCores = (int)sysconf(_SC_NPROCESSORS_ONLN);
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
			info.displayDpi = 0;

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
			info.displayResolution.set((float)hmax(w, h), (float)hmin(w, h));
			
			info.osVersion = [[UIDevice currentDevice].systemVersion floatValue];

			getStaticiOSInfo(name, info);
		}
		return info;
	}

	hstr getPackageName()
	{
		static hstr bundleId;
		if (bundleId == "")
		{
			NSString *bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
			bundleId = [bundleIdentifier UTF8String];
		}
		return bundleId;
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
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		NSString *buttons[] = {@"OK", nil, nil}; // set all buttons to nil, at first, except default one, just in case
		MessageBoxButton buttonTypes[] = {MESSAGE_BUTTON_OK, (MessageBoxButton)NULL, (MessageBoxButton)NULL};
		
		int i0 = 1, i1 = 0, i2 = 2;
		if (NSFoundationVersionNumber < NSFoundationVersionNumber_iOS_7_0)
		{
			i0 = 0, i1 = 1; // we want to bold the "OK" button, but in ios7 and up, the cancel button is bolded by default
		}
		if ((buttonMask & MESSAGE_BUTTON_OK) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			buttons[i1] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_OK, "OK").cStr()];
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_CANCEL, "Cancel").cStr()];
			buttonTypes[i1] = MESSAGE_BUTTON_OK;
			buttonTypes[i0] = MESSAGE_BUTTON_CANCEL;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			buttons[i1] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_YES, "Yes").cStr()];
			buttons[i2] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_NO, "No").cStr()];
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_CANCEL, "Cancel").cStr()];
			buttonTypes[i1] = MESSAGE_BUTTON_YES;
			buttonTypes[i2] = MESSAGE_BUTTON_NO;
			buttonTypes[i0] = MESSAGE_BUTTON_CANCEL;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO))
		{
			buttons[i1] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_YES, "Yes").cStr()];
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_NO, "No").cStr()];
			buttonTypes[i1] = MESSAGE_BUTTON_YES;
			buttonTypes[i0] = MESSAGE_BUTTON_NO;
		}
		else if (buttonMask & MESSAGE_BUTTON_CANCEL)
		{
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_CANCEL, "Cancel").cStr()];
			buttonTypes[i0] = MESSAGE_BUTTON_CANCEL;
		}
		else if (buttonMask & MESSAGE_BUTTON_OK)
		{
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_OK, "OK").cStr()];
			buttonTypes[i0] = MESSAGE_BUTTON_OK;
		}
		else if (buttonMask & MESSAGE_BUTTON_YES)
		{
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_YES, "Yes").cStr()];
			buttonTypes[i0] = MESSAGE_BUTTON_YES;
		}
		else if (buttonMask & MESSAGE_BUTTON_NO)
		{
			buttons[i0] = [NSString stringWithUTF8String:customButtonTitles.tryGet(MESSAGE_BUTTON_NO, "No").cStr()];
			buttonTypes[i0] = MESSAGE_BUTTON_NO;
		}
		
		NSString *titlens = [NSString stringWithUTF8String:title.cStr()];
		NSString *textns = [NSString stringWithUTF8String:text.cStr()];

		AprilMessageBoxDelegate *mbd = [[[AprilMessageBoxDelegate alloc] initWithModality:(style & MESSAGE_STYLE_MODAL)] autorelease];
		mbd.callback = callback;
		mbd.buttonTypes = buttonTypes;
		[mbd retain];

		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:titlens
														message:textns
													   delegate:mbd 
											  cancelButtonTitle:buttons[0]
											  otherButtonTitles:buttons[1], buttons[2], nil];
		if (alert != nil) // just in case, hapens in some very weird situations..
		{
			[alert show];
			if (style & MESSAGE_STYLE_MODAL) 
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
