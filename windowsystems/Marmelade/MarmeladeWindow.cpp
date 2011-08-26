/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica                                                       *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef HAVE_MARMELADE


#include <ctype.h> // tolower()

#include "MarmeladeWindow.h"
#include "Keys.h"
#include "Marmelade_RenderSystem.h"

#define MAX_CONFIGS 30

extern "C" int gAprilShouldInvokeQuitCallback;

namespace april
{
	InputEvent::InputEvent(InputEventType type)
	{
		mType = type;
	}

	InputKeyboardEvent::InputKeyboardEvent(KeySym key, InputEventState state)
	{
		mType = AIET_KEYBOARD_EVENT;
		mSym = key;
		mState = state;
	}

	InputMouseEvent::InputMouseEvent(KeySym button, InputEventState state, int32 x, int32 y)
	{
		mType = AIET_MOUSE_EVENT;
		mSym = button;
		mState = state;
		mX = x;
		mY = y;
	}

	InputMouseMotionEvent::InputMouseMotionEvent(int32 x, int32 y)
	{
		mType = AIET_MOUSE_MOTION_EVENT;
		mX = x;
		mY = y;
	}

	InputTouchEvent::InputTouchEvent(int32 id, InputEventState state, int32 x, int32 y)
	{
		mType = AIET_TOUCH_EVENT;
		mState = state;
		mID = id;
		mX = x;
		mY = y;
	}

	InputTouchMotionEvent::InputTouchMotionEvent(int32 id, int32 x, int32 y)
	{
		mType = AIET_TOUCH_MOTION_EVENT;
		mID = id;
		mX = x;
		mY = y;
	}
	
	MarmeladeWindow::MarmeladeWindow(int w, int h, bool fullscreen, chstr title)
	{
		// w,h and fullscreen ignored since this is a mobile device build
		//april::log("Creating Marmelade Windowsystem");
		fprintf(stderr, "Creating Marmelade Windowsystem\n");
		
		// set the name of the window to title.c_str() // ignored for mobile devices

		mTitle = title;
        mCursorInside = true; // ignored for mobile devices
		mFullscreen = fullscreen; // ignored for mobile devices

		if(_eglInit())
		{
			fprintf(stderr, "Error initializing egl! Terminating app!\n");
		}

		EGLSurface g_EGLSurface = NULL;
		EGLDisplay g_EGLDisplay = NULL;
		EGLDisplay g_EGLContext = NULL;

		// we are not running yet
		mRunning = false;
		// cursor is visible by default, but this is ignored for mobile devices
		mCursorVisible = true;

		/* set handler for pause ie. when phone rings or runs out of battery */
		s3eGLRegister (S3E_GL_SUSPEND, pauseHandler, NULL);
		/* set handler for resume after pause */
		s3eGLRegister (S3E_GL_RESUME, unpauseHandler, NULL);
		/* set handler for keyboard button events */
		s3eKeyboardRegister(S3E_KEYBOARD_KEY_EVENT, keyboardHandler, NULL);
		/* set handler for mouse click events */
		s3ePointerRegister(S3E_POINTER_BUTTON_EVENT, mouseClickHandler, NULL);
		/* set handler for mouse motion events */
		s3ePointerRegister(S3E_POINTER_MOTION_EVENT, mouseMotionHandler, NULL);
		/* set handler for touch tap events */
		s3ePointerRegister(S3E_POINTER_TOUCH_EVENT, touchTapHandler, NULL);
		/* set handler for touch motion events */
		s3ePointerRegister(S3E_POINTER_TOUCH_MOTION_EVENT, touchMotionHandler, NULL);

	}

	bool MarmeladeWindow::_eglInit()
	{
		mScreen = (uint16 *)s3eSurfacePtr();

		// init EGL
		EGLint major;
		EGLint minor;
		EGLint numFound = 0;
		EGLConfig configList[MAX_CONFIGS];

		int version = s3eGLGetInt(S3E_GL_VERSION);
		if ((version >> 8) != 1)
		{
			s3eDebugTracePrintf("reported GL version is %d", version);
			s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "This app required GLES v1.x");
			return 1;
		}

