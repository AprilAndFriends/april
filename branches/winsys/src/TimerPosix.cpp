/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <sys/time.h>
#include "Timer.h"
#include "RenderSystem.h"

namespace April
{
	

    Timer::Timer()
    {
        mDt = 0;
        mTd2 = 0;
        mTd = 0;
        mFrequency = 0;
        mPerformanceTimerStart = 0;
        mResolution = 0; // unused in SDL timer
        mMmTimerStart = 0;
        mMmTimerElapsed = 0;
        mPerformanceTimerElapsed = 0;
        mPerformanceTimer = 0;
        
		// for posix:
		timeval tv = { 0, 0 };
		gettimeofday(&tv, NULL);
		
		mPerformanceTimer	= 0; 
		mMmTimerStart	    = ((uint64_t(tv.tv_sec)) << 32) + int64_t(tv.tv_usec);
		mFrequency			= 1;
		mMmTimerElapsed	    = mMmTimerStart;
		
        
    }
    
    Timer::~Timer()
    {
        
    }
    
    float Timer::getTime()
    {
		timeval tv = { 0, 0 };
		timeval init_tv = { mMmTimerStart >> 32, mMmTimerStart & 0xFFFFFFFF };
		gettimeofday(&tv, NULL);
		
		
		return	(tv.tv_usec - init_tv.tv_usec) / 1000 +  // microsecs into millisecs
				(tv.tv_sec - init_tv.tv_sec) * 1000;
    }
    
    float Timer::diff(bool update)
    {
        if(update)
        {
            this->update();
            return mDt;
        }
        else
            return mDt;
        
    }
    
    void Timer::update()
    {
        mTd2 = getTime();
        mDt = (mTd2-mTd) * 0.1f;
        mTd = mTd2;
    }
}
