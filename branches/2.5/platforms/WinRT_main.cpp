/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <hltypes/hplatform.h>
#if _HL_WINRT
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "main.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinRT_ViewSource.h"
#include "WinRT_View.h"

using namespace Windows::ApplicationModel::Core;

int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv)
{
	april::WinRT::Args.clear();
	if (argv != NULL)
	{
		for_iter (i, 0, argc)
		{
			april::WinRT::Args += argv[i];
		}
	}
	april::WinRT::Init = anAprilInit;
	april::WinRT::Destroy = anAprilDestroy;
	CoreApplication::Run(ref new april::WinRT_ViewSource());
	return 0;
}
#endif
#endif