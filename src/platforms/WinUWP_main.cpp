/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WINUWP) && !defined(_OPENKODE)
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinUWP.h"
#include "WinUWP_App.h"

using namespace Windows::ApplicationModel::Core;

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
	april::WinUWP::Init = anAprilInit;
	april::WinUWP::Destroy = anAprilDestroy;
#ifdef _WINUWP_WINDOW
	IFrameworkViewSource^ frameworkViewSource = ref new april::FrameworkViewSource();
	//april::WinUWP::App = ;
	CoreApplication::Run(frameworkViewSource);
	//return 0;
	/*
	Application::Start(ref new ApplicationInitializationCallback(
		[](ApplicationInitializationCallbackParams^ p)
		{
			april::WinUWP::App = ref new april::WinUWP_App();
		}
	));
	*/
#endif
	return 0;
}
#endif
