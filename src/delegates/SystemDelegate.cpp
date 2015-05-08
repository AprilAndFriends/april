/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "SystemDelegate.h"

namespace april
{
	SystemDelegate::SystemDelegate()
	{
	}

	SystemDelegate::~SystemDelegate()
	{
	}

	bool SystemDelegate::onQuit(bool canCancel)
	{
		hlog::debug(logTag, "Event onQuit() was not implemented.");
		return true;
	}

	void SystemDelegate::onWindowSizeChanged(int width, int height, bool fullscreen)
	{
		hlog::debug(logTag, "Event onWindowSizeChanged() was not implemented.");
	}

	void SystemDelegate::onWindowFocusChanged(bool focused)
	{
		hlog::debug(logTag, "Event onWindowFocusChanged() was not implemented.");
	}

	void SystemDelegate::onInputModeChanged(Window::InputMode inputMode)
	{
		hlog::debug(logTag, "Event onInputModeChanged() was not implemented.");
	}

	void SystemDelegate::onVirtualKeyboardChanged(bool visible, float heightRatio)
	{
		hlog::debug(logTag, "Event onVirtualKeyboardChanged() was not implemented.");
	}

	void SystemDelegate::onLowMemoryWarning()
	{
		hlog::debug(logTag, "Event onLowMemoryWarning() was not implemented.");
	}

}
