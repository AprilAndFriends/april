/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WIN32) && !defined(_WINRT)
#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Application.h"
#include "april.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "Window.h"

int gAprilShouldInvokeQuitCallback = 0; // TODO - SDL depends on this, this hack should be removed

namespace april
{
	extern harray<hstr> args;

	int __mainStandard(void (*__aprilApplicationInit)(), void (*__aprilApplicationDestroy)(), int argc, char** argv)
	{
		harray<hstr> args;
		if (argv != NULL && argv[0] != NULL)
		{
			for_iter (i, 0, argc)
			{
				args += argv[i];
			}
		}
		april::application = new Application(__aprilApplicationInit, __aprilApplicationDestroy);
		april::application->init(args);
		if (april::window != NULL && april::rendersys != NULL)
		{
			april::application->enterMainLoop();
		}
		april::application->destroy();
		delete april::application;
		april::application = NULL;
		return 0;
	}
	
}
#endif
