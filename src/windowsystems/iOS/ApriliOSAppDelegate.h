/// @file
/// @version 4.2
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

- (BOOL)application:(UIApplication*) application didFinishLaunchingWithOptions:(NSDictionary*) launchOptions;
- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation;

@property (nonatomic, retain) UIWindow *uiwnd;
@property (nonatomic, retain) AprilViewController *viewController;
@property (nonatomic, retain) NSDictionary *appLaunchOptions;
@end
