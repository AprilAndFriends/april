/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_UNIX) || defined(_ANDROID)
#include <sys/time.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "RenderSystem.h"
#include "Timer.h"

namespace april
{
	Timer::Timer()
	{
		this->difference = 0.0f;
		this->td1 = 0.0;
		this->td2 = 0.0;
		this->frequency = 1LL;
		this->resolution = 0.001;
		this->start = (int64_t)htickCount();
		this->performanceTimer = false;
		this->performanceTimerStart = 0;
		this->performanceTimerElapsed = 0;
	}
	
	Timer::~Timer()
	{
	}
	
	double Timer::getTime() const
	{
		return ((double)((int64_t)htickCount() - this->start) * this->resolution * 1000.0);
	}
	
	float Timer::diff(bool update)
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
		this->difference = (float)((this->td2 - this->td1) * 0.001);
		if (this->difference < 0)
		{
			this->difference = 0; // in case user has moved the clock back, don't allow negative increments
		}
		this->td1 = this->td2;
	}

}
#endif