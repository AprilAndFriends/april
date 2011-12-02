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
#if defined(_OPENGL)
#include "OpenGL_RenderSystem.h"
#elif defined(HAVE_MARMELADE)
#include <s3e.h>
#include "Marmelade_RenderSystem.h"
#else
#include "DirectX9_RenderSystem.h"
#endif
#include "ImageSource.h"
#include "Window.h"

namespace april
{
	PlainVertex pv[4];
	TexturedVertex tv[4];

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

	void ColoredVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void TexturedVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredTexturedVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredTexturedNormalVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void TexturedNormalVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredNormalVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}
/************************************************************************************/
	RenderSystem::RenderSystem()
	{
		mDynamicLoading = false;
		mIdleUnloadTime = 0;
		mTextureFilter = Linear;
		mTextureWrapping = true;
	}
	
	RenderSystem::~RenderSystem()
	{
	}

	void RenderSystem::drawColoredQuad(grect rect, Color color)
	{
		pv[0].x = rect.x;			pv[0].y = rect.y;			pv[0].z = 0.0f;
		pv[1].x = rect.x + rect.w;	pv[1].y = rect.y;			pv[1].z = 0.0f;
		pv[2].x = rect.x;			pv[2].y = rect.y + rect.h;	pv[2].z = 0.0f;
		pv[3].x = rect.x + rect.w;	pv[3].y = rect.y + rect.h;	pv[3].z = 0.0f;
		render(TriangleStrip, pv, 4, color);
	}
	
	void RenderSystem::drawTexturedQuad(grect rect, grect src)
	{
		tv[0].x = rect.x;			tv[0].y = rect.y;			tv[0].z = 0.0f;	tv[0].u = src.x;			tv[0].v = src.y;
		tv[1].x = rect.x + rect.w;	tv[1].y = rect.y;			tv[1].z = 0.0f;	tv[1].u = src.x + src.w;	tv[1].v = src.y;
		tv[2].x = rect.x;			tv[2].y = rect.y + rect.h;	tv[2].z = 0.0f;	tv[2].u = src.x;			tv[2].v = src.y + src.h;
		tv[3].x = rect.x + rect.w;	tv[3].y = rect.y + rect.h;	tv[3].z = 0.0f;	tv[3].u = src.x + src.w;	tv[3].v = src.y + src.h;
		render(TriangleStrip, tv, 4);
	}
	
	void RenderSystem::drawTexturedQuad(grect rect, grect src, Color color)
	{
		tv[0].x = rect.x;			tv[0].y = rect.y;			tv[0].z = 0.0f;	tv[0].u = src.x;			tv[0].v = src.y;
		tv[1].x = rect.x + rect.w;	tv[1].y = rect.y;			tv[1].z = 0.0f;	tv[1].u = src.x + src.w;	tv[1].v = src.y;
		tv[2].x = rect.x;			tv[2].y = rect.y + rect.h;	tv[2].z = 0.0f;	tv[2].u = src.x;			tv[2].v = src.y + src.h;
		tv[3].x = rect.x + rect.w;	tv[3].y = rect.y + rect.h;	tv[3].z = 0.0f;	tv[3].u = src.x + src.w;	tv[3].v = src.y + src.h;
		render(TriangleStrip, tv, 4, color);
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
		texture->fillRect(0, 0, w, h, APRIL_COLOR_BLANK);
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
#ifdef HAVE_MARMELADE
		extensions += ".png";
#else
		extensions += ".png";
		extensions += ".jpg";
#endif
#if TARGET_OS_IPHONE
		extensions += ".pvr";
#endif
	}
	
	void createRenderSystem(chstr options)
	{
#if TARGET_OS_IPHONE
		if (options == "create_eagl")
			april::rendersys = OpenGL_RenderSystem::create(options);
		//else do nothing, rendersys was created ahead
#elif defined(_OPENGL)
		april::rendersys = OpenGL_RenderSystem::create(options);
#elif defined(HAVE_MARMELADE)
		april::rendersys = Marmelade_RenderSystem::create(options);
#else
		april::rendersys = DirectX9_RenderSystem::create(options);
#endif
	}
	
	void createRenderTarget(int w, int h, bool fullscreen, chstr title)
	{
		Window* window = 0;
#if TARGET_OS_IPHONE
		return;
#elif defined(HAVE_MARMELADE)
		window = createAprilWindow("Marmelade", w, h, fullscreen, title);
#elif defined(_WIN32)
		window = createAprilWindow("Win32", w, h, fullscreen, title);
#else
		window = createAprilWindow("SDL", w, h, fullscreen, title);
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
