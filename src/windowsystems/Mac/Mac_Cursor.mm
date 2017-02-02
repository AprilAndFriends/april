/// @file
/// @version 4.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <Cocoa/Cocoa.h>

#include <hltypes/hdir.h>
#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Mac_Cursor.h"

namespace april
{
	Mac_Cursor::Mac_Cursor(bool fromResource) : Cursor(fromResource)
	{
		systemCursor = NULL;
	}
	
	Mac_Cursor::~Mac_Cursor()
	{
		if (systemCursor)
		{
			[systemCursor release];
			systemCursor = NULL;
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
		
		if (filename.endsWith(".plist"))
		{
			hstr contents;
			if (this->fromResource)
			{
				hresource f;
				f.open(filename);
				contents = f.read();
			}
			else
			{
				hfile f;
				f.open(filename);
				contents = f.read();
			}
			NSData* plistData = [[NSString stringWithUTF8String:contents.cStr()] dataUsingEncoding:NSUTF8StringEncoding];
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
			hstr base = hdir::baseDir(filename), name = [nsname UTF8String];
			path = hdir::joinPath(base, name);
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
		if (this->fromResource)
		{
			hstr archive = hresource::getMountedArchives().tryGet("", "");
			if (archive.size() > 0)
			{
				path = hdir::joinPath(archive, path);
			}
		}
		image = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:path.cStr()]];

		if (!image)
		{
			hlog::write(logTag, "Error: Unable to load cursor image, plist found, but '" + path + "' not found.");
			return false;
		}
		systemCursor = [[NSCursor alloc] initWithImage:image hotSpot:hotSpot];
		[image release];
		return true;
	}
	
}
