/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica                                                       *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_MAIN_H
#define APRIL_MAIN_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>

#ifndef BUILDING_APRIL
#define main __ STOP_USING_MAIN___DEPRECATED_IN_APRIL
#endif
    
extern void april_init(const harray<hstr>& argv);
extern void april_destroy();

#define APRIL_NO_MAIN 1
	
#endif
