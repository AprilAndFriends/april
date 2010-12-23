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
#include "RenderSystem_GL.h"
#else
#include "RenderSystem_DirectX9.h"
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
	
	int hexstr_to_int(chstr s)
	{
		int i;
		sscanf(s.c_str(), "%x", &i);
		return i;
	}
/*****************************************************************************************/	
	void PlainVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}
/*****************************************************************************************/
	Color::Color(int r, int g, int b, int a)
	{
		set(r, g, b, a);
	}

	Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		set(r, g, b, a);
	}

	Color::Color(unsigned int color)
	{
		set(color);
	}

	Color::Color(chstr hex)
	{
		set(hex);
	}

	Color::Color()
	{
		r = 255;
		g = 255;
		b = 255;
		a = 255;
	}

	void Color::set(int r, int g, int b, int a)
	{
		this->r = (unsigned char)r;
		this->g = (unsigned char)g;
		this->b = (unsigned char)b;
		this->a = (unsigned char)a;
	}

	void Color::set(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	void Color::set(unsigned int color)
	{
		this->r = (unsigned char)((color >> 24) & 0xFF);
		this->g = (unsigned char)((color >> 16) & 0xFF);
		this->b = (unsigned char)((color >> 8) & 0xFF);
		this->a = (unsigned char)(color & 0xFF);
	}

	void Color::set(chstr hex)
	{
		hstr value = hex;
		if (value(0, 2) != "0x")
		{
			value = "0x" + value;
		}
		if (value.size() != 8 && value.size() != 10)
		{
			throw "Color format must be either 0xRRGGBBAA or 0xRRGGBB";
		}
		this->r = hexstr_to_int(value(2, 2));
		this->g = hexstr_to_int(value(4, 2));
		this->b = hexstr_to_int(value(6, 2));
		this->a = (value.size() == 10 ? hexstr_to_int(value(8, 2)) : 255);
	}
	
	hstr Color::hex()
	{
		return hsprintf("%02x%02x%02x%02x", this->r, this->g, this->b, this->a);
	}

	unsigned int Color::uint()
	{
		unsigned int i;
		i |= this->r << 24;
		i |= this->r << 16;
		i |= this->r << 8;
		i |= this->a;
		return i;
	}

	bool Color::operator==(Color& other)
	{
		return (this->r == other.r && this->g == other.g && this->b == other.b && this->a == other.a);
	}

	bool Color::operator!=(Color& other)
	{
		return !(*this == other);
	}
	
	Color Color::operator+(Color& other)
	{
		Color result(*this);
		result += other;
		return result;
	}

	Color Color::operator-(Color& other)
	{
		Color result(*this);
		result -= other;
		return result;
	}

	Color Color::operator*(Color& other)
	{
		Color result(*this);
		result *= other;
		return result;
	}

	Color Color::operator/(Color& other)
	{
		Color result(*this);
		result /= other;
		return result;
	}

	Color Color::operator*(float value)
	{
		Color result(*this);
		result *= value;
		return result;
	}

	Color Color::operator/(float value)
	{
		Color result(*this);
		result /= value;
		return result;
	}

	Color Color::operator+=(Color& other)
	{
		this->r = hclamp(this->r + other.r, 0, 255);
		this->g = hclamp(this->g + other.g, 0, 255);
		this->b = hclamp(this->b + other.b, 0, 255);
		this->a = hclamp(this->a + other.a, 0, 255);
		return (*this);
	}

	Color Color::operator-=(Color& other)
	{
		this->r = hclamp(this->r - other.r, 0, 255);
		this->g = hclamp(this->g - other.g, 0, 255);
		this->b = hclamp(this->b - other.b, 0, 255);
		this->a = hclamp(this->a - other.a, 0, 255);
		return (*this);
	}

	Color Color::operator*=(Color& other)
	{
		this->r = hclamp((int)(this->r_f() * other.r), 0, 255);
		this->g = hclamp((int)(this->g_f() * other.g), 0, 255);
		this->b = hclamp((int)(this->b_f() * other.b), 0, 255);
		this->a = hclamp((int)(this->a_f() * other.a), 0, 255);
		return (*this);
	}

	Color Color::operator/=(Color& other)
	{
		this->r = hclamp((int)(this->r_f() / other.r), 0, 255);
		this->g = hclamp((int)(this->g_f() / other.g), 0, 255);
		this->b = hclamp((int)(this->b_f() / other.b), 0, 255);
		this->a = hclamp((int)(this->a_f() / other.a), 0, 255);
		return (*this);
	}

	Color Color::operator*=(float value)
	{
		this->r = hclamp((int)(this->r * value), 0, 255);
		this->g = hclamp((int)(this->g * value), 0, 255);
		this->b = hclamp((int)(this->b * value), 0, 255);
		this->a = hclamp((int)(this->a * value), 0, 255);
		return (*this);
	}

	Color Color::operator/=(float value)
	{
		this->r = hclamp((int)(this->r / value), 0, 255);
		this->g = hclamp((int)(this->g / value), 0, 255);
		this->b = hclamp((int)(this->b / value), 0, 255);
		this->a = hclamp((int)(this->a / value), 0, 255);
		return (*this);
	}

/*****************************************************************************************/
	// DEPRECATED
	Color::Color(float r, float g, float b, float a)
	{
		this->r = (unsigned char)(r * 255);
		this->g = (unsigned char)(g * 255);
		this->b = (unsigned char)(b * 255);
		this->a = (unsigned char)(a * 255);
	}
	void Color::setColor(float r, float g, float b, float a)
	{
		this->r = (unsigned char)(r * 255);
		this->g = (unsigned char)(g * 255);
		this->b = (unsigned char)(b * 255);
		this->a = (unsigned char)(a * 255);
	}
	void Color::setColor(int r, int g, int b, int a)
	{
		set(r, g, b, a);
	}
	void Color::setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		set(r, g, b, a);
	}
	void Color::setColor(unsigned int color)
	{
		set(color);
	}
	void Color::setColor(chstr hex)
	{
		set(hex);
	}
