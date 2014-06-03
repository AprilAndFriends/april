/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WINRT_WINDOW) && defined(_WINP8)
#include <hltypes/hplatform.h>

#include "WinP8_App.h"
#include "WinP8_AppLauncher.h"

namespace april
{
	IFrameworkView^ WinP8_AppLauncher::CreateView()
	{
		return ref new WinP8_App();
	}
	
}
#endif
