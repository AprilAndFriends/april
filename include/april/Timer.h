/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_TIMER_WIN_H
#define APRIL_TIMER_WIN_H

#ifdef _WIN32
#include <windows.h>
#else
#include <stdint.h>
#define __int64 uint64_t
#endif

#include <stdio.h>
#include "AprilExport.h"

namespace April
{
    class AprilExport Timer
    {
        
        float mDt, mTd, mTd2;
        __int64       mFrequency;
        float         mResolution;
        __int64	      mMmTimerStart;
        unsigned long mMmTimerElapsed;
        bool		  mPerformanceTimer;
        __int64       mPerformanceTimerStart;
        __int64       mPerformanceTimerElapsed;
        
        
    public:
        
        Timer();
        ~Timer();
        
        float getTime();
        float diff(bool doUpdate = true);
        
        void update();
    };
}

#endif
