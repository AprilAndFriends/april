/// @file
/// @version 4.5
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

namespace april
{
	extern harray<hstr> args;
}

int __aprilMain(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)(), int argc, char** argv)
{
	if (argv != NULL && argv[0] != NULL)
	{
		for_iter (i, 0, argc)
		{
			april::args += argv[i];
		}
	}
	april::WinRT::Init = aprilApplicationInit;
	april::WinRT::Destroy = aprilApplicationDestroy;
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
