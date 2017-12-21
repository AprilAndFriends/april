/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINRT
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Application.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinRT.h"
#include "WinRT_XamlApp.h"

using namespace Windows::UI::Xaml;

namespace april
{
	int __mainStandard(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)(), int argc, char** argv)
	{
		harray<hstr> args;
		if (argv != NULL && argv[0] != NULL)
		{
			for_iter (i, 0, argc)
			{
				april::args += argv[i];
			}
		}
		april::application = new Application(aprilApplicationInit, aprilApplicationDestroy);
		april::application->setArgs(args);
		april::WinUWP::Init = aprilApplicationInit;
		april::WinUWP::Destroy = aprilApplicationDestroy;
#ifdef _WINRT_WINDOW
		Application::Start(ref new ApplicationInitializationCallback(
			[](ApplicationInitializationCallbackParams^ p)
			{
				april::WinRT::App = ref new april::WinRT_XamlApp();
			}
		));
#endif
		delete april::application;
		april::application = NULL;
		return 0;
	}
	
}
#endif
