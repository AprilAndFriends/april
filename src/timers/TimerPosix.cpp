/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_UNIX) || defined(_ANDROID)
#include <sys/time.h>

#include "RenderSystem.h"
#include "Timer.h"

namespace april
{
	Timer::Timer()
	{
		this->dt = 0;
		this->td = 0;
		this->td2 = 0;
		this->frequency = 0;
		this->performanceTimerStart = 0;
		this->resolution = 0; // unused in Posix timer
		this->start = 0;
		this->elapsed = 0;
		this->performanceTimerElapsed = 0;
		this->performanceTimer = 0;
		
		// for posix:
		timeval tv = {0, 0};
		gettimeofday(&tv, NULL);
		
		this->performanceTimer = 0; 
		this->start = ((uint64_t(tv.tv_sec)) << 32) + int64_t(tv.tv_usec);
		this->frequency = 1;
		this->elapsed = this->start;
	}
	
	Timer::~Timer()
	{
	}
	
	float Timer::getTime() const
	{
		timeval tv = {0, 0};
#ifdef __APPLE__
		timeval init_tv = { (time_t)(this->start >> 32), (__darwin_suseconds_t)(this->start & 0xFFFFFFFFFFFFFFFFLL) };
#else
		timeval init_tv = { (time_t)(this->start >> 32), (time_t)(this->start & 0xFFFFFFFFFFFFFFFFLL) };
#endif
		gettimeofday(&tv, NULL);
		return (tv.tv_usec - init_tv.tv_usec) / 1000 + (tv.tv_sec - init_tv.tv_sec) * 1000;
	}
	
	float Timer::diff(bool update)
	{
		if (update)
		{
			this->update();
		}
		return this->dt;
	}
	
	void Timer::update()
	{
		this->td2 = this->getTime();
		this->dt = (this->td2 - this->td) * 0.001f;
		if (this->dt < 0)
		{
			this->dt = 0; // in case user has moved the clock back, don't allow negative increments
		}
		this->td = this->td2;
	}

}
#endif