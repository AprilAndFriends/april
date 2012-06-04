/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>
#include <algorithm>
#ifdef __APPLE__
#include <TargetConditionals.h>
#ifndef _OPENGL
#define _OPENGL
#endif
#endif
#ifdef _ANDROID
#include <android/log.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "RenderSystem.h"
#include "ImageSource.h"
#include "TextureManager.h"
#include "Window.h"

namespace april
{
	PlainVertex pv[16];
	TexturedVertex tv[16];

	april::RenderSystem* rendersys = NULL;

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
		mTextureManager = new TextureManager();
	}
	
	RenderSystem::~RenderSystem()
	{
		delete mTextureManager;
	}

	void RenderSystem::restore()
	{
		april::log("restoring rendersystem");
	}

	void RenderSystem::drawQuad(grect rect, Color color)
	{
		pv[0].x = rect.x;			pv[0].y = rect.y;			pv[0].z = 0.0f;
		pv[1].x = rect.x + rect.w;	pv[1].y = rect.y;			pv[1].z = 0.0f;
		pv[2].x = rect.x + rect.w;	pv[2].y = rect.y + rect.h;	pv[2].z = 0.0f;
		pv[3].x = rect.x;			pv[3].y = rect.y + rect.h;	pv[3].z = 0.0f;
		pv[4].x = rect.x;			pv[4].y = rect.y;			pv[4].z = 0.0f;
		render(LineStrip, pv, 5, color);
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
		if (hresource::exists(filename))
		{
			return filename;
		}
		hstr name;
		harray<hstr> extensions = april::getTextureExtensions();
		foreach (hstr, it, extensions)
		{
			name = filename + (*it);
			if (hresource::exists(name))
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
				if (hresource::exists(name))
				{
					return name;
				}
			}
		}
		return "";
	}
	
	RamTexture* RenderSystem::loadRamTexture(chstr filename, bool dynamic)
	{
		hstr name = findTextureFile(filename);
		if (name == "")
		{
			return 0;
		}
		return new RamTexture(name, dynamic);
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
		mOrthoProjection = rect;
		float t = getPixelOffset();
		float wnd_w = (float)april::window->getWidth();
		float wnd_h = (float)april::window->getHeight();
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
		april::window->_setResolution(w, h);
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
		return april::window->isFullscreen();
	}

	void RenderSystem::presentFrame()
	{
		april::window->presentFrame();
	}
	
}
