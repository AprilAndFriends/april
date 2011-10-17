/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica                                                       *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_WIN32_WINDOW_H
#define APRIL_WIN32_WINDOW_H

#include <hltypes/hstring.h>

#include "Window.h"
#include "aprilExport.h"

namespace april
{
	class aprilExport Win32Window : public Window
	{

		bool mTouchEnabled; //! Wheter or not a win7+ touchscreen was detected
	public:
		Win32Window(int w, int h, bool fullscreen, chstr title);
		~Win32Window();
		
		void _setActive(bool active) { mActive = active; }
		
		// implementations
		void enterMainLoop();
		void terminateMainLoop();
		void destroyWindow();
		bool isRunning() { return mRunning; }
		void showSystemCursor(bool visible);
		bool isSystemCursorShown();
		int getWidth();
		int getHeight();
		void setWindowTitle(chstr title);
		void _setResolution(int w, int h);
		gvec2 getCursorPosition();
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();
		
		// event handlers
		void triggerKeyEvent(bool down, unsigned int keycode);
		void triggerCharEvent(unsigned int chr);
	
		void triggerMouseUpEvent(int button);
		void triggerMouseDownEvent(int button);
		void triggerMouseMoveEvent();
		bool triggerQuitEvent();
		void triggerFocusCallback(bool focused);

		void triggerTouchscreenCallback(bool enabled);

		DeviceType getDeviceType();
		
		float mCursorX; // TODO turn into private
		float mCursorY; // TODO turn into private
		
	private:
		bool mRunning;
		bool mActive;
		
	};
}

#endif
