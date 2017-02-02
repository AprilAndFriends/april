/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "Platform.h"
#import <UIKit/UIKit.h>

void getStaticiOSInfo(chstr name, april::SystemInfo& info)
{
	int w = info.displayResolution.x, h = info.displayResolution.y;
	// for future reference, look here: http://www.everymac.com/ultimate-mac-lookup/?search_keywords=iPad3%2C4
	// and here: http://www.paintcodeapp.com/news/ultimate-guide-to-iphone-resolutions
	
	if (name.startsWith("iPad"))
	{
		if (name.startsWith("iPad1"))
		{
			info.name = "iPad1";
			info.ram = 256;
			info.displayDpi = 132;
		}
		else if (name.startsWith("iPad2"))
		{
			info.name = "iPad2";
			info.ram = 512;
			info.cpuCores = 2;
			if (name == "iPad2,5" || name == "iPad2,6" || name == "iPad2,7") // iPad mini
			{
				info.name = "iPad Mini";
				info.displayDpi = 163;
			}
			else
				info.displayDpi = 132;
		}
		else if (name.startsWith("iPad3"))
		{
			if (name == "iPad3,4" || name == "iPad3,5" || name == "iPad3,6") // iPad4
			{
				info.name = "iPad4";
			}
			else
			{
				info.name = "iPad3";
			}
			info.ram = 1024;
			info.cpuCores = 2;
			info.displayDpi = 264;
		}
		else if (name.startsWith("iPad4"))
		{
			info.name = "iPad Air";
			info.ram = 1024;
			info.cpuCores = 2;
			if (name == "iPad4,4" || name == "iPad4,5") // iPad mini 2
			{
				info.name = "iPad Mini 2";
				info.displayDpi = 326;
			}
			else
				info.displayDpi = 264;
		}
		else
		{
			info.name = "iPad?";
			info.ram = 1024;
			info.cpuCores = 2;
			info.displayDpi = 264;
		}
	}
	else if (name.startsWith("iPhone"))
	{
		if (name == "iPhone1,1")
		{
			info.name = "iPhone2G";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPhone1,2")
		{
			info.name = "iPhone3G";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPhone2,1")
		{
			info.name = "iPhone3GS";
			info.ram = 256;
			info.displayDpi = 163;
		}
		else if (name.startsWith("iPhone3"))
		{
			info.name = "iPhone4";
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name.startsWith("iPhone4"))
		{
			info.name = "iPhone4S";
			info.cpuCores = 2;
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name.startsWith("iPhone5"))
		{
			info.name = "iPhone5";
			info.cpuCores = 2;
			info.ram = 1024;
			info.displayDpi = 326;
		}
		else if (name.startsWith("iPhone6"))
		{
			info.name = "iPhone5S";
			info.cpuCores = 2;
			info.ram = 1024;
			info.displayDpi = 326;
		}
		else if (name.startsWith("iPhone7,1"))
		{
			// iPhone6Plus has a resolution of 2208x1242 but is downscaled to 1920x1080
			// The physical DPI is 401, but because of this, it is better to use the upscaled equivalent DPI of 461
			// we also need to account for possible zoomed mode. there can be 2 zoomed modes, one for compatibility upscaling iphone5 resolution
			// and one as a feature, upscaling iphone6 resolution
			if (w == 1704)
			{
				info.name = "iPhone6Plus_Zoomed_4inch";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 355;
			}
			else if (w == 2001)
			{
				info.name = "iPhone6Plus_Zoomed_4.7inch";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 417;
			}
			else
			{
				info.name = "iPhone6Plus";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 461;
			}
		}
		else if (name.startsWith("iPhone7,2"))
		{
			if ([[UIScreen mainScreen] nativeScale] != 2)
			{
				info.name = "iPhone6_Zoomed_4inch";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 326;
			}
			else
			{
				info.name = "iPhone6";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 326;
			}
		}
		else if (name.startsWith("iPhone8,1"))
		{
			if ([[UIScreen mainScreen] nativeScale] != 2)
			{
				info.name = "iPhone6S_Zoomed_4inch";
				info.cpuCores = 2;
				info.ram = 2048;
				info.displayDpi = 316;
			}
			else
			{
				info.name = "iPhone6S";
				info.cpuCores = 2;
				info.ram = 2048;
				info.displayDpi = 326;
			}
		}
		else if (name.startsWith("iPhone8,2"))
		{
			if (w == 1704)
			{
				info.name = "iPhone6SPlus_Zoomed_4inch";
				info.cpuCores = 2;
				info.ram = 2048;
				info.displayDpi = 355;
			}
			else if (w == 2001)
			{
				info.name = "iPhone6SPlus_Zoomed_4.7inch";
				info.cpuCores = 2;
				info.ram = 2048;
				info.displayDpi = 417;
			}
			else
			{
				info.name = "iPhone6SPlus";
				info.cpuCores = 2;
				info.ram = 2048;
				info.displayDpi = 461;
			}
		}
		else
		{
			info.name = "iPhone?";
			info.ram = 1024;
			info.displayDpi = 326;
		}
	}
	else if (name.startsWith("iPod"))
	{
		if (name == "iPod1,1")
		{
			info.name = "iPod1";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPod2,1")
		{
			info.name = "iPod2";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPod3,1")
		{
			info.name = "iPod3";
			info.ram = 256;
			info.displayDpi = 163;
		}
		else if (name == "iPod4,1")
		{
			info.name = "iPod4";
			info.ram = 256;
			info.displayDpi = 326;
		}
		else if (name == "iPod5,1")
		{
			info.name = "iPod5";
			info.ram = 512;
			info.displayDpi = 326;
		}
		else
		{
			info.name = "iPod?";
			info.ram = 512;
			info.cpuCores = 2;
			info.displayDpi = 326;
		}
	}
	else if (name.startsWith("x86")) // iPhone Simulator
	{
		if ((float) w / h >= 3.0f / 2.0f) // iPhone
		{
			if (w == 480)
			{
				info.name = "iPhone3GS";
				info.ram = 256;
				info.displayDpi = 163;
			}
			else if (w == 960)
			{
				info.name = "iPhone4";
				info.ram = 512;
				info.displayDpi = 326;
			}
			else if (w == 1136)
			{
				if ([[UIScreen mainScreen] nativeScale] != 2)
				{
					info.name = "iPhone6_Zoomed_4inch";
					info.cpuCores = 2;
					info.ram = 1024;
					info.displayDpi = 316;
				}
				else
				{
					info.name = "iPhone5";
					info.cpuCores = 2;
					info.ram = 1024;
					info.displayDpi = 326;
				}
			}
			else if (w == 1334)
			{
				info.name = "iPhone6";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 326;
			}
			else if (w == 2208)
			{
				info.name = "iPhone6Plus";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 461;
			}
			else if (w == 1704)
			{
				info.name = "iPhone6Plus_Zoomed_4inch";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 355;
			}
			else if (w == 2001)
			{
				info.name = "iPhone6Plus_Zoomed_4.7inch";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 417;
			}
		}
		else
		{
			if (h == 768)
			{
				info.name = "iPad2";
				info.ram = 512;
				info.cpuCores = 2;
				info.displayDpi = 132;
			}
			else if (h == 1536)
			{
				info.name = "iPad3";
				info.ram = 1024;
				info.cpuCores = 2;
				info.displayDpi = 264;
			}
			else
			{
				info.name = "iPad Pro";
				info.ram = 4096;
				info.cpuCores = 4;
				info.displayDpi = 264;
			}
		}
	}
	//else: i386 (iphone simulator) and possible future device types
}
