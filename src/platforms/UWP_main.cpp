/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _UWP
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Application.h"
#include "april.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "Window.h"
#include "UWP.h"
#include "UWP_App.h"

using namespace Windows::ApplicationModel::Core;

namespace april
{
	int __mainStandard(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)(), int argc, char** argv)
	{
#ifdef _UWP_WINDOW
		harray<hstr> args;
		if (argv != NULL && argv[0] != NULL)
		{
			for_iter (i, 0, argc)
			{
				args += argv[i];
			}
		}
		april::application = new Application(aprilApplicationInit, aprilApplicationDestroy);
		april::application->setArgs(args);
		IFrameworkViewSource^ frameworkViewSource = ref new april::UWP::FrameworkViewSource();
		CoreApplication::Run(frameworkViewSource);
		if (april::application != NULL) // safe is safe
		{
			delete april::application;
			april::application = NULL;
		}
		UWP::app = nullptr;
#endif
		return april::getExitCode();
	}
	
}
#endif
