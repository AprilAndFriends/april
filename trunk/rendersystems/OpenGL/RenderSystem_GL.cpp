/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com),                                 *
                   Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef _OPENGL

#include "RenderSystem_GL.h"
#include "Keys.h"
#include "ImageSource.h"
#ifdef IPHONE_PLATFORM
#include <OpenGLES/ES1/gl.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else // __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif // __APPLE__

#include <gtypes/Vector2.h>

April::Timer globalTimer;

namespace April
{
	extern void (*g_logFunction)(chstr);
	static int windowId;
#ifdef _WIN32
	static HWND hWnd;
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

	float cursor_x=0,cursor_y=0;

	unsigned int platformLoadGLTexture(const char* name,int* w,int* h)
	{
		GLuint texid;
		ImageSource* img=loadImage(name);
		if (!img) return 0;
		*w=img->w; *h=img->h;
		glGenTextures(1, &texid);
		glBindTexture(GL_TEXTURE_2D, texid);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, img->bpp, img->w,img->h, 0, img->format, GL_UNSIGNED_BYTE,img->data);
		delete img;


		return texid;
	}
	
	void win_mat_invert()
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		float mat[16],t;
		glGetFloatv(GL_PROJECTION_MATRIX,mat);
		t=mat[1]; mat[1]=mat[0]; mat[0]=t;
		t=mat[5]; mat[5]=-mat[4]; mat[4]=t;
		mat[13]=-mat[13];
		glLoadMatrixf(mat);
	}
	#endif
	// translation from abstract render ops to gl's render ops
	int gl_render_ops[]=
	{
		0,
		GL_TRIANGLES,      // ROP_TRIANGLE_LIST
		GL_TRIANGLE_STRIP, // ROP_TRIANGLE_STRIP
		GL_TRIANGLE_FAN,   // ROP_TRIANGLE_FAN
		GL_LINES,          // ROP_LINE_LIST
		GL_LINE_STRIP,     // ROP_LINE_STRIP
	};



	GLTexture::GLTexture(chstr filename,bool dynamic)
	{
		mFilename=filename;
		mDynamic=dynamic;
		mTexId=0; mWidth=mHeight=0;
	}

	GLTexture::GLTexture(unsigned char* rgba,int w,int h)
	{
		mWidth=w; mHeight=h;
		mDynamic=0;
		mFilename="UserTexture";
		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, w,h, 0, GL_RGBA, GL_UNSIGNED_BYTE,rgba);

	}

	GLTexture::~GLTexture()
	{
		unload();
	}

	bool GLTexture::load()
	{
		mUnusedTimer=0;
		if (mTexId) return 1;
		rendersys->logMessage("loading GL texture '"+mFilename+"'");
		mTexId=platformLoadGLTexture(mFilename.c_str(),&mWidth,&mHeight);
		if (!mTexId)
		{
			rendersys->logMessage("Failed to load texture: "+mFilename);
			return 0;
		}
		
		for (Texture** it=mDynamicLinks.iter();it;it=mDynamicLinks.next())
			((GLTexture*)(*it))->load();

		return 1;
	}

	bool GLTexture::isLoaded()
	{
		return mTexId != 0;
	}

	void GLTexture::unload()
	{
		if (mTexId)
		{
			rendersys->logMessage("unloading GL texture '"+mFilename+"'");
			glDeleteTextures(1, &mTexId);
			mTexId=0;
		}
	}

	int GLTexture::getSizeInBytes()
	{
		return mWidth*mHeight*3;
	}


	GLRenderSystem::GLRenderSystem(int w,int h) :
		mTexCoordsEnabled(0), mColorEnabled(0)
	{		
		glViewport(0,0,w,h);
		glClearColor(0,0,0,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		mTextureFilter=Linear;
        //glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_GREATER);
        //glClearDepth(1.0f);
        
		//glEnable(GL_CULL_FACE);
	}

	GLRenderSystem::~GLRenderSystem()
	{
		rendersys->logMessage("Destroying OpenGL Rendersystem");
	}

	hstr GLRenderSystem::getName()
	{
		return "OpenGL";
	}
	
	float GLRenderSystem::getPixelOffset()
	{
		return 0.0f;
	}

	Texture* GLRenderSystem::loadTexture(chstr filename,bool dynamic)
	{
		if (mDynamicLoading) dynamic=1;
		if (dynamic) rendersys->logMessage("creating dynamic GL texture '"+filename+"'");
		GLTexture* t=new GLTexture(filename,dynamic);
		if (!dynamic)
		{
			if (!t->load())
			{
				delete t;
				return 0;
			}
		}
		return t;
	}

	Texture* GLRenderSystem::createTextureFromMemory(unsigned char* rgba,int w,int h)
	{
		
		rendersys->logMessage("creating user-defined GL texture");
		GLTexture* t=new GLTexture(rgba,w,h);
		return t;
	}
	
	Texture* GLRenderSystem::createEmptyTexture(int w,int h,TextureFormat fmt,TextureType type)
	{
		return 0; //todo
	}

	void GLRenderSystem::setTexture(Texture* t)
	{
		GLTexture* glt=(GLTexture*) t;
		if (t == 0) glBindTexture(GL_TEXTURE_2D,0);
		else
		{
			if (glt->mTexId == 0 && glt->isDynamic())
			{
				glt->load();
			}
			glt->_resetUnusedTimer();
			glBindTexture(GL_TEXTURE_2D,glt->mTexId);
			
			TextureFilter filter=t->getTextureFilter();
			if (filter != mTextureFilter)
				setTextureFilter(filter);
		}
	}

	void GLRenderSystem::clear(bool color,bool depth)
	{
		GLbitfield mask=0;
		if (color) mask |= GL_COLOR_BUFFER_BIT;
		if (depth) mask |= GL_DEPTH_BUFFER_BIT;
		glClear(mask);
		
	}

	void GLRenderSystem::_setModelviewMatrix(const gtypes::Matrix4& matrix)
	{
		glLoadMatrixf(matrix.mat);
	}

	void GLRenderSystem::_setProjectionMatrix(const gtypes::Matrix4& matrix)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(matrix.mat);
		glMatrixMode(GL_MODELVIEW);
	}

	void GLRenderSystem::setBlendMode(BlendMode mode)
	{
		if (mode == ALPHA_BLEND || mode == DEFAULT)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (mode == ADD)
		{
			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		}
	}
	
	void GLRenderSystem::setTextureFilter(TextureFilter filter)
	{
		if (filter == Linear)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else if (filter == Nearest)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else
			logMessage("trying to set unsupported texture filter!");
		mTextureFilter=filter;
	}

	void GLRenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices)
	{
		if (!mTexCoordsEnabled) { glEnableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=true; }
		if (mColorEnabled) { glColor4f(1,1,1,mAlphaMultiplier); glDisableClientState(GL_COLOR_ARRAY); mColorEnabled=false; }

		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), (char*) v+3*sizeof(float));

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);

	}

	void GLRenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)
	{
		if (!mTexCoordsEnabled) { glEnableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=true; }
		if (!mColorEnabled) { mColorEnabled=true; glDisableClientState(GL_COLOR_ARRAY); }    
		glColor4f(r,g,b,a*mAlphaMultiplier);
		
		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), (char*) v+3*sizeof(float));


		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void GLRenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices)
	{
		if (mTexCoordsEnabled) { glBindTexture(GL_TEXTURE_2D,0); glDisableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=false; }
		if (mColorEnabled) { glColor4f(1,1,1,mAlphaMultiplier); glDisableClientState(GL_COLOR_ARRAY); mColorEnabled=false; }

		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void GLRenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)
	{
		if (mTexCoordsEnabled) { glBindTexture(GL_TEXTURE_2D,0); glDisableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=false; }
		if (!mColorEnabled) { mColorEnabled=true; glDisableClientState(GL_COLOR_ARRAY);}

		glColor4f(r,g,b,a*mAlphaMultiplier);
		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	int GLRenderSystem::getWindowWidth()
	{
		return glutGet(GLUT_WINDOW_WIDTH);
	}
	
	int GLRenderSystem::getWindowHeight()
	{
		return glutGet(GLUT_WINDOW_HEIGHT);
	}

	void GLRenderSystem::render(RenderOp renderOp,ColoredVertex* v,int nVertices)
	{
		if (mTexCoordsEnabled) { glBindTexture(GL_TEXTURE_2D,0); glDisableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=false; }
		if (!mColorEnabled) { mColorEnabled=true; }
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(ColoredVertex), v);
		glColor4f(1,1,1,mAlphaMultiplier);
		glColorPointer(4, GL_UNSIGNED_BYTE,sizeof(ColoredVertex), (char*) v+3*sizeof(float));

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void GLRenderSystem::render(RenderOp renderOp,ColoredTexturedVertex* v,int nVertices)
	{
		// TODO
	}
	
	void GLRenderSystem::setRenderTarget(Texture* source)
	{
		// TODO
	}
	
	void GLRenderSystem::beginFrame()
	{
		// TODO
		}

		void GLRenderSystem::setAlphaMultiplier(float value)
		{
			mAlphaMultiplier=value;
			glColor4f(1,1,1,value);
		}

		void GLRenderSystem::setWindowTitle(chstr title)
		{
			glutSetWindowTitle(title.c_str());
		}
	
	gtypes::Vector2 GLRenderSystem::getCursorPos()
	{
		return gtypes::Vector2(cursor_x,cursor_y);
	}

	void GLRenderSystem::showSystemCursor(bool b)
	{
		if (b) glutSetCursor(GLUT_CURSOR_INHERIT);
		else   glutSetCursor(GLUT_CURSOR_NONE);
	}
	
	bool GLRenderSystem::isSystemCursorShown()
	{
		int cursor=glutGet(GLUT_WINDOW_CURSOR);
		return (cursor == GLUT_CURSOR_NONE) ? 0 : 1;
	}

	void GLRenderSystem::presentFrame()
	{
		glutSwapBuffers();
	}
	
	void GLRenderSystem::terminateMainLoop()
	{
		exit(0);
	}
	
	void GLRenderSystem::enterMainLoop()
	{
		glutMainLoop();
	}

	bool GLRenderSystem::triggerUpdate(float time_increase)
	{
		return mUpdateCallback(time_increase);
		return 1;
	}
	
	bool GLRenderSystem::triggerKeyEvent(bool down,unsigned int keycode)
	{
		if (keycode == 9) keycode=AK_TAB;
		else if (keycode >= 1 && keycode <= 12) keycode+=0x6F; // function keys
		
		if (down) { if (mKeyDownCallback) mKeyDownCallback(keycode); }
		else      { if (mKeyUpCallback)   mKeyUpCallback(keycode); }
		return 1;
	}
	
	bool GLRenderSystem::triggerMouseEvent(int event,float x,float y,int button)
	{
		if      (event == 0) { if (mMouseDownCallback) mMouseDownCallback(x,y,button); }
		else if (event == 1) { if (mMouseUpCallback)   mMouseUpCallback(x,y,button); }
		else                 { if (mMouseMoveCallback) mMouseMoveCallback(x,y); }
		return 1;
	}		


/***************************************************/

	void keyboard_up_handler(unsigned char key, int x, int y)
	{
		((GLRenderSystem*) rendersys)->triggerKeyEvent(0,key);
	}

	void keyboard_handler(unsigned char key, int x, int y)
	{
		if (key == 27) //esc
		{
			rendersys->terminateMainLoop();
			return;
		}
		((GLRenderSystem*) rendersys)->triggerKeyEvent(1,key);
	}

	void special_handler(int key, int x, int y)
	{
		((GLRenderSystem*) rendersys)->triggerKeyEvent(1,key);
	}

	void mouse_click_handler(int button, int state, int x,int y)
	{
		cursor_x=x; cursor_y=y;
		if (state == GLUT_DOWN)
			((GLRenderSystem*) rendersys)->triggerMouseEvent(0,x,y,button);
		else
			((GLRenderSystem*) rendersys)->triggerMouseEvent(1,x,y,button);
	}

	void mouse_move_handler(int x,int y)
	{
		cursor_x=x; cursor_y=y;
		((GLRenderSystem*) rendersys)->triggerMouseEvent(2,x,y,0);
	}

	void quit_handler()
	{
		((GLRenderSystem*) rendersys)->triggerQuitEvent();
	}
	void GLRenderSystem::triggerQuitEvent()
	{
		if(mQuitCallback)
		{
			if(mQuitCallback(true))
			{
				glutDestroyWindow(windowId);
				terminateMainLoop();
			}
		}
	}
	
	void gl_draw()
	{
		static unsigned int x = globalTimer.getTime();
		float k =(globalTimer.getTime()-x)/1000.0f;
		x = globalTimer.getTime();
		if (!((GLRenderSystem*) rendersys)->triggerUpdate(k)) throw "done";
		rendersys->presentFrame();
	}

	void createGLRenderSystem(int w,int h,bool fullscreen,chstr title)
	{
		g_logFunction("Creating OpenGL Rendersystem");
		const char *argv[] = {"program"};
		int argc=1;
		glutInit(&argc,(char**) argv);
		glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
		int _w=glutGet(GLUT_SCREEN_WIDTH);
		int _h=glutGet(GLUT_SCREEN_HEIGHT);
		glutInitWindowPosition(_w/2-w/2,_h/2-h/2);
		glutInitWindowSize(w,h);
		windowId = glutCreateWindow(title.c_str());
#ifdef _WIN32
		hWnd = FindWindow("GLUT", title.c_str());
		SetFocus(hWnd);
#endif
		if (fullscreen) glutFullScreen();
		glEnable(GL_TEXTURE_2D);
		glutDisplayFunc(gl_draw);
		glutMouseFunc(mouse_click_handler);
		glutKeyboardFunc(keyboard_handler);
		glutKeyboardUpFunc(keyboard_up_handler);
		glutMotionFunc(mouse_move_handler);
		glutPassiveMotionFunc(mouse_move_handler);
#if (GLUT_MACOSX_IMPLEMENTATION >= 2)
		glutWMCloseFunc(quit_handler);
#endif
		
		glutSpecialFunc(special_handler);

		glutIdleFunc(gl_draw);

		// DevIL defaults
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		rendersys=new GLRenderSystem(w,h);
	}

}

#endif
