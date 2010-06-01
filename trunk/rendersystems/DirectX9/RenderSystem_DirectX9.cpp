/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef _DIRECTX9

#include "RenderSystem_DirectX9.h"
#include "ImageSource.h"
#include <windows.h>
#include <stdlib.h>
#include <gtypes/Vector2.h>

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

	float cursor_x=0,cursor_y=0;

	unsigned int platformLoadDirectx9Texture(const char* name,int* w,int* h)
	{
		unsigned int texid;
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
	// translation from abstract render ops to gl's render ops
	int gl_render_ops[]=
	{
		0,
		GL_TRIANGLES,      // ROP_TRIANGLE_LIST
		GL_TRIANGLE_STRIP, // ROP_TRIANGLE_STRIP
		GL_TRIANGLE_FAN,   // ROP_TRIANGLE_FAN
		GL_LINES,          // ROP_LINE_LIST
		GL_LINE_STRIP,     // ROP_LINE_STRIP
		GL_LINE_LOOP,      // ROP_LINE_LOOP
	};



	Directx9Texture::Directx9Texture(std::string filename,bool dynamic)
	{
		mFilename=filename;
		mDynamic=dynamic;
		mTexId=0; mWidth=mHeight=0;
	}

	Directx9Texture::Directx9Texture(unsigned char* rgba,int w,int h)
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

	Directx9Texture::~Directx9Texture()
	{
		unload();
	}

	bool Directx9Texture::load()
	{
		mUnusedTimer=0;
		if (mTexId) return 1;
		rendersys->logMessage("loading GL texture '"+mFilename+"'");
		mTexId=platformLoadDirectx9Texture(mFilename.c_str(),&mWidth,&mHeight);
		if (!mTexId)
		{
			rendersys->logMessage("Failed to load texture: "+mFilename);
			return 0;
		}

		std::vector<Texture*>::iterator it=mDynamicLinks.begin();
		for(;it!=mDynamicLinks.end();it++)
			((Directx9Texture*)(*it))->load();

		return 1;
	}

	bool Directx9Texture::isLoaded()
	{
		return mTexId != 0;
	}

	void Directx9Texture::unload()
	{
		if (mTexId)
		{
			rendersys->logMessage("unloading GL texture '"+mFilename+"'");
			glDeleteTextures(1, &mTexId);
			mTexId=0;
		}
	}

	int Directx9Texture::getSizeInBytes()
	{
		return mWidth*mHeight*3;
	}


	DirectX9RenderSystem::DirectX9RenderSystem(int w,int h) :
		mTexCoordsEnabled(0), mColorEnabled(0)
	{
		glViewport(0,0,w,h);
		glClearColor(0,0,0,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glEnableClientState(GL_VERTEX_ARRAY);

		
		glDisable(GL_CULL_FACE);
	}

	DirectX9RenderSystem::~DirectX9RenderSystem()
	{

	}

	std::string DirectX9RenderSystem::getName()
	{
		return "OpenGL Render System";
	}

	Texture* DirectX9RenderSystem::loadTexture(std::string filename,bool dynamic)
	{
		if (mDynamicLoading) dynamic=1;
		if (dynamic) rendersys->logMessage("creating dynamic GL texture '"+filename+"'");
		Directx9Texture* t=new Directx9Texture(filename,dynamic);
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

	Texture* DirectX9RenderSystem::createTextureFromMemory(unsigned char* rgba,int w,int h)
	{
		
		rendersys->logMessage("creating user-defined GL texture");
		Directx9Texture* t=new Directx9Texture(rgba,w,h);
		return t;

	}

	void DirectX9RenderSystem::setTexture(Texture* t)
	{
		Directx9Texture* glt=(Directx9Texture*) t;
		if (t == 0) glBindTexture(GL_TEXTURE_2D,0);
		else
		{
			if (glt->mTexId == 0 && glt->isDynamic())
			{
				glt->load();
			}
			glt->_resetUnusedTimer();
			glBindTexture(GL_TEXTURE_2D,glt->mTexId);
		}
	}

	void DirectX9RenderSystem::clear(bool color,bool depth)
	{
		GLbitfield mask=0;
		if (color) mask |= GL_COLOR_BUFFER_BIT;
		if (depth) mask |= GL_DEPTH_BUFFER_BIT;
		glClear(mask);
		
	}

	void DirectX9RenderSystem::_setModelviewMatrix(const gtypes::Matrix4& matrix)
	{
		glLoadMatrixf(matrix.mat);
	}

	void DirectX9RenderSystem::_setProjectionMatrix(const gtypes::Matrix4& matrix)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(matrix.mat);
		glMatrixMode(GL_MODELVIEW);
	}

	void DirectX9RenderSystem::setBlendMode(BlendMode mode)
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

	void DirectX9RenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices)
	{
		if (!mTexCoordsEnabled) { glEnableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=true; }
		if (mColorEnabled) { glColor4f(1,1,1,mAlphaMultiplier); glDisableClientState(GL_COLOR_ARRAY); mColorEnabled=false; }

		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), (char*) v+3*sizeof(float));

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);

	}

	void DirectX9RenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)
	{
		if (!mTexCoordsEnabled) { glEnableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=true; }
		if (!mColorEnabled) { mColorEnabled=true; glDisableClientState(GL_COLOR_ARRAY); }    
		glColor4f(r,g,b,a*mAlphaMultiplier);
		
		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), (char*) v+3*sizeof(float));


		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices)
	{
		if (mTexCoordsEnabled) { glBindTexture(GL_TEXTURE_2D,0); glDisableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=false; }
		if (mColorEnabled) { glColor4f(1,1,1,mAlphaMultiplier); glDisableClientState(GL_COLOR_ARRAY); mColorEnabled=false; }

		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)
	{
		if (mTexCoordsEnabled) { glBindTexture(GL_TEXTURE_2D,0); glDisableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=false; }
		if (!mColorEnabled) { mColorEnabled=true; glDisableClientState(GL_COLOR_ARRAY);}

		glColor4f(r,g,b,a*mAlphaMultiplier);
		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	int DirectX9RenderSystem::getWindowWidth()
	{
		return glutGet(GLUT_WINDOW_WIDTH);
	}
	
	int DirectX9RenderSystem::getWindowHeight()
	{
		return glutGet(GLUT_WINDOW_HEIGHT);
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,ColoredVertex* v,int nVertices)
	{
		if (mTexCoordsEnabled) { glBindTexture(GL_TEXTURE_2D,0); glDisableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=false; }
		if (!mColorEnabled) { mColorEnabled=true; }
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(ColoredVertex), v);
		glColor4f(1,1,1,mAlphaMultiplier);
		glColorPointer(4, GL_UNSIGNED_BYTE,sizeof(ColoredVertex), (char*) v+3*sizeof(float));

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void DirectX9RenderSystem::setAlphaMultiplier(float value)
	{
		mAlphaMultiplier=value;
		glColor4f(1,1,1,value);
	}

	void DirectX9RenderSystem::setWindowTitle(std::string title)
	{
		glutSetWindowTitle(title.c_str());
	}
	
	gtypes::Vector2 DirectX9RenderSystem::getCursorPos()
	{
		return gtypes::Vector2(cursor_x,cursor_y);
	}

	void DirectX9RenderSystem::showSystemCursor(bool b)
	{
		if (b) glutSetCursor(GLUT_CURSOR_INHERIT);
		else   glutSetCursor(GLUT_CURSOR_NONE);
	}
	
	bool DirectX9RenderSystem::isSystemCursorShown()
	{
		int cursor=glutGet(GLUT_WINDOW_CURSOR);
		return (cursor == GLUT_CURSOR_NONE) ? 0 : 1;
	}

	void DirectX9RenderSystem::presentFrame()
	{
		glutSwapBuffers();
	}
	
	void DirectX9RenderSystem::terminateMainLoop()
	{
		destroy();
		exit(0);
	}
	
	void DirectX9RenderSystem::enterMainLoop()
	{
		glutMainLoop();
	}

	bool DirectX9RenderSystem::triggerUpdate(float time_increase)
	{
		return mUpdateCallback(time_increase);
		return 1;
	}
	
	bool DirectX9RenderSystem::triggerKeyEvent(bool down,unsigned int keycode,unsigned int charcode)
	{
		if (down) { if (mKeyDownCallback) mKeyDownCallback(keycode,charcode); }
		else      { if (mKeyUpCallback)   mKeyUpCallback(keycode,charcode); }
		return 1;
	}
	
	bool DirectX9RenderSystem::triggerMouseEvent(int event,float x,float y,int button)
	{
		if      (event == 0) { if (mMouseDownCallback) mMouseDownCallback(x,y,button); }
		else if (event == 1) { if (mMouseUpCallback)   mMouseUpCallback(x,y,button); }
		else                 { if (mMouseMoveCallback) mMouseMoveCallback(x,y); }
		return 1;
	}


/***************************************************/

	void keyboard_up_handler(unsigned char key, int x, int y)
	{
		((DirectX9RenderSystem*) rendersys)->triggerKeyEvent(0,key,key);
	}

	void keyboard_handler(unsigned char key, int x, int y)
	{
		if (key == 27) //esc
		{
			rendersys->terminateMainLoop();
		}
		((DirectX9RenderSystem*) rendersys)->triggerKeyEvent(1,key,key);
	}

	void special_handler(int key, int x, int y)
	{
		((DirectX9RenderSystem*) rendersys)->triggerKeyEvent(1,key,key);
	}

	void mouse_click_handler(int button, int state, int x,int y)
	{
		cursor_x=x; cursor_y=y;
		if (state == GLUT_DOWN)
			((DirectX9RenderSystem*) rendersys)->triggerMouseEvent(0,x,y,button);
		else
			((DirectX9RenderSystem*) rendersys)->triggerMouseEvent(1,x,y,button);
	}

	void mouse_move_handler(int x,int y)
	{
		cursor_x=x; cursor_y=y;
		((DirectX9RenderSystem*) rendersys)->triggerMouseEvent(2,x,y,0);
	}

	void gl_draw()
	{
		static unsigned int x=GetTickCount();
		float k=(GetTickCount()-x)/1000.0f;
		x=GetTickCount();
		if (!((DirectX9RenderSystem*) rendersys)->triggerUpdate(k)) throw "done";
		rendersys->presentFrame();
	}

	void createDX9RenderSystem(int w,int h,bool fullscreen,std::string title)
	{

	}

}

#endif
