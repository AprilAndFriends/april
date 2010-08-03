/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#include <GLUT/glut.h>
#include "GLUTWindow.h"
#include "Keys.h"

namespace April
{
#ifdef _WIN32
	HWND hWnd;
#else
#include <sys/time.h>
	unsigned GetTickCount()
	{
		struct timeval tv;
		if(gettimeofday(&tv, NULL) != 0)
			return 0;
		
		return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	}
	
#endif
	
	
	GLUTWindow* GLUTWindow::_instance;
	
	GLUTWindow::GLUTWindow(int w, int h, bool fullscreen, chstr title)
	{
		
		const char *argv[] = {"program"};
		int argc=1;
		glutInit(&argc,(char**) argv);
		glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
		int _w=glutGet(GLUT_SCREEN_WIDTH);
		int _h=glutGet(GLUT_SCREEN_HEIGHT);
		glutInitWindowPosition(_w/2-w/2,_h/2-h/2);
		glutInitWindowSize(w,h);
		glutCreateWindow(title.c_str());
#ifdef _WIN32
		hWnd = FindWindow("GLUT", title.c_str());
		SetFocus(hWnd);
#endif
		if (fullscreen) glutFullScreen();
		
		glutDisplayFunc(		_handleDisplayAndUpdate);
		glutIdleFunc(			_handleDisplayAndUpdate);
		
		glutMouseFunc(			_handleMouseButton);
		glutMotionFunc(			_handleMouseMove);
		glutPassiveMotionFunc(	_handleMouseMove);
		
		glutKeyboardFunc(		_handleKeyDown);
		glutKeyboardUpFunc(		_handleKeyUp);
		glutSpecialFunc(		_handleKeySpecial);
		
		_instance = this;
	}
	
	//////////////////////
	// implementations
	//////////////////////
	
	void GLUTWindow::enterMainLoop()
	{
		glutMainLoop();
	}
	
	void GLUTWindow::terminateMainLoop()
	{
		//destroy(); // TODO
		exit(0);
	}
	
	
	void GLUTWindow::presentFrame()
	{
		glutSwapBuffers();
	}
	
	
	void GLUTWindow::showSystemCursor(bool b)
	{
		if (b) glutSetCursor(GLUT_CURSOR_INHERIT);
		else   glutSetCursor(GLUT_CURSOR_NONE);
	}
	
	bool GLUTWindow::isSystemCursorShown()
	{
		int cursor=glutGet(GLUT_WINDOW_CURSOR);
		return (cursor == GLUT_CURSOR_NONE) ? 0 : 1;
	}
	
	
	void GLUTWindow::setWindowTitle(chstr title)
	{
		glutSetWindowTitle(title.c_str());
	}
	
	gtypes::Vector2 GLUTWindow::getCursorPos()
	{
		return gtypes::Vector2(mCursorX,mCursorY);
	}
	
	int GLUTWindow::getWindowWidth()
	{
		return glutGet(GLUT_WINDOW_WIDTH);
	}
	
	int GLUTWindow::getWindowHeight()
	{
		return glutGet(GLUT_WINDOW_HEIGHT);
	}
	
	
	
	
	
	/////////////////////////
	// overrides
	/////////////////////////
	
	void GLUTWindow::handleKeyEvent(Window::KeyEventType type, unsigned int keycode, unsigned int unicode)
	{
		if (keycode == 9) 
			keycode=AK_TAB;
		else 
			if (keycode >= GLUT_KEY_F1 && keycode <= GLUT_KEY_F12)  // function keys
				keycode+=0x6F;
		
		
		Window::handleKeyEvent(type, (KeySyms)keycode, unicode);
	}
	
	//////////////////////
	// private parts
	//////////////////////

	void GLUTWindow::_handleKeyUp(unsigned char key, int x, int y)
	{
		GLUTWindow::_instance->handleKeyEvent(Window::AKEYEVT_UP, key, key>=' ' ? key : 0);
	}
	
	void GLUTWindow::_handleKeyDown(unsigned char key, int x, int y)
	{
		/*
		if (key == 27) //esc
		{
			rendersys->terminateMainLoop();
		}
		 */
		
		GLUTWindow::_instance->handleKeyEvent(Window::AKEYEVT_DOWN, key, key>=' ' ? key : 0);
	}
	
	void GLUTWindow::_handleKeySpecial(int key, int x, int y)
	{
		GLUTWindow::_instance->handleKeyEvent(Window::AKEYEVT_DOWN, key, 0);
	}
	
	void GLUTWindow::_handleMouseButton(int button, int state, int x,int y)
	{
		GLUTWindow::_instance->mCursorX=x; 
		GLUTWindow::_instance->mCursorY=y;
		if (state == GLUT_DOWN)
			GLUTWindow::_instance->handleMouseEvent(Window::AMOUSEEVT_DOWN, x, y, (Window::MouseButton)button);
		else
			GLUTWindow::_instance->handleMouseEvent(Window::AMOUSEEVT_UP, x, y, (Window::MouseButton)button);
	}
	
	void GLUTWindow::_handleMouseMove(int x,int y)
	{
		
		GLUTWindow::_instance->mCursorX=x; 
		GLUTWindow::_instance->mCursorY=y;
		
		// TODO last argument should be button, as remembered from _handleMouseButton
		// this is because glutMotionFunc() also calls it, and we may want to know which button is active.
		
		GLUTWindow::_instance->handleMouseEvent(Window::AMOUSEEVT_MOVE, x, y, (Window::MouseButton)0);
		
	}
	
	void GLUTWindow::_handleDisplayAndUpdate()
	{
		static unsigned int x=GetTickCount();
		float k=(GetTickCount()-x)/1000.0f;
		x=GetTickCount();
		
		GLUTWindow::_instance->performUpdate(k);
		GLUTWindow::_instance->presentFrame();
	}
	

	

}
