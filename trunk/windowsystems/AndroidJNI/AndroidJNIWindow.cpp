/// @file
/// @author  Boris Mikic
/// @version 1.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#include "RenderSystem.h"
#include "Timer.h"
#include "AndroidJniWindow.h"

namespace april
{
	/*
	static HWND hWnd;
	static gvec2 cursorPosition;
	static bool cursorVisible = true;
	static april::Timer globalTimer;
	static Win32Window* instance;
	*/

	static gvec2 cursorPosition;
	static april::Timer globalTimer;

	AndroidJNIWindow::AndroidJNIWindow(int w, int h, bool fullscreen, chstr title) : Window()
	{
		if (april::rendersys != NULL)
		{
			april::log("Creating Android JNI Windowsystem");
		}
		mWidth = w;
		mHeight = h;
		//mRunning = true;
		mActive = true;
		mFullscreen = fullscreen;
		//mTouchEnabled = false;
		mTitle = title;
	}
	
	AndroidJNIWindow::~AndroidJNIWindow()
	{
		//log("Destroying Android JNI Windowsystem");
	}


	void AndroidJNIWindow::enterMainLoop()
	{
		float time = globalTimer.getTime();
		float t;
		//bool cVisible = cursorVisible;
		//POINT w32_cursorPosition;
		float k = 0.0f;
		//while (mRunning)
		{
			// mouse position
			//GetCursorPos(&w32_cursorPosition);
			//ScreenToClient(hWnd, &w32_cursorPosition);
			//cursorPosition.set((float)w32_cursorPosition.x, (float)w32_cursorPosition.y);
			//*
			doEvents();
			t = globalTimer.getTime();
			if (t == time)
			{
				return; // don't redraw frames which won't change
			}
			k = (t - time) / 1000.0f;
			if (k > 0.5f)
			{
				k = 0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
			}

			time = t;
			if (!mActive)
			{
				k = 0;
				for (int i = 0; i < 5; i++)
				{
					doEvents();
					hthread::sleep(40);
					//Sleep(40);
				}
			}
			//*/
			// rendering
			if (mUpdateCallback != NULL)
			{
				(*mUpdateCallback)(k);
			}
			rendersys->presentFrame();
		}
	}
	
	void AndroidJNIWindow::terminateMainLoop()
	{
	}

	void AndroidJNIWindow::destroyWindow()
	{
	}

	void AndroidJNIWindow::showSystemCursor(bool visible)
	{
	}

	bool AndroidJNIWindow::isSystemCursorShown()
	{
		return false;
	}

	int AndroidJNIWindow::getWidth()
	{
		return mWidth;
	}

	int AndroidJNIWindow::getHeight()
	{
		return mHeight;
	}

	void AndroidJNIWindow::setWindowTitle(chstr title)
	{
	}

	gvec2 AndroidJNIWindow::getCursorPosition()
	{
		return cursorPosition;
	}

	void AndroidJNIWindow::presentFrame()
	{
	}

	void* AndroidJNIWindow::getIDFromBackend()
	{
		return NULL;
	}

	void AndroidJNIWindow::doEvents()
	{
	}

	Window::DeviceType AndroidJNIWindow::getDeviceType()
	{
		return Window::DEVICE_ANDROID_PHONE;
	}

}
#endif