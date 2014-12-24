/// @file
/// @version 3.5
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
	/// @brief Defines a delegate for special system callbacks.
	class aprilExport SystemDelegate
	{
	public:
		/// @brief Basic constructor.
		SystemDelegate();
		/// @brief Destructor.
		virtual ~SystemDelegate();

		/// @brief Called when system initiates quitting.
		/// @param[in] canCancel Whether quitting can be canceled.
		/// @return True if system can quit. False if system should not quit (will be ignored if canCancel is false).
		virtual bool onQuit(bool canCancel);
		/// @brief Called after window size has changed.
		/// @param[in] width New width of the window.
		/// @param[in] height New height of the window.
		/// @param[in] fullscreen Whether the window is fullscreen.
		virtual void onWindowSizeChanged(int width, int height, bool fullscreen);
		/// @brief Called after window focus has changed.
		/// @param[in] focused Whether the window is focused right now.
		virtual void onWindowFocusChanged(bool focused);
		/// @brief Called after input mode has changed.
		/// @param[in] inputMode Which InputMode is currently active.
		virtual void onInputModeChanged(Window::InputMode inputMode);
		/// @brief Called after virtual keyboard visibility has changed.
		/// @param[in] visible Whether the virtual keyboard is currently visible.
		/// @param[in] heightRatio Range of [0, 1] defining how much of the window-height the virtual keyboard takes up.
		virtual void onVirtualKeyboardChanged(bool visible, float heightRatio);
		/// @brief Called when the system is running low on RAM.
		virtual void onLowMemoryWarning();

	};

}
#endif
