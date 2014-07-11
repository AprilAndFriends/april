/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
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

		virtual bool onQuit(bool canCancel);
		virtual void onWindowSizeChanged(int width, int height, bool fullscreen);
		virtual void onWindowFocusChanged(bool focused);
		virtual void onInputModeChanged(Window::InputMode inputMode);
		virtual void onVirtualKeyboardChanged(bool visible, float heightRatio);
		virtual void onLowMemoryWarning();
	};

}
#endif
