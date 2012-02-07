/// @file
/// @author  Boris Mikic
/// @version 1.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <windows.h>

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "main.h"
#include "RenderSystem.h"
#include "Window.h"

bool gAprilShouldInvokeQuitCallback = false;

int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int _argc, char **_argv)
{
    harray<hstr> args;
	if(_argv)
	{
		for (int i = 0; i < _argc; i++) 
		{
			args += _argv[i];
		}
	}
	
	anAprilInit(args);
    april::rendersys->getWindow()->enterMainLoop();
	anAprilDestroy();
	return 0;
}
#endif