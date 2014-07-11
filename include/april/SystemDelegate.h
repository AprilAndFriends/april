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
/// Defines a delegate for special system callbacks.

#ifndef APRIL_SYSTEM_DELEGATE_H
#define APRIL_SYSTEM_DELEGATE_H

#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Window.h"

namespace april
{
	class aprilExport SystemDelegate
	{
	public:
		SystemDelegate();
		virtual ~SystemDelegate();

		bool onQuit(bool canCancel);
		void onWindowSizeChanged(int width, int height, Window::DeviceOrientation deviceOrientation);
		void onWindowFocusChanged(bool focused);
		void onVirtualKeyboardVisibilityChanged(bool visible);
		bool onHandleUrl(chstr url);
		void onLowMemoryWarning();

	};

}
#endif
