/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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
#include "PVRTexture.h"

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
		if(w < 5.)
		{
			hlog::write(april::logTag, "In messageBox()'s label hack, width override took place");
			w = 400; // hardcoded width! seems to work ok
			
		}
		
		
		UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(0.0f, 30.0f, alertView.bounds.size.width, 40.0f)]; 
		label.backgroundColor = [UIColor clearColor]; 
		label.textColor = [UIColor whiteColor]; 
		label.font = [UIFont systemFontOfSize:14.0f]; 
		label.textAlignment = UITextAlignmentCenter;
		label.text = alertView.message; 
		[alertView addSubview:label]; 
		[label release];
	}
	
}
@end


namespace april
{
	SystemInfo getSystemInfo()
	{
		static SystemInfo info;
		info.cpuCores = sysconf(_SC_NPROCESSORS_ONLN);
		if (info.locale == "")
		{
			info.locale = [[[NSLocale preferredLanguages] objectAtIndex:0] UTF8String];
			
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

			getStaticiOSInfo(name, info);
		}
		// iPhone simulator
		if (info.maxTextureSize == 0 && april::rendersys != NULL)
		{
			info.maxTextureSize = april::rendersys->getMaxTextureSize();

			int h = info.displayResolution.y;
			if (h == 320) // iPhone3GS
				info.displayDpi = 163;
			else if (h == 768) // iPad 1/2
				info.displayDpi = 132;
			else if (h == 640) // iPhone4+
				info.displayDpi = 326;
			else if (h == 1536) // iPad3+
				info.displayDpi = 256;
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
		hlog::warn(april::logTag, "Cannot use getUserDataPath() on this platform.");
		return ".";
	}
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
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
	}
	
	NSURL* _getFileURL(chstr filename)
	{
		
		// consider that "filename" is "../media/hello.jpg" and "appname" is
		// installed in "/Applications/appname/bin/". then:
		
		// bundle: file:///Applications/appname/bin/appname.app/
		// file:  file:///Applications/appname/bin/appname.app/../../media/hello.jpg
		// url: file:///Applications/appname/media/hello.jpg
		
#if defined(__MAC_10_6)
		
		NSString* cdp = [[[[NSFileManager alloc] init] autorelease] currentDirectoryPath];
		NSString* file = [NSString stringWithFormat:@"%@/%s", cdp, filename.c_str()];
		NSURL* url = [NSURL fileURLWithPath:file];
		url = [url absoluteURL];
		
		[cdp release];
		[file release];
#else
		// FIXME use NSURL fileURLWithPath:
		NSString * bundle = [[[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/"] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
		NSURL * bundleURL = [NSURL URLWithString:[@"file://" stringByAppendingString:bundle]];
		NSURL * file = [NSURL URLWithString:[NSString stringWithFormat:@"../%s", filename.c_str()]
							  relativeToURL:bundleURL];
		NSURL * url = [file absoluteURL];
		
#endif
		//NSLog(@"_getFileURL: %@", url);
		
		return url;
		
	}
}
#endif
