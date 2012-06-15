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
#endif
#ifdef _ANDROID
#include <android/log.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "aprilUtil.h"
#include "ImageSource.h"
#include "RamTexture.h"
#include "RenderSystem.h"
#include "Texture.h"
#include "Window.h"

namespace april
{
	PlainVertex pv[16];
	TexturedVertex tv[16];
	
	april::RenderSystem* rendersys = NULL;
	
	RenderSystem::RenderSystem() : created(false), forcedDynamicLoading(false), textureIdleUnloadTime(0.0f),
		textureFilter(Texture::FILTER_LINEAR), textureAddressMode(Texture::ADDRESS_WRAP)
	{
		this->name = "Generic";
	}
	
	RenderSystem::~RenderSystem()
	{
		this->destroy();
	}
	
	bool RenderSystem::create(chstr options)
	{
		if (!this->created)
		{
			april::log(hsprintf("creating rendersystem '%s' (options: '%s')", this->name.c_str(), options.c_str()));
			this->created = true;
			return true;
		}
		return false;
	}
	
	bool RenderSystem::destroy()
	{
		if (this->created)
		{
			april::log(hsprintf("destroying rendersystem '%s'", this->name.c_str()));
			while (this->textures.size() > 0)
			{
				delete this->textures[0];
			}
			this->created = false;
			return true;
		}
		return false;
	}
	
	void RenderSystem::reset()
	{
		april::log("resetting rendersystem");
	}

	void RenderSystem::setOrthoProjection(grect rect)
	{
		this->orthoProjection = rect;
		float t = this->getPixelOffset();
		float wnd_w = (float)april::window->getWidth();
		float wnd_h = (float)april::window->getHeight();
		rect.x -= t * rect.w / wnd_w;
		rect.y -= t * rect.h / wnd_h;
		this->projectionMatrix.ortho(rect);
		this->_setProjectionMatrix(this->projectionMatrix);
	}
	
	void RenderSystem::setOrthoProjection(gvec2 size)
	{
		this->setOrthoProjection(grect(0.0f, 0.0f, size));
	}
	
	void RenderSystem::setModelviewMatrix(gmat4 matrix)
	{
		this->modelviewMatrix = matrix;
		this->_setModelviewMatrix(this->modelviewMatrix);
	}
	
	void RenderSystem::setProjectionMatrix(gmat4 matrix)
	{
		this->projectionMatrix = matrix;
		this->_setProjectionMatrix(this->projectionMatrix);
	}
	
	void RenderSystem::setResolution(int w, int h)
	{
		log(hsprintf("changing resolution: %d x %d", w, h));
		april::window->_setResolution(w, h);
	}
	
	Texture* RenderSystem::loadTexture(chstr filename, bool delayLoad)
	{
		hstr name = this->_findTextureFilename(filename);
		if (name == "")
		{
			return NULL;
		}
		if (this->forcedDynamicLoading)
		{
			delayLoad = true;
		}
		Texture* texture = this->_createTexture(name);
		if (!delayLoad)
		{
			texture->load();
		}
		if (!delayLoad && !texture->isLoaded())
		{
			delete texture;
			return NULL;
		}
		return texture;
	}

	Texture* RenderSystem::loadRamTexture(chstr filename, bool delayLoad)
	{
		hstr name = this->_findTextureFilename(filename);
		if (name == "")
		{
			return NULL;
		}
		if (this->forcedDynamicLoading)
		{
			delayLoad = true;
		}
		RamTexture* texture = new RamTexture(name);
		if (!delayLoad)
		{
			texture->load();
		}
		if (!delayLoad && !texture->isLoaded())
		{
			delete texture;
			return NULL;
		}
		return texture;
	}
	
	void RenderSystem::unloadTextures()
	{
		foreach (Texture*, it, this->textures)
		{
			(*it)->unload();
		}
	}
	
	void RenderSystem::setIdentityTransform()
	{
		this->modelviewMatrix.setIdentity();
		this->_setModelviewMatrix(this->modelviewMatrix);
	}
	
	void RenderSystem::translate(float x, float y, float z)
	{
		this->modelviewMatrix.translate(x, y, z);
		this->_setModelviewMatrix(this->modelviewMatrix);
	}
	
	void RenderSystem::rotate(float angle, float ax, float ay, float az)
	{
		this->modelviewMatrix.rotate(ax, ay, az, angle);
		this->_setModelviewMatrix(this->modelviewMatrix);
	}	
	
	void RenderSystem::scale(float s)
	{
		this->modelviewMatrix.scale(s);
		this->_setModelviewMatrix(this->modelviewMatrix);
	}
	
