/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica                                                       *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_SDL_WINDOW_H
#define APRIL_SDL_WINDOW_H

#ifdef HAVE_MARMELADE

#include <s3e.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Window.h"

namespace april
{
	class MarmeladeWindow : public Window
	{
	public:
		MarmeladeWindow(int w, int h, bool fullscreen, chstr title);
		~MarmeladeWindow();
		
		// implementations
		void enterMainLoop();
		void terminateMainLoop();
		void destroyWindow();
		void showSystemCursor(bool visible);
		bool isSystemCursorShown();
		int getWidth();
		int getHeight();
		void setWindowTitle(chstr title);
		gvec2 getCursorPosition();
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();
		bool isCursorInside();

		float mCursorX; // TODO turn into private
		float mCursorY; // TODO turn into private
		
	private:
	
		void _handleKeyEvent(s3eKeyboardEvent *evt, s3eKey *key, unsigned int unicode);
		void _handleMouseEvent(s3ePointerEvent *pointerevt,
			s3ePointerMotionEvent *motionevt,
			s3ePointerTouchEvent *touchevt,
			s3ePointerTouchMotionEvent *touchmotionevt);
		void _handleDisplayAndUpdate();
		
		
		uint16 *mScreen;
		bool mRunning;
		bool mCursorVisible;
		bool mCursorInside;

	};
}

#endif
#endif
