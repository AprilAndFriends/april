/************************************************************************************\
 This source file is part of the Awesome Portable Rendering Interface Library         *
 For latest info, see http://libapril.sourceforge.net/                                *
 **************************************************************************************
 Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
 *                                                                                    *
 * This program is free software; you can redistribute it and/or modify it under      *
 * the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
 \************************************************************************************/

#import <UIKit/UIKit.h>
#import "main.h"

//int main(int argc, char *argv[]) {

int(*april_RealMain)(int argc, char** argv);

#undef main
int main(int argc, char** argv)
{
	return april_main(april_real_main, argc, argv);
}

int april_main (int(*real_main)(int argc, char** argv), int argc, char **argv)
{
	april_RealMain = real_main;
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, nil, @"AprilUIKitDelegate");
    [pool release];
    return retVal;
}
