/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "Platform.h"
#import <UIKit/UIKit.h>

void getStaticiOSInfo(chstr name, april::SystemInfo& info)
{
	int w = info.displayResolution.x;
	int h = info.displayResolution.y;
	// for future reference, look here: http://www.everymac.com/ultimate-mac-lookup/?search_keywords=iPad3%2C4
	// and here: http://www.paintcodeapp.com/news/ultimate-guide-to-iphone-resolutions
	
	if (name.startsWith("iPad"))
	{
		if (name.startsWith("iPad1,"))
		{
			info.name = "iPad 1";
			info.ram = 256;
			info.displayDpi = 132;
		}
		else if (name.startsWith("iPad2,"))
		{
			info.name = "iPad 2";
			info.ram = 512;
			info.cpuCores = 2;
			if (name == "iPad2,5" || name == "iPad2,6" || name == "iPad2,7") // iPad Mini
			{
				info.name = "iPad Mini";
				info.displayDpi = 163;
			}
			else
			{
				info.displayDpi = 132;
			}
		}
		else if (name.startsWith("iPad3,"))
		{
			if (name == "iPad3,4" || name == "iPad3,5" || name == "iPad3,6") // iPad4
			{
				info.name = "iPad 4";
			}
			else
			{
				info.name = "iPad 3";
			}
			info.ram = 1024;
			info.cpuCores = 2;
			info.displayDpi = 264;
		}
		else if (name.startsWith("iPad4,"))
		{
			info.ram = 1024;
			info.cpuCores = 2;
			if (name == "iPad4,4" || name == "iPad4,5" || name == "iPad4,6") // iPad Mini 2
			{
				info.name = "iPad Mini 2";
				info.displayDpi = 326;
			}
			else if (name == "iPad4,7" || name == "iPad4,8" || name == "iPad4,9") // iPad Mini 3
			{
				info.name = "iPad Mini 3";
				info.displayDpi = 326;
			}
			else
			{
				info.name = "iPad Air";
				info.displayDpi = 264;
			}
		}
		else if (name.startsWith("iPad5,"))
		{
			info.ram = 2048;
			if (name == "iPad5,1" || name == "iPad5,2") // iPad Mini 4
			{
				info.name = "iPad Mini 4";
				info.cpuCores = 2;
				info.displayDpi = 326;
			}
			else
			{
				info.name = "iPad Air 2";
				info.cpuCores = 3;
				info.displayDpi = 264;
			}
		}
		else if (name.startsWith("iPad6,"))
		{
			info.name = "iPad Pro";
			info.cpuCores = 2;
			info.displayDpi = 264;
			info.ram = 4096;
			if (name == "iPad6,11" || name == "iPad6,12") // 9.7"
			{
				info.name = "iPad 5";
				info.ram = 2048;
			}
		}
		else if (name.startsWith("iPad7,"))
		{
			info.name = "iPad Pro";
			info.cpuCores = 6;
			info.ram = 4096;
			info.displayDpi = 264;
			if (name == "iPad7,5" || name == "iPad7,6") // 9.7"
			{
				info.name = "iPad 6";
				info.cpuCores = 4;
				info.ram = 2048;
			}
			else if (name == "iPad7,11" ||  name == "iPad7,12")
			{
				info.name = "iPad 7";
			}
		}
		else if (name.startsWith("iPad8,"))
		{
			info.name = "iPad Pro";
			info.cpuCores = 8;
			info.displayDpi = 264;
			if (name == "iPad8,1" || name == "iPad8,2" || name == "iPad8,3" || name == "iPad8,4") // 12.9"
			{
				info.ram = 6144;
			}
			else // 11"
			{
				info.ram = 4096;
			}
		}
		else
		{
			info.name = "iPad ???";
			info.ram = 4096;
			info.cpuCores = 4;
			info.displayDpi = 264;
		}
	}
	else if (name.startsWith("iPhone"))
	{
		if (name == "iPhone1,1")
		{
			info.name = "iPhone 2G";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPhone1,2")
		{
			info.name = "iPhone 3G";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPhone2,1")
		{
			info.name = "iPhone 3GS";
			info.ram = 256;
			info.displayDpi = 163;
		}
		else if (name.startsWith("iPhone3,"))
		{
			info.name = "iPhone 4";
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name.startsWith("iPhone4,"))
		{
			info.name = "iPhone 4S";
			info.cpuCores = 2;
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name.startsWith("iPhone5,"))
		{
			info.cpuCores = 2;
			info.ram = 1024;
			info.displayDpi = 326;
			if (name == "iPhone5,1" || name == "iPhone5,2")
			{
				info.name = "iPhone 5";
			}
			else
			{
				info.name = "iPhone 5C";
			}
		}
		else if (name.startsWith("iPhone6,"))
		{
			info.name = "iPhone 5S";
			info.cpuCores = 2;
			info.ram = 1024;
			info.displayDpi = 326;
		}
		else if (name.startsWith("iPhone7,"))
		{
			info.cpuCores = 2;
			info.ram = 1024;
			if (name.startsWith("iPhone7,2"))
			{
				info.name = "iPhone 6";
				info.displayDpi = 326;
				if ([[UIScreen mainScreen] nativeScale] != 2)
				{
					info.name += " Zoomed_4inch";
					info.displayDpi = 316;
				}
			}
			else
			{
				// iPhone 6+ has a resolution of 2208x1242 but is downscaled to 1920x1080
				// The physical DPI is 401, but because of this, it is better to use the upscaled equivalent DPI of 461
				// we also need to account for possible zoomed mode. there can be 2 zoomed modes, one for compatibility upscaling iPhone 5 resolution
				// and one as a feature, upscaling iPhone 6 resolution
				info.name = "iPhone 6+";
				info.displayDpi = 461;
				if (w == 1704)
				{
					info.name += " Zoomed_4inch";
					info.displayDpi = 355;
				}
				else if (w == 2001)
				{
					info.name += " Zoomed_4.7inch";
					info.displayDpi = 417;
				}
			}
		}
		else if (name.startsWith("iPhone8,"))
		{
			info.cpuCores = 2;
			info.ram = 2048;
			if (name.startsWith("iPhone8,1"))
			{
				info.name = "iPhone 6S";
				info.displayDpi = 326;
				if ([[UIScreen mainScreen] nativeScale] != 2)
				{
					info.name += " Zoomed_4inch";
					info.displayDpi = 316;
				}
			}
			else if (name.startsWith("iPhone8,2"))
			{
				info.name = "iPhone 6S+";
				info.displayDpi = 461;
				if (w == 1704)
				{
					info.name += " Zoomed_4inch";
					info.displayDpi = 355;
				}
				else if (w == 2001)
				{
					info.name += " Zoomed_4.7inch";
					info.displayDpi = 417;
				}
			}
			else
			{
				info.name = "iPhone SE";
				info.displayDpi = 326;
			}
		}
		else if (name.startsWith("iPhone9,"))
		{
			info.cpuCores = 2;
			if (name == "iPhone9,1" || name == "iPhone9,3")
			{
				info.name = "iPhone 7";
				info.ram = 2048;
				info.displayDpi = 326;
				if ([[UIScreen mainScreen] nativeScale] != 2)
				{
					info.name += " Zoomed_4inch";
					info.displayDpi = 316;
				}
			}
			else
			{
				info.name = "iPhone 7+";
				info.ram = 3072;
				info.displayDpi = 461;
				if (w == 1704)
				{
					info.name += " Zoomed_4inch";
					info.displayDpi = 355;
				}
				else if (w == 2001)
				{
					info.name += " Zoomed_4.7inch";
					info.displayDpi = 417;
				}
			}
		}
		else if (name.startsWith("iPhone10,"))
		{
			info.cpuCores = 6;
			if (name == "iPhone10,1" || name == "iPhone10,4")
			{
				info.name = "iPhone 8";
				info.ram = 2048;
				info.displayDpi = 326;
			}
			else if (name == "iPhone10,2" || name == "iPhone10,5")
			{
				info.name = "iPhone 8+";
				info.ram = 3072;
				info.displayDpi = 461;
			}
			else
			{
				info.name = "iPhone X";
				info.ram = 3072;
				info.displayDpi = 458;
			}
		}
		else if (name.startsWith("iPhone11,"))
		{
			info.cpuCores = 6;
			if (name == "iPhone11,8")
			{
				info.name = "iPhone XR";
				info.ram = 2048;
				info.displayDpi = 326;
			}
			else if (name == "iPhone11,2")
			{
				info.name = "iPhone XS";
				info.ram = 2048;
				info.displayDpi = 458;
			}
			else
			{
				info.name = "iPhone XS MAX";
				info.ram = 3072;
				info.displayDpi = 458;
			}
		}
		else if (name.startsWith("iPhone12,"))
		{
			info.ram = 4096;
			info.cpuCores = 6;
			if (name == "iPhone12,5")
			{
				info.name = "iPhone 11 Pro Max";
				info.displayDpi = 326;
			}
			else if (name == "iPhone12,3")
			{
				info.name = "iPhone 11 Pro";
				info.displayDpi = 458;
			}
			else
			{
				info.name = "iPhone 11";
				info.displayDpi = 458;
			}
		}
		else
		{
			info.name = "iPhone ???";
			info.ram = 2048;
			info.displayDpi = 326;
		}
	}
	else if (name.startsWith("iPod"))
	{
		if (name == "iPod1,1")
		{
			info.name = "iPod Touch";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPod2,1")
		{
			info.name = "iPod Touch 2";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPod3,1")
		{
			info.name = "iPod Touch 3";
			info.ram = 256;
			info.displayDpi = 163;
		}
		else if (name == "iPod4,1")
		{
			info.name = "iPod Touch 4";
			info.ram = 256;
			info.displayDpi = 326;
		}
		else if (name == "iPod5,1")
		{
			info.name = "iPod Touch 5";
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name == "iPod7,1")
		{
			info.name = "iPod Touch 6";
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name == "iPod9,1")
		{
			info.name = "iPod Touch 7";
			info.displayDpi = 326;
		}
		else
		{
			info.name = "iPod ???";
			info.ram = 512;
			info.cpuCores = 2;
			info.displayDpi = 326;
		}
	}
	else if (name.startsWith("x86")) // iPhone Simulator
	{
		if ((float)w / h >= 1.5f) // iPhone
		{
			if (w == 480)
			{
				info.name = "iPhone 3GS";
				info.ram = 256;
				info.displayDpi = 163;
			}
			else if (w == 960)
			{
				info.name = "iPhone 4";
				info.ram = 512;
				info.displayDpi = 326;
			}
			else if (w == 1136)
			{
				if ([[UIScreen mainScreen] nativeScale] != 2)
				{
					info.name = "iPhone 6 Zoomed_4inch";
					info.cpuCores = 2;
					info.ram = 1024;
					info.displayDpi = 316;
				}
				else
				{
					info.name = "iPhone 5";
					info.cpuCores = 2;
					info.ram = 1024;
					info.displayDpi = 326;
				}
			}
			else if (w == 1334)
			{
				info.name = "iPhone 6";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 326;
			}
			else if (w == 2208)
			{
				info.name = "iPhone 6 Plus";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 461;
			}
			else if (w == 1704)
			{
				info.name = "iPhone 6+ Zoomed_4inch";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 355;
			}
			else if (w == 2001)
			{
				info.name = "iPhone 6+ Zoomed_4.7inch";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 417;
			}
			else if (w == 2436)
			{
				info.name = "iPhone X";
				info.cpuCores = 6;
				info.ram = 3072;
				info.displayDpi = 458;
			}
			else if (w == 1792)
			{
				info.name = "iPhone 11";
				info.cpuCores = 6;
				info.ram = 4096;
				info.displayDpi = 458;
			}
			else if (w == 2688)
			{
				info.name = "iPhone 11 Pro Max";
				info.cpuCores = 6;
				info.ram = 4096;
				info.displayDpi = 458;
			}
		}
		else
		{
			if (h == 768)
			{
				info.name = "iPad 2";
				info.ram = 512;
				info.cpuCores = 2;
				info.displayDpi = 132;
			}
			else if (h == 1536)
			{
				info.name = "iPad 3";
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
