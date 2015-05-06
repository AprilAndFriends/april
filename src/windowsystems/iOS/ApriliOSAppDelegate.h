/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines an iOS app delegate.

#import <UIKit/UIKit.h>
@class AprilViewController;

@interface ApriliOSAppDelegate : NSObject<UIApplicationDelegate>
{
	UIWindow *uiwnd;
	AprilViewController *viewController;
}

@property (nonatomic, retain) UIWindow *uiwnd;
@property (nonatomic, retain) AprilViewController *viewController;
@property (nonatomic, retain) NSDictionary *appLaunchOptions;
@end
