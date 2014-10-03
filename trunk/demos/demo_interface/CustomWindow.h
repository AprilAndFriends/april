/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifndef CUSTOM_WINDOW_H
#define CUSTOM_WINDOW_H

#include <april/Cursor.h>
#include <april/Window.h>
#include <hltypes/hstring.h>

class CustomWindow : public april::Window
{
public:
	CustomWindow();
	~CustomWindow();
	bool create(int w, int h, bool fullscreen, chstr title, april::Window::Options options);
	bool destroy();

	int getWidth();
	int getHeight();
	void* getBackendId();

	bool updateOneFrame();
	void presentFrame();
	void checkEvents();

	static LRESULT CALLBACK _processCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	HWND hWnd;

};

#endif
