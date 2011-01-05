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

#if defined(__APPLE__) && !defined(_OPENGL)
#define _OPENGL
#endif
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#ifdef USE_IL
#include <IL/il.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "RenderSystem.h"
#ifdef _OPENGL
#include "OpenGL_RenderSystem.h"
#else
#include "DirectX9_RenderSystem.h"
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
	
	void logf(chstr message, ...)
	{
		va_list args;
		va_start(args, message);
		april::log(hvsprintf(message.c_str(), args));
		va_end(args);
	}
/*****************************************************************************************/
	void PlainVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}
/*****************************************************************************************/
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

	void RenderSystem::drawColoredQuad(grect rect, Color color)
	{
		PlainVertex v[4];
		v[0].x = rect.x;          v[0].y = rect.y;          v[0].z = 0;
		v[1].x = rect.x + rect.w; v[1].y = rect.y;          v[1].z = 0;
		v[2].x = rect.x;          v[2].y = rect.y + rect.h; v[2].z = 0;
		v[3].x = rect.x + rect.w; v[3].y = rect.y + rect.h; v[3].z = 0;
		
		render(TriangleStrip, v, 4, color);
	}
	
	void RenderSystem::drawTexturedQuad(grect rect, grect src)
	{
		TexturedVertex v[4];
		v[0].x = rect.x;          v[0].y = rect.y;          v[0].z = 0; v[0].u = src.x;         v[0].v = src.y;
		v[1].x = rect.x + rect.w; v[1].y = rect.y;          v[1].z = 0; v[1].u = src.x + src.w; v[1].v = src.y;
		v[2].x = rect.x;          v[2].y = rect.y + rect.h; v[2].z = 0; v[2].u = src.x;         v[2].v = src.y + src.h;
		v[3].x = rect.x + rect.w; v[3].y = rect.y + rect.h; v[3].z = 0; v[3].u = src.x + src.w; v[3].v = src.y + src.h;

		render(TriangleStrip, v, 4);		
	}
	
	void RenderSystem::drawTexturedQuad(grect rect, grect src, Color color)
	{
		TexturedVertex v[4];
		v[0].x = rect.x;          v[0].y = rect.y;          v[0].z = 0; v[0].u = src.x;         v[0].v = src.y;
		v[1].x = rect.x + rect.w; v[1].y = rect.y;          v[1].z = 0; v[1].u = src.x + src.w; v[1].v = src.y;
		v[2].x = rect.x;          v[2].y = rect.y + rect.h; v[2].z = 0; v[2].u = src.x;         v[2].v = src.y + src.h;
		v[3].x = rect.x + rect.w; v[3].y = rect.y + rect.h; v[3].z = 0; v[3].u = src.x + src.w; v[3].v = src.y + src.h;
		
		render(TriangleStrip, v, 4, color);
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
			hstr filename = filename.substr(0, index);
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
	
	Texture* RenderSystem::loadRAMTexture(chstr filename, bool dynamic)
	{
		hstr name = findTextureFile(filename);
		if (name == "")
		{
			return 0;
		}
		return new RAMTexture(name,dynamic);
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
		
	void RenderSystem::setOrthoProjection(float w, float h, float x_offset, float y_offset)
	{
		float t = getPixelOffset();
		float wnd_w = getWindow()->getWidth();
		float wnd_h = getWindow()->getHeight();
		mProjectionMatrix.ortho(w, h, x_offset + t * w / wnd_w, y_offset + t * h / wnd_h);
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
	
	const gmat4& RenderSystem::getModelviewMatrix()
	{
		return mModelviewMatrix;
	}
	
	const gmat4& RenderSystem::getProjectionMatrix()
	{
		return mProjectionMatrix;
	}
	
/*********************************************************************************/

	void RenderSystem::presentFrame()
	{
		getWindow()->presentFrame();
	}
	
/*********************************************************************************/
	void init(chstr rendersystem_name, int w, int h, bool fullscreen, chstr title)
	{
#ifdef USE_IL
		ilInit();
#endif
#ifdef _WIN32
		Window* window = createAprilWindow("Win32", w, h, fullscreen, title);
#else
		Window* window = createAprilWindow("SDL", w, h, fullscreen, title);
#endif
#ifdef _OPENGL
		createOpenGL_RenderSystem(window);
#else
		createDirectX9_RenderSystem(window);
#endif
		extensions += ".png";
		extensions += ".jpg";
#if TARGET_OS_IPHONE
		extensions += ".pvr";
#endif
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
