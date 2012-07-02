/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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
		this->mmTimerStart = 0;
		this->mmTimerElapsed = 0;
		this->performanceTimerElapsed = 0;
		this->performanceTimer = 0;
		
		// for posix:
		timeval tv = {0, 0};
		gettimeofday(&tv, NULL);
		
		this->performanceTimer = 0; 
		this->mmTimerStart = ((uint64_t(tv.tv_sec)) << 32) + int64_t(tv.tv_usec);
		this->frequency = 1;
		this->mmTimerElapsed = this->mmTimerStart;
	}
	
	Timer::~Timer()
	{
	}
	
	float Timer::getTime()
	{
		timeval tv = {0, 0};
		timeval init_tv = {this->mmTimerStart >> 32, this->mmTimerStart & 0xFFFFFFFF};
		gettimeofday(&tv, NULL);
		return (tv.tv_usec - init_tv.tv_usec) / 1000 + (tv.tv_sec - init_tv.tv_sec) * 1000;
	}
	
	float Timer::diff(bool update)
	{
		if (update)
		{
			this->update();
			return this->dt;
		}
		return this->dt;
	}
	
	void Timer::update()
	{
		this->td2 = this->getTime();
		this->dt = (this->td2 - this->td) * 0.001f;
		this->td = this->td2;
	}

}
#endif