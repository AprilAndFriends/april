/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _UWP_WINDOW
#include "UWP.h"

namespace april
{
	UWP_App^ UWP::app = nullptr;

	IFrameworkView^ UWP::FrameworkViewSource::CreateView()
	{
		UWP::app = ref new UWP_App();
		return UWP::app;
	}

}
#endif
