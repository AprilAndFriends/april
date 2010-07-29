/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
Heavily based on SDLMain.m                                                           *
 by Darrell Walisser <dwaliss1@purdue.edu>                                           *
 and Max Horn <max@quendi.de>                                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#import <Cocoa/Cocoa.h>

static int gArgc=0;
static char** gArgv;
static BOOL gFinderLaunch=NO;
static int(*gRealMain)(int argc, char** argv);

/* For some reaon, Apple removed setAppleMenu from the headers in 10.4,
 but the method still is there and works. To avoid warnings, we declare
 it ourselves here. */
@interface NSApplication(April_Missing_Methods)
- (void)setAppleMenu:(NSMenu *)menu;
@end





@interface AprilApplication : NSApplication
@end

@implementation AprilApplication
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
    /* Post a SDL_QUIT event */
    //SDL_Event event;
    //event.type = SDL_QUIT;
    //SDL_PushEvent(&event);
}
@end




@interface AprilAppDelegate : NSObject
@end


/* The main class of the application, the application's delegate */
@implementation AprilAppDelegate

static NSString *getApplicationName(void)
{
    NSDictionary *dict;
    NSString *appName = 0;
	
    /* Determine the application name */
    dict = (NSDictionary *)CFBundleGetInfoDictionary(CFBundleGetMainBundle());
    if (dict)
        appName = [dict objectForKey: @"CFBundleName"];
    
    if (![appName length])
        appName = [[NSProcessInfo processInfo] processName];
	
    return appName;
}

static void setApplicationMenu(void)
{
    /* warning: this code is very odd */
    NSMenu *appleMenu;
    NSMenuItem *menuItem;
    NSString *title;
    NSString *appName;
    
    appName = getApplicationName();
    appleMenu = [[NSMenu alloc] initWithTitle:@""];
    
    /* Add menu items */
    title = [@"About " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
	
    [appleMenu addItem:[NSMenuItem separatorItem]];
	
    title = [@"Hide " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];
	
    menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
    [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];
	
    [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
	
    [appleMenu addItem:[NSMenuItem separatorItem]];
	
    title = [@"Quit " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];
	
    
    /* Put menu into the menubar */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [menuItem setSubmenu:appleMenu];
    [[NSApp mainMenu] addItem:menuItem];
	
    /* Tell the application object that this is now the application menu */
    [NSApp setAppleMenu:appleMenu];
	
    /* Finally give up our references to the objects */
    [appleMenu release];
    [menuItem release];
}

/* Create a window menu */
static void setupWindowMenu(void)
{
    NSMenu      *windowMenu;
    NSMenuItem  *windowMenuItem;
    NSMenuItem  *menuItem;
	
    windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    /* "Minimize" item */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [windowMenu addItem:menuItem];
    [menuItem release];
    
    /* Put menu into the menubar */
    windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
    [windowMenuItem setSubmenu:windowMenu];
    [[NSApp mainMenu] addItem:windowMenuItem];
    
    /* Tell the application object that this is now the window menu */
    [NSApp setWindowsMenu:windowMenu];
	
    /* Finally give up our references to the objects */
    [windowMenu release];
    [windowMenuItem release];
}

/* Replacement for NSApplicationMain */
static void CustomApplicationMain (int argc, char **argv)
{
    NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
    AprilAppDelegate				*appDelegate;
	
    /* Ensure the application object is initialised */
    [AprilApplication sharedApplication];
    
#ifdef SDL_USE_CPS
    {
        CPSProcessSerNum PSN;
        /* Tell the dock about us */
        if (!CPSGetCurrentProcess(&PSN))
            if (!CPSEnableForegroundOperation(&PSN,0x03,0x3C,0x2C,0x1103))
                if (!CPSSetFrontProcess(&PSN))
                    [SDLApplication sharedApplication];
    }
#endif /* SDL_USE_CPS */
	
    /* Set up the menubar */
    [NSApp setMainMenu:[[NSMenu alloc] init]];
    setApplicationMenu();
    setupWindowMenu();
	
    /* Create AprilAppDelegate and make it the app delegate */
    appDelegate = [[AprilAppDelegate alloc] init];
    [NSApp setDelegate:appDelegate];
    
    /* Start the main event loop */
    [NSApp run];
    
    [appDelegate release];
    [pool release];
}


/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;
	
    /* Hand off to main application code */
    //gCalledAppMainline = TRUE;
    status = gRealMain (gArgc, gArgv);
	
    /* We're done, thank you for playing */
    exit(status);
}
@end


/* Main entry point to executable - should *not* be SDL_main! */
int april_main (int(*real_main)(int argc, char** argv), int argc, char **argv)
{
    /* Copy the arguments into a global variable */
    /* This is passed if we are launched by double-clicking */
    
	gRealMain = real_main;
	if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
        gArgv = (char **) malloc(sizeof (char *) * 2);
        gArgv[0] = argv[0];
        gArgv[1] = NULL;
        gArgc = 1;
        gFinderLaunch = YES;
    } else {
        int i;
        gArgc = argc;
        gArgv = (char **) malloc(sizeof (char *) * (argc+1));
        for (i = 0; i <= argc; i++)
            gArgv[i] = argv[i];
        gFinderLaunch = NO;
    }
	
    CustomApplicationMain (argc, argv);
    return 0;
}
