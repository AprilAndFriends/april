/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenKODE window.

#ifdef _OPENKODE_WINDOW
#ifndef APRIL_OPENKODE_WINDOW_H
#define APRIL_OPENKODE_WINDOW_H
#include <KD/kd.h>

#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "egl.h"
#include "Window.h"

namespace april
{
	class aprilExport OpenKODE_Window : public Window
	{
	public:
		OpenKODE_Window();
		~OpenKODE_Window();
		bool create(int w, int h, bool fullscreen, chstr title, chstr options = "");
		bool destroy();

		HL_DEFINE_SET(gvec2, cursorPosition, CursorPosition);
		int getWidth();
		int getHeight();
		void setTitle(chstr title);
		bool isCursorVisible();
		void setCursorVisible(bool value);
		void* getBackendId();
		void _setResolution(int w, int h);

		bool updateOneFrame();
		void presentFrame();
		void checkEvents();

	protected:
		KDWindow* kdWindow;

	};

}
#endif
#endif