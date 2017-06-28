/// @file
/// @version 4.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WIN32) && !defined(_OPENKODE) && !defined(_WINRT)
#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "Window.h"

int gAprilShouldInvokeQuitCallback = 0;

namespace april
{
	extern harray<hstr> args;
}

int __april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv)
{
	if (argv != NULL && argv[0] != NULL)
	{
		for_iter (i, 0, argc)
		{
			april::args += argv[i];
		}
	}
	anAprilInit(april::args);
	if (april::window != NULL && april::rendersys != NULL)
	{
		april::window->enterMainLoop();
	}
	anAprilDestroy();
	return 0;
}
#endif
