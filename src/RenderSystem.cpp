/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Ivan Vucica                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>
#include <algorithm>

#ifdef __APPLE__
#include <TargetConditionals.h>
#ifndef _OPENGL
#define _OPENGL
#endif
#endif
#ifdef USE_IL
#include <IL/il.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "RenderSystem.h"
#ifndef _OPENGL
#include "DirectX9_RenderSystem.h"
#else
#include "OpenGL_RenderSystem.h"
#endif
#include "ImageSource.h"
#include "Window.h"

namespace april
{
	april::RenderSystem* rendersys;
	harray<hstr> extensions;

	void april_writelog(chstr message)
	{
		printf("%s\n", message.c_str());		
	}
	
	void (*g_logFunction)(chstr) = april_writelog;
	
	void log(chstr message, chstr prefix)
	{
		g_logFunction(prefix + message);
	}
	
/************************************************************************************/
	void PlainVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}
/************************************************************************************/
	RenderSystem::RenderSystem()
	{
		mAlphaMultiplier = 1.0f;
		mDynamicLoading = false;
		mIdleUnloadTime = 0;
		mTextureFilter = Linear;
		mTextureWrapping = true;
	}
	
	RenderSystem::~RenderSystem()
	{
	}

	PlainVertex pVertices[4];
	void RenderSystem::drawColoredQuad(grect rect, Color color)
	{
		pVertices[0].x = rect.x;          pVertices[0].y = rect.y;          pVertices[0].z = 0;
		pVertices[1].x = rect.x + rect.w; pVertices[1].y = rect.y;          pVertices[1].z = 0;
		pVertices[2].x = rect.x;          pVertices[2].y = rect.y + rect.h; pVertices[2].z = 0;
		pVertices[3].x = rect.x + rect.w; pVertices[3].y = rect.y + rect.h; pVertices[3].z = 0;
		
		render(TriangleStrip, pVertices, 4, color);
	}
	
	TexturedVertex tVertices[4];
	void RenderSystem::drawTexturedQuad(grect rect, grect src)
	{
		tVertices[0].x = rect.x;          tVertices[0].y = rect.y;          tVertices[0].z = 0; tVertices[0].u = src.x;         tVertices[0].v = src.y;
		tVertices[1].x = rect.x + rect.w; tVertices[1].y = rect.y;          tVertices[1].z = 0; tVertices[1].u = src.x + src.w; tVertices[1].v = src.y;
		tVertices[2].x = rect.x;          tVertices[2].y = rect.y + rect.h; tVertices[2].z = 0; tVertices[2].u = src.x;         tVertices[2].v = src.y + src.h;
		tVertices[3].x = rect.x + rect.w; tVertices[3].y = rect.y + rect.h; tVertices[3].z = 0; tVertices[3].u = src.x + src.w; tVertices[3].v = src.y + src.h;

		render(TriangleStrip, tVertices, 4);		
	}
	
	void RenderSystem::drawTexturedQuad(grect rect, grect src, Color color)
	{
		tVertices[0].x = rect.x;          tVertices[0].y = rect.y;          tVertices[0].z = 0; tVertices[0].u = src.x;         tVertices[0].v = src.y;
		tVertices[1].x = rect.x + rect.w; tVertices[1].y = rect.y;          tVertices[1].z = 0; tVertices[1].u = src.x + src.w; tVertices[1].v = src.y;
		tVertices[2].x = rect.x;          tVertices[2].y = rect.y + rect.h; tVertices[2].z = 0; tVertices[2].u = src.x;         tVertices[2].v = src.y + src.h;
		tVertices[3].x = rect.x + rect.w; tVertices[3].y = rect.y + rect.h; tVertices[3].z = 0; tVertices[3].u = src.x + src.w; tVertices[3].v = src.y + src.h;
		
		render(TriangleStrip, tVertices, 4, color);
	}
	
	hstr RenderSystem::findTextureFile(chstr _filename)
	{
		hstr filename = _filename;
		if (hfile::exists(filename))
		{
			return filename;
		}
		hstr name;
		foreach (hstr, it, extensions)
		{
			name = filename + (*it);
			if (hfile::exists(name))
			{
				return name;
			}
		}
		int index = filename.rfind(".");
		if (index >= 0)
		{
			filename = filename.substr(0, index);
			foreach (hstr, it, extensions)
			{
				name = filename + (*it);
				if (hfile::exists(name))
				{
					return name;
				}
			}
		}
		return "";
	}
	
	RAMTexture* RenderSystem::loadRAMTexture(chstr filename, bool dynamic)
	{
		hstr name = findTextureFile(filename);
		if (name == "")
		{
			return 0;
		}
		return new RAMTexture(name, dynamic);
	}
	
	Texture* RenderSystem::createBlankTexture(int w, int h, TextureFormat fmt, TextureType type)
	{
		Texture* texture = this->createEmptyTexture(w, h, fmt, type);
		texture->fillRect(0, 0, w, h, april::Color(APRIL_COLOR_WHITE, 0));
		return texture;
	}
	
	void RenderSystem::setIdentityTransform()
	{
		mModelviewMatrix.setIdentity();
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::translate(float x, float y, float z)
	{
		mModelviewMatrix.translate(x, y, z);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::rotate(float angle, float ax, float ay, float az)
	{
		mModelviewMatrix.rotate(ax, ay, az, angle);
		_setModelviewMatrix(mModelviewMatrix);
	}	
	
	void RenderSystem::scale(float s)
	{
		mModelviewMatrix.scale(s);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::scale(float sx, float sy, float sz)
	{
		mModelviewMatrix.scale(sx, sy, sz);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::lookAt(const gvec3 &eye, const gvec3 &direction, const gvec3 &up)
	{
		mModelviewMatrix.lookAt(eye, direction, up);
		_setModelviewMatrix(mModelviewMatrix);
	}
		
	void RenderSystem::setOrthoProjection(float x, float y, float w, float h)
	{
		setOrthoProjection(grect(x, y, w, h));
	}
	
	void RenderSystem::setOrthoProjection(gvec2 size)
	{
		setOrthoProjection(grect(0.0f, 0.0f, size));
	}
	
	void RenderSystem::setOrthoProjection(grect rect)
	{
		float t = getPixelOffset();
		float wnd_w = (float)mWindow->getWidth();
		float wnd_h = (float)mWindow->getHeight();
		rect.x -= t * rect.w / wnd_w;
		rect.y -= t * rect.h / wnd_h;
		mProjectionMatrix.ortho(rect);
		_setProjectionMatrix(mProjectionMatrix);
	}
	
    void RenderSystem::setPerspective(float fov, float aspect, float nearClip, float farClip)
	{
		mProjectionMatrix.perspective(fov, aspect, nearClip, farClip);
		_setProjectionMatrix(mProjectionMatrix);
	}
	
	void RenderSystem::setModelviewMatrix(const gmat4& matrix)
	{
		mModelviewMatrix = matrix;
		_setModelviewMatrix(matrix);
	}
	void RenderSystem::setProjectionMatrix(const gmat4& matrix)
	{
		mProjectionMatrix = matrix;
		_setProjectionMatrix(matrix);
	}
	
	void RenderSystem::setResolution(int w, int h)
	{
		log(hsprintf("changing resolution: %d x %d", w, h));
		mWindow->_setResolution(w, h);
	}

	const gmat4& RenderSystem::getModelviewMatrix()
	{
		return mModelviewMatrix;
	}
	
	const gmat4& RenderSystem::getProjectionMatrix()
	{
		return mProjectionMatrix;
	}
	
	bool RenderSystem::isFullscreen()
	{
		return mWindow->isFullscreen();
	}
/************************************************************************************/

	void RenderSystem::presentFrame()
	{
		getWindow()->presentFrame();
	}
	
/*********************************************************************************/
	void init()
	{
#ifdef USE_IL
		ilInit();
#endif
		extensions += ".png";
		extensions += ".jpg";
#if TARGET_OS_IPHONE
		extensions += ".pvr";
#endif
	}
	
	void createRenderSystem(chstr rendersystem_name)
	{
#ifdef _OPENGL
		april::rendersys = OpenGL_RenderSystem::create();
#else
		april::rendersys = DirectX9_RenderSystem::create();
#endif
	}
	
	void createRenderTarget(int w,int h,bool fullscreen,chstr title)
	{
#ifdef _WIN32
		Window* window = createAprilWindow("Win32", w, h, fullscreen, title);
#else
		Window* window = createAprilWindow("SDL", w, h, fullscreen, title);
#endif
		april::rendersys->assignWindow(window);
	}
	
	void setLogFunction(void (*fnptr)(chstr))
	{
		g_logFunction = fnptr;
	}
	
	void destroy()
	{
		if (rendersys)
		{
			delete rendersys->getWindow();
			delete rendersys;
			rendersys = NULL;
		}
	}
	
	void addTextureExtension(chstr extension)
	{
		extensions += extension;
	}

}
