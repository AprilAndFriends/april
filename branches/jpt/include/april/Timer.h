/// @file
/// @author  Kresimir Spes
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic timer.

#ifndef APRIL_TIMER_H
#define APRIL_TIMER_H

#ifdef _WIN32
#include <windows.h>
#else
#include <stdint.h>
#define __int64 uint64_t
#endif
#include <stdio.h>

#include "aprilExport.h"

namespace april
{
    class aprilExport Timer
    {
	public:
		Timer();
		~Timer();
		
		float getTime();
		float diff(bool update = true);
        
        void update();
		
	protected:
		float mDt;
		float mTd;
		float mTd2;
		__int64 mFrequency;
		float mResolution;
		__int64 mMmTimerStart;
		unsigned long mMmTimerElapsed;
		bool mPerformanceTimer;
		__int64 mPerformanceTimerStart;
		__int64 mPerformanceTimerElapsed;
		
	};
}

#endif