		g_EGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (!g_EGLDisplay)
		{
			s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "eglGetDisplay failed");
			return 1;
		}

		EGLBoolean res = eglInitialize(g_EGLDisplay, &major, &minor);
		if (!res)
		{
			s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "eglInitialize failed");
			return 1;
		}

		if (!eglGetConfigs(g_EGLDisplay, configList, MAX_CONFIGS, &numFound))
		{
			s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "eglGetConfigs failed");
			return 1;
		}

		int config = -1;
		printf("found %d configs\n", numFound);
		for (int i = 0; i < numFound; i++)
		{
			EGLint surfacetype = 0;
			EGLint depth = 0;
			eglGetConfigAttrib(g_EGLDisplay, configList[i], EGL_SURFACE_TYPE, &surfacetype);
			eglGetConfigAttrib(g_EGLDisplay, configList[i], EGL_DEPTH_SIZE, &depth);
			printf("config %d: depth=%d\n", i, depth);
			if (surfacetype & EGL_WINDOW_BIT && depth)
			{
				config = i;
				break;
			}
		}

		if (config == -1)
		{
			s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "No suitable config found.  Trying random config");
			config = 0;
		}

		 printf("using config %d\n", config);

		g_EGLContext = eglCreateContext(g_EGLDisplay, configList[config], NULL, NULL);
		if (!g_EGLContext)
		{
			s3eDebugErrorPrintf("eglCreateContext failed: %#x", eglGetError());
			return 1;
		}

		void* nativeWindow = s3eGLGetNativeWindow();
		g_EGLSurface = eglCreateWindowSurface(g_EGLDisplay, configList[config], nativeWindow, NULL);
		if (!g_EGLSurface)
		{
			s3eDebugErrorPrintf("eglCreateWindowSurface failed: %#x", eglGetError());
			return 1;
		}

		res = eglMakeCurrent(g_EGLDisplay, g_EGLSurface, g_EGLSurface, g_EGLContext);
		if (!res)
		{
			s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "eglMakeCurrent failed");
			return 1;
		}
    
		int w = s3eSurfaceGetInt(S3E_SURFACE_WIDTH);
		int h = s3eSurfaceGetInt(S3E_SURFACE_HEIGHT);

		glViewport( 0, 0, w, h );
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity( );

		glOrthof( -w/2, w/2, -h/2, h/2, -100.0, 100.0 );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity( );

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glShadeModel(GL_SMOOTH);
		
		glClear(GL_COLOR_BUFFER_BIT);
		presentFrame();
		if (mScreen == NULL)
		{
			april::log("Requested display mode could not be provided");
			exit(0);
		}

		return 0;
	}

	void MarmeladeWindow::_eglTerminate()
	{
		eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if(g_EGLContext && g_EGLDisplay)
		{
			eglDestroyContext(g_EGLDisplay, g_EGLContext);
			g_EGLContext = NULL;
		}
		if(g_EGLSurface && g_EGLDisplay)
		{
			eglDestroySurface(g_EGLDisplay, g_EGLSurface);
			g_EGLSurface = NULL;
		}

		eglTerminate(g_EGLDisplay);
	}
	
	//////////////////////
	// implementations
	//////////////////////

	void* MarmeladeWindow::getIDFromBackend()
	{
		return NULL;
	}
	
	void MarmeladeWindow::enterMainLoop()
	{
		mRunning = true;	
		while (mRunning)
		{
			//check if we should quit...
			if (gAprilShouldInvokeQuitCallback)
			{
				uint32 event;
				/*
				event == quit
				dodaj event u event queue
				*/
				gAprilShouldInvokeQuitCallback = 0;
			}
			//moramo pohvatat sve eventove
			doEvents();
			update();
		}
				
	}
	
	void MarmeladeWindow::doEvents()
	{
		/* parse events and do stuff */
		for(int i = 0; i < mEvents.size(); ++i)
		{
			if(mEvents[i]->mType == AIET_MOUSE_MOTION_EVENT)
			{
				float x,y;
				x = (float)((InputMouseMotionEvent*)mEvents[i])->mX;
				y = (float)((InputMouseMotionEvent*)mEvents[i])->mY;
				handleMouseEvent(AMOUSEEVT_MOVE, x, y, AMOUSEBTN_NONE);
			}
			else if(mEvents[i]->mType == AIET_MOUSE_EVENT)
			{
				MouseButton btn;
				if(((InputMouseEvent*)mEvents[i])->mState == AIES_STATE_DOWN)
				{
					if(((InputMouseEvent*)mEvents[i])->mSym == AK_LBUTTON) btn = AMOUSEBTN_LEFT;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_LBUTTON) btn = AMOUSEBTN_LEFT;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_RBUTTON) btn = AMOUSEBTN_RIGHT;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_MBUTTON) btn = AMOUSEBTN_MIDDLE;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_WHEELDN) btn = AMOUSEBTN_WHEELDN;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_WHEELUP) btn = AMOUSEBTN_WHEELUP;
					else btn = AMOUSEBTN_NONE;

					handleMouseEvent(AMOUSEEVT_DOWN, ((InputMouseEvent*)mEvents[i])->mX, ((InputMouseEvent*)mEvents[i])->mY, btn);
				}

				else if(((InputMouseEvent*)mEvents[i])->mState == AIES_STATE_UP)
				{
					if(((InputMouseEvent*)mEvents[i])->mSym == AK_LBUTTON) btn = AMOUSEBTN_LEFT;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_LBUTTON) btn = AMOUSEBTN_LEFT;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_RBUTTON) btn = AMOUSEBTN_RIGHT;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_MBUTTON) btn = AMOUSEBTN_MIDDLE;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_WHEELDN) btn = AMOUSEBTN_WHEELDN;
					else if(((InputMouseEvent*)mEvents[i])->mSym == AK_WHEELUP) btn = AMOUSEBTN_WHEELUP;
					else btn = AMOUSEBTN_NONE;

					handleMouseEvent(AMOUSEEVT_UP, ((InputMouseEvent*)mEvents[i])->mX, ((InputMouseEvent*)mEvents[i])->mY, btn);
				}
			}

			else if(mEvents[i]->mType == AIET_KEYBOARD_EVENT)
			{
				if(((InputKeyboardEvent*)mEvents[i])->mState == AIES_STATE_DOWN)
				{
					handleKeyEvent(AKEYEVT_DOWN, ((InputKeyboardEvent*)mEvents[i])->mSym, 0);
				}
				else if(((InputKeyboardEvent*)mEvents[i])->mState == AIES_STATE_UP)
				{
					handleKeyEvent(AKEYEVT_UP, ((InputKeyboardEvent*)mEvents[i])->mSym, 0);
				}
			}

			else if(mEvents[i]->mType == AIET_TOUCH_EVENT)
			{
				/* TODO */
			}

			else if(mEvents[i]->mType == AIET_TOUCH_MOTION_EVENT)
			{
				/* TODO */
			}

			else if(mEvents[i]->mType == AIET_UNKNOWN)
			{
#ifdef _DEBUG_OUTPUT
				fprintf(stderr, "Unknown input happened!\n");
#endif
			}
		}


		/* deallocate input events and clear the event queue*/
		for(int i = 0; i < mEvents.size(); ++i)
		{
			delete mEvents[i];
		}
		mEvents.clear();
	}
	
	MarmeladeWindow::~MarmeladeWindow()
	{
		// quit + cleanup
	}
	
	void MarmeladeWindow::terminateMainLoop()
	{
		mRunning = false;
	}

	void MarmeladeWindow::setRunning(bool running)
	{
		mRunning = running;
	}
	
	void MarmeladeWindow::destroyWindow()
	{
		_eglTerminate();
	}
	
	void MarmeladeWindow::presentFrame()
	{
		eglSwapBuffers(g_EGLDisplay, g_EGLSurface);
	}
	
	void MarmeladeWindow::showSystemCursor(bool b)
	{
		mCursorVisible = b;
	}
	
	bool MarmeladeWindow::isSystemCursorShown()
	{
		return mCursorVisible;
	}
	
	void MarmeladeWindow::setWindowTitle(chstr title)
	{
		// postavi naziv prozora na title.c_str()
		mTitle = title;
	}
	
	gvec2 MarmeladeWindow::getCursorPosition()
	{
		return gvec2(mCursorX,mCursorY);
	}
	
	int MarmeladeWindow::getWidth()
	{
		return s3eSurfaceGetInt(S3E_SURFACE_WIDTH);
	}
	
	int MarmeladeWindow::getHeight()
	{
		return s3eSurfaceGetInt(S3E_SURFACE_HEIGHT);
	}
	
	bool MarmeladeWindow::isCursorInside()
	{
		return mCursorInside;
	}

	//////////////////////
	// callbacks
	//////////////////////

	int32 MarmeladeWindow::keyboardHandler(void *sys, void *args)
	{
		s3eKeyboardEvent *evt = (s3eKeyboardEvent *)sys;
#ifdef _DEBUG_OUTPUT
		fprintf(stderr, "Key [%d] State [%d]\n", evt->m_Key, evt->m_Pressed);
#endif

		InputEventState state;
		switch(evt->m_Pressed)
		{
		case S3E_KEY_STATE_DOWN:
			state = AIES_STATE_DOWN;
			break;
		case S3E_KEY_STATE_PRESSED:
			state = AIES_STATE_PRESSED;
			break;
		case S3E_KEY_STATE_RELEASED:
			state = AIES_STATE_RELEASED;
			break;
		default:
			state = AIES_STATE_UP;
			break;
		}

		InputKeyboardEvent *mevent = new InputKeyboardEvent((KeySym)evt->m_Key, state);
		((MarmeladeWindow*)april::rendersys->getWindow())->pushEvent((InputEvent*)mevent);

		return 0;
	}

	int32 MarmeladeWindow::mouseClickHandler(void *sys, void *args)
	{
		s3ePointerEvent *evt = (s3ePointerEvent *)sys;
#ifdef _DEBUG_OUTPUT
		fprintf(stderr, "Button [%d] State [%d] @ [%d, %d]\n", evt->m_Button, evt->m_Pressed, evt->m_x, evt->m_y);
#endif

		InputEventState state;
		KeySym button;

		switch(evt->m_Pressed)
		{
		case S3E_POINTER_STATE_DOWN:
			state = AIES_STATE_DOWN;
			break;
		case S3E_POINTER_STATE_PRESSED:
			state = AIES_STATE_PRESSED;
			break;
		case S3E_POINTER_STATE_RELEASED:
			state = AIES_STATE_RELEASED;
			break;
		case S3E_POINTER_STATE_UP:
			state = AIES_STATE_UP;
			break;
		default:
			state = AIES_STATE_UNKNOWN;
			break;
		}

		switch(evt->m_Button)
		{
		case S3E_POINTER_BUTTON_LEFTMOUSE:
			button = AK_LBUTTON;
			break;
		case S3E_POINTER_BUTTON_RIGHTMOUSE:
			button = AK_RBUTTON;
			break;
		case S3E_POINTER_BUTTON_MIDDLEMOUSE:
			button = AK_MBUTTON;
			break;
		case S3E_POINTER_BUTTON_MOUSEWHEELDOWN:
			button = AK_WHEELDN;
			break;
		case S3E_POINTER_BUTTON_MOUSEWHEELUP:
			button = AK_WHEELUP;
			break;
		default:
			button = AK_UNKNOWN;
			break;
		}

		InputMouseEvent *mevent = new InputMouseEvent(button, state, evt->m_x, evt->m_y);
		((MarmeladeWindow*)april::rendersys->getWindow())->pushEvent((InputEvent*)mevent);

		return 0;
	}

	int32 MarmeladeWindow::mouseMotionHandler(void *sys, void *args)
	{
		s3ePointerMotionEvent *evt = (s3ePointerMotionEvent *)sys;
#ifdef _DEBUG_OUTPUT
		fprintf(stderr, "Mouse Motion [%d,%d]\n", evt->m_x, evt->m_y);
#endif

		InputMouseMotionEvent *mevent = new InputMouseMotionEvent(evt->m_x, evt->m_y);
		((MarmeladeWindow*)april::rendersys->getWindow())->pushEvent((InputEvent*)mevent);

		return 0;
	}

	int32 MarmeladeWindow::touchTapHandler(void *sys, void *args)
	{

		s3ePointerTouchEvent *evt = (s3ePointerTouchEvent *)sys;
#ifdef _DEBUG_OUTPUT
		fprintf(stderr, "Touch ID[%d] State [%d] @ [%d, %d]\n", evt->m_TouchID, evt->m_Pressed, evt->m_x, evt->m_y);
#endif

		InputEventState state;
		switch(evt->m_Pressed)
		{
		case S3E_POINTER_STATE_DOWN:
			state = AIES_STATE_DOWN;
			break;
		case S3E_POINTER_STATE_PRESSED:
			state = AIES_STATE_PRESSED;
			break;
		case S3E_POINTER_STATE_RELEASED:
			state = AIES_STATE_RELEASED;
			break;
		case S3E_POINTER_STATE_UP:
			state = AIES_STATE_UP;
			break;
		default:
			state = AIES_STATE_UNKNOWN;
			break;
		}

		InputTouchEvent *mevent = new InputTouchEvent(evt->m_TouchID, state, evt->m_x, evt->m_y);
		((MarmeladeWindow*)april::rendersys->getWindow())->pushEvent((InputEvent*)mevent);

		return 0;
	}

	int32 MarmeladeWindow::touchMotionHandler(void *sys, void *args)
	{
		s3ePointerTouchMotionEvent *evt = (s3ePointerTouchMotionEvent *)sys;
#ifdef _DEBUG_OUTPUT
		fprintf(stderr, "Touch ID[%d] Motion [%d,%d]\n", evt->m_TouchID, evt->m_x, evt->m_y);
#endif

		InputTouchMotionEvent *mevent = new InputTouchMotionEvent(evt->m_TouchID, evt->m_x, evt->m_y);
		((MarmeladeWindow*)april::rendersys->getWindow())->pushEvent((InputEvent*)mevent);

		return 0;
	}

	int32 MarmeladeWindow::pauseHandler(void *sys, void *args)
	{
		MarmeladeWindow *win = ((MarmeladeWindow*)april::rendersys->getWindow());

		/* have to call saveStateFunction() !!! */

		win->_eglTerminate();
		win->setRunning(false);
		return 0;
	}

	int32 MarmeladeWindow::unpauseHandler(void *sys, void *args)
	{
		MarmeladeWindow *win = ((MarmeladeWindow*)april::rendersys->getWindow());

		if(win->_eglInit())
		{
			fprintf(stderr, "Error on resuming and recreating egl context! Terminating app!\n");
			exit(0);
		}

		/* have to call resumeStateFunction() !!! */

		win->setRunning(true);
		return 0;
	}

	void MarmeladeWindow::pushEvent(InputEvent *evt)
	{
		mEvents += evt;
	}

	//////////////////////
	// private parts
	//////////////////////	
	
	void MarmeladeWindow::update()
	{
		static unsigned int x = (unsigned int)s3eTimerGetMs();
		float k = ((int)s3eTimerGetMs() - x) / 1000.0f;
		x = (unsigned int)s3eTimerGetMs();
		
		april::rendersys->getWindow()->performUpdate(k);
		april::rendersys->presentFrame();

		// dajemo vremena androidu da obavi svoje stvari
		s3eDeviceYield(0);
		s3eDeviceYield(0);
	}

}

#endif
