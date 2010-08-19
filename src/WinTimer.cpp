/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include "Timer.h"

namespace April
{
    Timer::Timer()
    {
        dt = 0;
        td2 = 0;
        td = 0;
        frequency = 0;
        performance_timer_start = 0;
        resolution = 0;
        mm_timer_start = 0;
        mm_timer_elapsed = 0;
        performance_timer_elapsed = 0;
        performance_timer = 0;
        
        // Check To See If A Performance Counter Is Available
        // If One Is Available The Timer Frequency Will Be Updated
        if (!QueryPerformanceFrequency((LARGE_INTEGER *) &frequency))
        {
            // No Performace Counter Available
            printf("[april] performance timer not available, multimedia timer will be used instead!\n");
            performance_timer	= FALSE;					// Set Performance Timer To FALSE
            mm_timer_start	= timeGetTime();			    // Use timeGetTime() To Get Current Time
            resolution		= 1.0f/1000.0f;				    // Set Our Timer Resolution To .001f
            frequency			= 1000;						// Set Our Timer Frequency To 1000
            mm_timer_elapsed	= mm_timer_start;		    // Set The Elapsed Time To The Current Time
        }
        else
        {
            // Performance Counter Is Available, Use It Instead Of The Multimedia Timer
            printf("[april] performance timer will be used.\n");
            // Get The Current Time And Store It In performance_timer_start
            QueryPerformanceCounter((LARGE_INTEGER *) &performance_timer_start);
            performance_timer			= TRUE;				// Set Performance Timer To TRUE
            // Calculate The Timer Resolution Using The Timer Frequency
            resolution				= (float) (((double)1.0f)/((double)frequency));
            // Set The Elapsed Time To The Current Time
            performance_timer_elapsed	= performance_timer_start;
        }
            
        
    }
    
    Timer::~Timer()
    {
        
    }
    
    float Timer::getTime()
    {
        __int64 time;									// time Will Hold A 64 Bit Integer
        if (performance_timer)							// Are We Using The Performance Timer?
        {
            QueryPerformanceCounter((LARGE_INTEGER *) &time);	// Grab The Current Performance Time
            // Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
            return ( (float) ( time - performance_timer_start) * resolution)*1000.0f;
        }
        else
        {
            // Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
            return( (float) ( timeGetTime() - mm_timer_start) * resolution)*1000.0f;
        }
    }
    
    float Timer::diff(bool update)
    {
        if(update)
        {
            td2 = getTime();
            dt = (td2-td) * 0.001f;
            td = td2;
            return dt;
        }
        else
            return dt;
        
    }
    
    void Timer::update()
    {
        td2 = getTime();
        dt = (td2-td) * 0.1f;
        td = td2;
    }
}
