/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinRT ViewSource.

#if defined(_WINRT_WINDOW) && defined(_WINP8)
#ifndef APRIL_WINRT_VIEW_SOURCE_H
#define APRIL_WINRT_VIEW_SOURCE_H

#include <hltypes/hplatform.h>

using namespace Windows::ApplicationModel::Core;

namespace april
{
	ref class WinRT_ViewSource : public IFrameworkViewSource
	{
	public:
		virtual IFrameworkView^ CreateView();

	};

}

#endif
#endif