	void RenderSystem::scale(float sx, float sy, float sz)
	{
		this->modelviewMatrix.scale(sx, sy, sz);
		this->_setModelviewMatrix(this->modelviewMatrix);
	}
	
	void RenderSystem::lookAt(const gvec3 &eye, const gvec3 &direction, const gvec3 &up)
	{
		this->modelviewMatrix.lookAt(eye, direction, up);
		this->_setModelviewMatrix(this->modelviewMatrix);
	}
		
	void RenderSystem::setPerspective(float fov, float aspect, float nearClip, float farClip)
	{
		this->projectionMatrix.perspective(fov, aspect, nearClip, farClip);
		this->_setProjectionMatrix(this->projectionMatrix);
	}
	
	void RenderSystem::drawRect(grect rect, Color color)
	{
		pv[0].x = rect.x;			pv[0].y = rect.y;			pv[0].z = 0.0f;
		pv[1].x = rect.x + rect.w;	pv[1].y = rect.y;			pv[1].z = 0.0f;
		pv[2].x = rect.x + rect.w;	pv[2].y = rect.y + rect.h;	pv[2].z = 0.0f;
		pv[3].x = rect.x;			pv[3].y = rect.y + rect.h;	pv[3].z = 0.0f;
		pv[4].x = rect.x;			pv[4].y = rect.y;			pv[4].z = 0.0f;
		this->render(LineStrip, pv, 5, color);
	}

	void RenderSystem::drawFilledRect(grect rect, Color color)
	{
		pv[0].x = rect.x;			pv[0].y = rect.y;			pv[0].z = 0.0f;
		pv[1].x = rect.x + rect.w;	pv[1].y = rect.y;			pv[1].z = 0.0f;
		pv[2].x = rect.x;			pv[2].y = rect.y + rect.h;	pv[2].z = 0.0f;
		pv[3].x = rect.x + rect.w;	pv[3].y = rect.y + rect.h;	pv[3].z = 0.0f;
		this->render(TriangleStrip, pv, 4, color);
	}
	
	void RenderSystem::drawTexturedRect(grect rect, grect src)
	{
		tv[0].x = rect.x;			tv[0].y = rect.y;			tv[0].z = 0.0f;	tv[0].u = src.x;			tv[0].v = src.y;
		tv[1].x = rect.x + rect.w;	tv[1].y = rect.y;			tv[1].z = 0.0f;	tv[1].u = src.x + src.w;	tv[1].v = src.y;
		tv[2].x = rect.x;			tv[2].y = rect.y + rect.h;	tv[2].z = 0.0f;	tv[2].u = src.x;			tv[2].v = src.y + src.h;
		tv[3].x = rect.x + rect.w;	tv[3].y = rect.y + rect.h;	tv[3].z = 0.0f;	tv[3].u = src.x + src.w;	tv[3].v = src.y + src.h;
		this->render(TriangleStrip, tv, 4);
	}
	
	void RenderSystem::drawTexturedRect(grect rect, grect src, Color color)
	{
		tv[0].x = rect.x;			tv[0].y = rect.y;			tv[0].z = 0.0f;	tv[0].u = src.x;			tv[0].v = src.y;
		tv[1].x = rect.x + rect.w;	tv[1].y = rect.y;			tv[1].z = 0.0f;	tv[1].u = src.x + src.w;	tv[1].v = src.y;
		tv[2].x = rect.x;			tv[2].y = rect.y + rect.h;	tv[2].z = 0.0f;	tv[2].u = src.x;			tv[2].v = src.y + src.h;
		tv[3].x = rect.x + rect.w;	tv[3].y = rect.y + rect.h;	tv[3].z = 0.0f;	tv[3].u = src.x + src.w;	tv[3].v = src.y + src.h;
		this->render(TriangleStrip, tv, 4, color);
	}
	
	void RenderSystem::presentFrame()
	{
		april::window->presentFrame();
	}
	
	hstr RenderSystem::_findTextureFilename(chstr filename)
	{
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
			hstr noExtensionName = filename.substr(0, index);
			foreach (hstr, it, extensions)
			{
				name = noExtensionName + (*it);
				if (hresource::exists(name))
				{
					return name;
				}
			}
		}
		return "";
	}
	
	void RenderSystem::_registerTexture(Texture* texture)
	{
		this->textures += texture;
	}
	
	void RenderSystem::_unregisterTexture(Texture* texture)
	{
		this->textures -= texture;
	}
	
}
