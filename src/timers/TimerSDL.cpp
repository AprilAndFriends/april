/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _SDL_WINDOW
#include <SDL/SDL.h>

#include "RenderSystem.h"
#include "Timer.h"

namespace april
{
	Timer::Timer()
	{
		this->dt = 0;
		this->td2 = 0;
		this->td = 0;
		this->frequency = 0;
		this->performanceTimerStart = 0;
		this->resolution = 0.001;
		this->mTimerStart = 0;
		this->mTimerElapsed = 0;
		performanceTimerElapsed = 0;
		performanceTimer = 0;
		// for sdl:
		performanceTimer = 0; // was: "false"
		this->mTimerStart = SDL_GetTicks();
		this->td1 = this->mTimerStart;
		this->frequency	= 1000;
		this->mTimerElapsed = this->mTimerStart;
	}
	
	Timer::~Timer()
	{
	}
	
	double Timer::getTime()
	{
		return (double)(SDL_GetTicks() - this->mTimerStart);
	}
	
	double Timer::diff(bool doUpdate)
	{
		if (doUpdate)
		{
			this->update();
		}
		return this->dt;
	}
	
	void Timer::update()
	{
		this->td2 = this->getTime();
		this->difference = hmax((this->td2 - this->td1) * this->resolution, 0.0); // limiting to 0 in case user has moved the clock back, don't allow negative increments
		this->td1 = this->td2;
	}
	
}
#endif
