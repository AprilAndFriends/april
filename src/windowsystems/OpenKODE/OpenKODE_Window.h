/// @file
/// @version 4.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
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
		float launchDelay;
	public:
		OpenKODE_Window();
		~OpenKODE_Window();
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		bool destroy();

		int getWidth() const;
		int getHeight() const;
		void setTitle(chstr title);
		bool isCursorVisible() const;
		void setCursorVisible(bool value);
		void* getBackendId() const;
		void setResolution(int w, int h, bool fullscreen);

		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);

		void handleActivityChange(bool active);

		bool updateOneFrame();
		void presentFrame();
		void checkEvents();

		void showVirtualKeyboard();
		void hideVirtualKeyboard();
		Cursor* _createCursor(bool fromResource);

	protected:
		int _getAprilTouchIndex(int kdIndex) const;
		
		bool kdTouches[16];
		KDWindow* kdWindow;

		bool _isMousePointer() const;
		bool _processEvent(const KDEvent* evt);

	};

}
#endif
#endif