/*****************************************************************************************/
	Texture::Texture()
	{
		mFilename = "";
		mUnusedTimer = 0;
		mTextureFilter = Linear;
		mTextureWrapping = true;
	}

	Texture::~Texture()
	{
		foreach (Texture*, it, mDynamicLinks)
		{
			(*it)->removeDynamicLink(this);
		}
	}
	
	Color Texture::getPixel(int x, int y)
	{
		return Color::CLEAR;
	}
	
	Color Texture::getInterpolatedPixel(float x, float y)
	{
		return Color::CLEAR; // TODO
	}
	
	void Texture::update(float time_increase)
	{
		if (mDynamic && isLoaded())
		{
			float max_time = rendersys->getIdleTextureUnloadTime();
			if (max_time > 0)
			{
				if (mUnusedTimer > max_time)
				{
					unload();
				}
				mUnusedTimer += time_increase;
			}
		}
	}
	
	void Texture::addDynamicLink(Texture* lnk)
	{
		if (!mDynamicLinks.contains(lnk))
		{
			mDynamicLinks += lnk;
			lnk->addDynamicLink(this);
		}
	}
	
	void Texture::removeDynamicLink(Texture* lnk)
	{
		if (mDynamicLinks.contains(lnk))
		{
			mDynamicLinks -= lnk;
		}
	}
	
	void  Texture::_resetUnusedTimer(bool recursive)
	{
		mUnusedTimer = 0;
		if (recursive)
		{
			foreach (Texture*, it, mDynamicLinks)
			{
				(*it)->_resetUnusedTimer(0);
			}
		}
	}
/*****************************************************************************************/
	RAMTexture::RAMTexture(chstr filename, bool dynamic)
	{
		mFilename = filename;
		mBuffer = NULL;
		if (!dynamic)
		{
			load();
		}
	}

	RAMTexture::~RAMTexture()
	{
		unload();
	}
	
	void RAMTexture::load()
	{
		if (!mBuffer)
		{
			april::log("loading RAM texture '" + mFilename + "'");
			mBuffer = loadImage(mFilename);
			mWidth = mBuffer->w;
			mHeight = mBuffer->h;
		}
	}
	
	void RAMTexture::unload()
	{
		if (mBuffer)
		{
			april::log("unloading RAM texture '" + mFilename + "'");
			delete mBuffer;
			mBuffer = NULL;
		}
	}
	
	bool RAMTexture::isLoaded()
	{
		return (mBuffer != NULL);
	}
	
	Color RAMTexture::getPixel(int x, int y)
	{
		if (!mBuffer)
		{
			load();
		}
		mUnusedTimer = 0;
		return mBuffer->getPixel(x, y);
	}
	
	void RAMTexture::setPixel(int x, int y, Color c)
	{
		if (!mBuffer)
		{
			load();
		}
		mUnusedTimer = 0;
		mBuffer->setPixel(x, y, c);
	}
	
	Color RAMTexture::getInterpolatedPixel(float x, float y)
	{
		if (!mBuffer)
		{
			load();
		}
		mUnusedTimer = 0;
		return mBuffer->getInterpolatedPixel(x, y);
	}
	
	int RAMTexture::getSizeInBytes()
	{
		return (mWidth * mHeight * 3);
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
	
	hstr RenderSystem::findTextureFile(chstr filename)
	{
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
		float wnd_w = getWindow()->getWindowWidth();
		float wnd_h = getWindow()->getWindowHeight();
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
		createGLRenderSystem(window);
#else
		createDX9RenderSystem(window);
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
