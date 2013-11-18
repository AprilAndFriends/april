#include "Platform.h"

void getStaticiOSInfo(chstr name, april::SystemInfo& info)
{

	if (name.starts_with("iPad"))
	{
		if (name.starts_with("iPad1"))
		{
			info.name = "iPad1";
			info.ram = 256;
			info.displayDpi = 132;
		}
		else if (name.starts_with("iPad2"))
		{
			info.name = "iPad2";
			info.ram = 512;
			info.cpuCores = 2;
			if (name == "iPad2,5") // iPad mini
				info.displayDpi = 163;
			else
				info.displayDpi = 132;
		}
		else if (name.starts_with("iPad3"))
		{
			info.name = "iPad3";
			info.ram = 1024;
			info.cpuCores = 2;
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
	else if (name.starts_with("iPhone"))
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
		else if (name.starts_with("iPhone3"))
		{
			info.name = "iPhone4";
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name.starts_with("iPhone4"))
		{
			info.name = "iPhone4S";
			info.cpuCores = 2;
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name.starts_with("iPhone5"))
		{
			info.name = "iPhone5";
			info.cpuCores = 2;
			info.ram = 1024;
			info.displayDpi = 326;
		}
		else
		{
			info.name = "iPhone?";
			info.ram = 1024;
			info.displayDpi = 326;
		}
	}
	else if (name.starts_with("iPod"))
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
		else
		{
			info.name = "iPod?";
			info.ram = 512;
			info.cpuCores = 2;
			info.displayDpi = 326;
		}
	}
	//else: i386 (iphone simulator) and possible future device types
}
