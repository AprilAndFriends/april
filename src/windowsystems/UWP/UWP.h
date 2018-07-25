/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines UWP utility and global stuff.

#ifdef _UWP_WINDOW
#ifndef APRIL_UWP_H
#define APRIL_UWP_H

#include "UWP_App.h"

using namespace Windows::ApplicationModel;
using namespace Windows::Graphics::Display;

namespace april
{
	ref class UWP_App;

	class UWP
	{
	public:
		ref class FrameworkViewSource sealed : IFrameworkViewSource
		{
		public:
			virtual IFrameworkView^ CreateView();

		};

		static UWP_App^ app;
		static float inline getDpiRatio() { return getDpiRatio(DisplayInformation::GetForCurrentView()->LogicalDpi); }
		static float inline getDpiRatio(float dpi) { return (dpi / 96.0f); }

	};
	
}
#endif
#endif
