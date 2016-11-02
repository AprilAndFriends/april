/// @file
/// @version 4.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENKODE
#include <KD/kd.h>

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "main_base.h"
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
	hlog::write(april::logTag, "Initializing OpenKODE: " + hstr(kdQueryAttribcv(KD_ATTRIB_VERSION)));
	anAprilInit(april::args);
	if (april::window != NULL && april::rendersys != NULL)
	{
		april::window->enterMainLoop();
	}
	anAprilDestroy();
	return 0;
}
#endif
