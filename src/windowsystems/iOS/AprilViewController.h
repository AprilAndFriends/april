/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines an april view controller.

#ifdef _IOS_WINDOW
#import <UIKit/UIKit.h>

@interface AprilViewController : UIViewController
{
}
- (void)removeImageView:(bool)fast;
@end
#endif
