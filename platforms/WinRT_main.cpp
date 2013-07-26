/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "main.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinRT_View.h"
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
    Application::Start(ref new ApplicationInitializationCallback(
        [](ApplicationInitializationCallbackParams^ p)
		{
            april::WinRT::App = ref new april::WinRT_XamlApp();
        }
	));
	return 0;
}
#endif
