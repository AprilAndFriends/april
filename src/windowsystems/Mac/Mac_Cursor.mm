/// @file
/// @version 3.36
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
#import <Cocoa/Cocoa.h>
#include <hltypes/hstring.h>
#include <hltypes/hresource.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include "Mac_Cursor.h"
#include "april.h"

namespace april
{
	Mac_Cursor::Mac_Cursor() : Cursor()
	{
		mCursor = NULL;
	}
	
	Mac_Cursor::~Mac_Cursor()
	{
		if (mCursor)
		{
			[mCursor release];
			mCursor = NULL;
		}
	}
	
	bool Mac_Cursor::_create(chstr filename)
	{
		if (!Cursor::_create(filename))
		{
			return false;
		}
		if (filename == "")
		{
			return false;
		}
		NSPoint hotSpot;
		NSImage* image;
		hstr path;
		
		if (filename.ends_with(".plist"))
		{
			hresource f(filename);
			hstr contents = f.read();
			f.close();
			NSData* plistData = [[NSString stringWithUTF8String:contents.c_str()] dataUsingEncoding:NSUTF8StringEncoding];
			NSString *error;
			NSPropertyListFormat format;
			NSDictionary* plist = [NSPropertyListSerialization propertyListFromData:plistData mutabilityOption:NSPropertyListImmutable format:&format errorDescription:&error];
			if (!plist)
			{
				hlog::write(logTag, "Error: " + hstr([error UTF8String]));
				[error release];
				return false;
			}
			NSString* nsname = [plist objectForKey:@"image"];
			if (!nsname)
			{
				hlog::write(logTag, "Error: invalid cursor plist, 'image' property is missing");
				return false;
			}
			hstr base = hdir::basedir(filename), name = [nsname UTF8String];
			path = hdir::join_path(base, name);
			NSNumber* x = [plist objectForKey:@"hotSpotX"];
			NSNumber* y = [plist objectForKey:@"hotSpotY"];
			if (x)
			{
				hotSpot.x = [x floatValue];
			}
			if (y)
			{
				hotSpot.y = [y floatValue];
			}
		}
		else
		{
			path = filename;
		}
		path = hdir::join_path(hresource::getArchive(), path);
		image = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:path.c_str()]];
        NSBitmapImageRep *rep = [[image representations] objectAtIndex:0];
        int renderedWidth = [rep pixelsWide];
        int renderedHeight = [rep pixelsHigh];
        
        float x = 1.0f;
        [rep setSize:NSMakeSize(renderedWidth / x, renderedHeight / x)];
        [image setSize:NSMakeSize(renderedWidth / x, renderedHeight / x)];
        
        [image addRepresentation:rep];
        [image removeRepresentation:[[image representations] objectAtIndex:0]];
        
		if (!image)
		{
			hlog::write(logTag, "Error: Unable to load cursor image, '" + filename + "' not found");
			return false;
		}
		mCursor = [[NSCursor alloc] initWithImage:image hotSpot:hotSpot];
		[image release];
		return true;
	}
}
