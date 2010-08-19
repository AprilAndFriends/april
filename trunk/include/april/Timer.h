/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_TIMER_H
#define APRIL_TIMER_H

#include <windows.h>
#include <stdio.h>
#include "AprilExport.h"

namespace April
{
    class AprilExport Timer
    {
        
        float dt, td, td2;
        __int64       frequency;									// Timer Frequency
        float         resolution;									// Timer Resolution
        unsigned long mm_timer_start;								// Multimedia Timer Start Value
        unsigned long mm_timer_elapsed;								// Multimedia Timer Elapsed Time
        bool		  performance_timer;							// Using The Performance Timer?
        __int64       performance_timer_start;						// Performance Timer Start Value
        __int64       performance_timer_elapsed;					// Performance Timer Elapsed Time
        
        
    public:
        
        Timer();
        ~Timer();
        
        float getTime();
        float diff(bool update = true);
        
        void update();
    };
}

#endif
