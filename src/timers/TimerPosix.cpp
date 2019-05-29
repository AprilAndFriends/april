/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_UNIX) || defined(__ANDROID__)
#include <sys/time.h>
#include <time.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "RenderSystem.h"
#include "Timer.h"

#ifdef _IOS
	#import <Foundation/Foundation.h>
	#include <mach/mach_time.h>
#endif
#ifdef _MAC
	#include <sys/sysctl.h>
	#import <AppKit/NSWindow.h>
	#import <Foundation/NSString.h>
#endif
#ifdef __ANDROID__
	#include <sys/sysinfo.h>
#endif

namespace april
{
#ifdef _MAC
	inline static struct timeval _simpleUnixNowTime()
	{
		struct timeval result;
		struct timezone tz;
		gettimeofday(&result, &tz);
		return result;
	}
#endif
	
	inline static int64_t _currentMicroTime()
	{
#ifdef _IOS
		// iOS does not support CLOCK_MONOTONIC_RAW (before iOS 10 clock_gettime() wasn't supported at all)
		mach_timebase_info_data_t info;
		mach_timebase_info(&info);
		uint64_t result = mach_absolute_time();
		result *= info.numer;
		result /= info.denom;
		return (result / (int64_t)1000);
#else
#ifdef _MAC
		SInt32 minor;
		// Mac OSX only started supporting clock_gettime() after 10.12, can be removed when min. supported Mac version becomes 10.12
		if (Gestalt(gestaltSystemVersionMinor, &minor) == noErr && minor < 12)
		{
			struct timeval bootTime;
			size_t size = sizeof(bootTime);
			static int mib[2] = { CTL_KERN, KERN_BOOTTIME };
			if (sysctl(mib, 2, &bootTime, &size, NULL, 0) != -1 && bootTime.tv_sec != 0)
			{
				struct timeval now = _simpleUnixNowTime();
				// cast first, because if we multiply by 1000 before casting we could get an overflow on 32 bit systems
				int64_t tv_sec = (int64_t)(now.tv_sec - bootTime.tv_sec);
				int64_t tv_usec = (int64_t)(now.tv_usec - bootTime.tv_usec);
				return (tv_sec * (int64_t)1000000 + tv_usec);
			}
			return (int64_t)0;
		}
#endif
		struct timespec ts;
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0)
		{
			// cast first, because if we multiply by 1000 before casting we could get an overflow on 32 bit systems
			int64_t tv_sec = (int64_t)ts.tv_sec;
			int64_t tv_nsec = (int64_t)ts.tv_nsec;
			return (tv_sec * (int64_t)1000000 + tv_nsec / (int64_t)1000);
		}
		return (int64_t)0;
#endif
	}
	
	Timer::Timer()
	{
		this->difference = 0.0;
		this->td2 = 0.0;
		this->frequency = (int64_t)1000;
		this->resolution = 0.000001;
		this->start = _currentMicroTime();
		this->td1 = this->start;
		this->performanceTimer = false;
		this->performanceTimerStart = 0;
		this->performanceTimerElapsed = 0;
	}
	
	Timer::~Timer()
	{
	}
	
	double Timer::getTime() const
	{
		return (double)(_currentMicroTime() - this->start);
	}
	
	double Timer::diff(bool update)
	{
		if (update)
		{
			this->update();
		}
		return this->difference;
	}
	
	void Timer::update()
	{
		this->td2 = this->getTime();
		this->difference = hmax((this->td2 - this->td1) * this->resolution, 0.0); // limiting to 0 in case user has moved the clock back, don't allow negative increments
		this->td1 = this->td2;
	}

}
#endif
