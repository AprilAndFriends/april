/// @file
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 1.51
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <windows.h>

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "main.h"
#include "RenderSystem.h"
#include "Window.h"

bool gAprilShouldInvokeQuitCallback = false;

int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv)
{
    harray<hstr> args;
	if (argv != NULL)
	{
		for_iter (i, 0, argc)
		{
			args += argv[i];
		}
	}
	anAprilInit(args);
    april::rendersys->getWindow()->enterMainLoop();
	anAprilDestroy();
	return 0;
}
#endif