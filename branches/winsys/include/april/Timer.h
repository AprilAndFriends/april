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

#ifdef _WIN32
    #include "TimerWin.h"
#elif defined(_LINUX)
    #include "TimerLinux.h"
#elif defined(HAVE_SDL)
	#include "TimerSDL.h"
#else
	#warning Neither _WIN32, nor _LINUX, nor HAVE_SDL are included
#endif

#endif
