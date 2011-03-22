/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2011 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#include <windows.h>
#include "main.h"
#include "RenderSystem.h"
#include "Window.h"
#include "aprilExport.h"
bool gAprilShouldInvokeQuitCallback = false;
aprilExport int april_main (void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int _argc, char **_argv)
{
    harray<hstr> argv;
	if(_argv)
	{
		for (int i = 0; i < _argc; i++) 
		{
			argv.push_back(_argv[i]);
		}
	}
	
	anAprilInit(argv);
    april::rendersys->getWindow()->enterMainLoop();
	anAprilDestroy();
	return 0;
}