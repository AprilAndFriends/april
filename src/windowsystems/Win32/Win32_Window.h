/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a Win32 window.

#ifdef _WIN32_WINDOW
#ifndef APRIL_WIN32_WINDOW_H
#define APRIL_WIN32_WINDOW_H

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Timer.h"
#include "Window.h"

namespace april
{
	class Cursor;

	class Win32_Window : public Window
	{
	public:
		Win32_Window();
		~Win32_Window();
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		bool destroy();

		void setTitle(chstr title);
		bool isCursorVisible() const;
		int getWidth() const;
		int getHeight() const;
		void* getBackendId() const;
		void setResolution(int w, int h, bool fullscreen);
		HCURSOR getCursorHandle() const;

		bool updateOneFrame();
		void presentFrame();
		void checkEvents();

		void showVirtualKeyboard();
		void hideVirtualKeyboard();

		static LRESULT CALLBACK childProcessCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
	protected:
		HWND hWnd;
		HCURSOR defaultCursor;
		hstr fpsTitle;
		bool fpsCounter;

		Cursor* _createCursor(bool fromResource);

		void _setupStyles(DWORD& style, DWORD& exstyle, bool fullscreen);
		void _adjustWindowSizeForClient(int x, int y, int& w, int& h, DWORD style, DWORD exstyle);
		void _refreshCursor();
		void _updateCursorPosition();

		static LRESULT CALLBACK _mainProcessCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	};

}

#endif
#endif
