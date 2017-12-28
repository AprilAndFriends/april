/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINUWP
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Application.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinUWP.h"
#include "WinUWP_App.h"

using namespace Windows::ApplicationModel::Core;

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
		// TODOx - remove
		april::WinUWP::Init = aprilApplicationInit;
		april::WinUWP::Destroy = aprilApplicationDestroy;
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
		delete april::application;
		april::application = NULL;
		return 0;
	}
	
}
#endif
