/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WINRT) && !defined(_OPENKODE)
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinRT.h"
#include "WinRT_XamlApp.h"

using namespace Windows::UI::Xaml;

int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv)
{
	april::WinRT::Args.clear();
	if (argv != NULL && argv[0] != NULL)
	{
		for_iter (i, 0, argc)
		{
			april::WinRT::Args += argv[i];
		}
	}
	april::WinRT::Init = anAprilInit;
	april::WinRT::Destroy = anAprilDestroy;
#ifdef _WINRT_WINDOW
	Application::Start(ref new ApplicationInitializationCallback(
		[](ApplicationInitializationCallbackParams^ p)
		{
			april::WinRT::App = ref new april::WinRT_XamlApp();
		}
	));
#endif
	return 0;
}
#endif
