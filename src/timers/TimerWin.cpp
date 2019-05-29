/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WIN32
#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hplatform.h>

#include "april.h"
#include "RenderSystem.h"
#include "Timer.h"

namespace april
{
	Timer::Timer()
	{
		this->difference = 0.0;
		this->td2 = 0.0;
		this->frequency = (int64_t)0;
		this->resolution = 0;
		this->start = (int64_t)0;
		this->performanceTimer = false;
		this->performanceTimerStart = 0;
		this->performanceTimerElapsed = 0;
		if (!QueryPerformanceFrequency((LARGE_INTEGER*)&this->frequency))
		{
			hlog::warn(logTag, "Performance timer not available, multimedia timer will be used instead!");
			this->start = htickCount();
			this->resolution = 0.001;
			this->frequency = (int64_t)1000;
		}
		else
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&this->performanceTimerStart);
			this->performanceTimer = true;
			this->resolution = 1.0 / this->frequency;
			this->performanceTimerElapsed = this->performanceTimerStart;
			this->start = this->performanceTimerStart;
		}
		this->td1 = (double)this->start;
	}
	
	Timer::~Timer()
	{
	}
	
	double Timer::getTime() const
	{
		if (this->performanceTimer)
		{
			int64_t time;
			QueryPerformanceCounter((LARGE_INTEGER*)&time);
			return ((double)(time - this->performanceTimerStart));
		}
		return ((double)(htickCount() - this->start));
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
